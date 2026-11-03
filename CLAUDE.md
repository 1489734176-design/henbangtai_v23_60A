# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Summary

Bare-metal MM32SPIN0230 (Cortex-M0) firmware for a brushless electric drill. No RTOS: a 16 kHz TIM1 update interrupt drives the motor state machine, ADC sampling, and a derived 1 ms foreground task. The sensorless BLDC commutation core is a **prebuilt library** (`MC_CORE/002_mc_core_V1_01.lib`), consumed via `MC_CORE/mc_core.h`. Vendor HAL lives in `MM32SPIN0230/` and `CMSIS/` — treat as read-only.

## Directory Layout

| Path | Ownership | Contents |
|------|-----------|----------|
| `USER/` | App | `main.c` (init + 1 ms loop), `board.c` (pin/peripheral setup), `user_control.c` (trigger, protection, state machine, PWM target), `led.c` (12-slot LED/key/NTC multiplexer), `flash_data_save.c` (10-slot flash wear leveling), `mm32_it.c` (ADC/TIM1/TIM14/COMP ISRs). Headers in `USER/inc/`. |
| `MOTOR_CONTROL/` | App | `motor_control.c` / `motor_control.h` — motor state machine (`MOTOR_STOP→MOTION→POSITION→RUN`), `pwm_set()`, block/current calibration. `motor_config.h` — timing/protection constants. |
| `DRIVE/` | App | Peripheral wrappers: `drv_adc.c`, `drv_comp.c`, `drv_pwm.c` (six-step phases, brake/stop, CCR update), `drv_opamp.c`, `drv_iwdg.c`, `drv_led.c`, `drv_div.c`, `drv_sqrt.c`. |
| `SYSTEM/` | App | `systick.c` — blocking microsecond delay only (not the scheduler). |
| `MC_CORE/` | Core lib | `mc_core.h` (types + ISR dispatchers) + `002_mc_core_V1_01.lib` (closed source). |
| `MM32SPIN0230/`, `CMSIS/` | Vendor | HAL + device headers + startup assembly. |
| `KEIL_PRJ/` | IDE | `MM32SPIN0230_Demo.uvprojx` (single target `MM32SPIN0230`, ARM-ADS toolset). `Objects/` and `Listings/` are generated outputs. |
| `parameter-300N.h` | Spec | Top-level config macro table (protection currents, thresholds, feature enables). `USER/inc/parameter.h` mirrors this for builds. |
| `docs/BLDC_PROJECT_WALKTHROUGH.md` | Guide | Architecture walkthrough, runtime sequence, four interrupt domains, debug/observation order. |
| `.planning/` | Process | Task plans and findings for the in-flight comment-enrichment work. |

**Key note on `parameter-300N.h`:** The repository root holds `parameter-300N.h` (a named product parameter set), but the build include path lists `USER/inc` first, so `USER/inc/parameter.h` is the header actually compiled. Confirm any parameter edit lands in the file the compiler will pick up.

## Build & Flash

Keil µVision with ARM Compiler 6 (ARM-ADS). From a Developer Command Prompt (requires `UV4.exe` on PATH):

```
UV4.exe -b KEIL_PRJ\MM32SPIN0230_Demo.uvprojx -t MM32SPIN0230          # incremental build
UV4.exe -r KEIL_PRJ\MM32SPIN0230_Demo.uvprojx -t MM32SPIN0230           # rebuild all
```

Outputs: `KEIL_PRJ\Objects\SPIN0230_DEMO.axf` and `.hex`. Flash / debug through the project's configured J-Link target inside uVision. CLI build cannot flash.

Include path order (from the project): `MM32SPIN0230/Include; MM32SPIN0230/HAL_Lib/inc; DRIVE/inc; SYSTEM/inc; USER/inc; MOTOR_CONTROL; MC_CORE`.

No host-side test framework or coverage gate exists. Every change must complete a warning-reviewed clean rebuild; motor/ADC/PWM/protection changes require bench verification on the target board (record hardware revision, supply voltage, load, parameter set, observed safety behavior in the PR).

## Runtime Architecture

### Interrupt domains (see `USER/mm32_it.c`)
All critical — no blocking calls, Flash writes, or large loops allowed inside them.

| Domain | Frequency / trigger | Responsibility |
|---|---|---|
| `TIM1_BRK_UP_TRG_COM_IRQHandler` | 16 kHz | Motor state tick (`MC_Machine_State`), 1 ms flag generation (every 16th), LED/key/NTC scan step, the `motor_block_detect` counter advance. |
| `ADC_IRQHandler` | TIM1 CC4 sync | Publishes the full ADC scan to `mc_core_list.Board_ADC_DATA`, applies immediate + persistence peak-current OCP, then calls `mc_core_adc_isr_handle`. |
| `COMP1_2_IRQHandler` | BEMF zero-crossing | Calls `mc_core_comp_isr_handle` → schedules TIM14 delayed commutation (`change_phase`). |
| `TIM14_IRQHandler` | Post-zero timing | `mc_core_timer14_isr_handle` executes the delayed commutation. |

### Foreground 1 ms task (`USER/main.c` `while(1)`)
Order is significant:
```
direction filter → voltage protect → trigger debounce → current protect
→ MOS temperature → pwm_duty_aim_deal → (Wrench key | gear) → [optional speed PI]
→ user_state_control → error_hold_updata → LED handler → sync gToolEn→motor_en → IPD trigger
```
`main()` owns tool permission (`user_list.user_state`/`flag.gToolEn`); the motor core owns electrical commutation (`motor_control_list.bldc_state`/`mc_core_list.motor_en`). Do not confuse the two.

### Four global data hubs
- `user_list` (`User_list_Struct`) — trigger, direction, gears, measurements, real-time user faults.
- `motor_control_list` (`Motor_Control_list`) — outer motor state, PWM targets, calibration/brake config.
- `mc_core_list` (`Mc_core_list`) — shared with the closed .lib; ADC/BEMF data, commutation timing, current PWM, motor enable, core faults.
- `machine_error_hold` — OR-latches instantaneous `userErr` and `MC_error` so a fault that clears live stays latched until the recovery sequence runs.

### Protection strategy
`user_control.c::user_oc_handle` (average + smart peak current, VCC-gain corrected), `Volt_Handler` (VBUS under/over-voltage with separate trip/recovery persistence), `User_Temperature_Handler` (MOS NTC with hysteresis), `motor_control.c::motor_block_detect` (current-tiered or duty-based stall, recovery-delay gated). Hardware-level instantaneous overcurrent is latched by the TIM1 break path and cleared only after the motor reaches stop.

## Coding Style

- Match the file: application modules typically use **tabs**, vendor HAL uses 4 spaces. Braces on their own line.
- `snake_case` for functions/variables; `UPPER_SNAKE_CASE` for macros; `_t`/`_TypeDef`/`_Struct` suffixes for types.
- Pair new modules as `name.c` + `USER/inc/name.h` (or the owning module's `inc/`). Preserve Doxygen `@file`/`@attention` blocks and include guards.
- Avoid broad reformatting; vendor sources are read-only.

## Safety & Edit Boundaries

- **Never** bypass current, thermal, undervoltage, watchdog, or stall protections to pass a bench test.
- Do not edit `MM32SPIN0230/`, `CMSIS/`, `MC_CORE/002_mc_core_V1_01.lib`, or generated `KEIL_PRJ/` artifacts.
- `USER/user_function.c` is **not** in the Keil source list and duplicates `user_control.c` — leave it out of scope.
- When editing a protection threshold, update the macro in `USER/inc/parameter.h` (the compiled header); `parameter-300N.h` at root is a named reference set and is not what the compiler sees unless you change include order.

## Development Notes

- Debug GPIO (`io_debug_init`, PA2) is available but normally disabled; uncomment only for timing scope checks.
- `Vcc_5V_CAL()` mutates ADC1 config at boot (factory trim, 2.5 V ref measure, Q15 gain into `user_list.Vcc_cal_gain`); current-protection math depends on that gain — re-run calibration after any ADC reference or rail change.
- `LedSoc_Run` uses asymmetric hysteresis (`deltaVolt`) to prevent SOC display chatter; keep that asymmetry if adjusting thresholds.
- The optional speed-loop path (`LIMIT_SPEED_EN`) is compiled out (`parameter.h` sets it `0`); if enabling it, `Speed_PI_init`/`Speed_PI_reset`/`MC_PI_Handler` come alive in the 1 ms loop and `pwm_duty_aim2` becomes the speed-PI target.
