# Findings & Decisions

## Requirements

- Add comments at function beginnings.
- Add comments inside functions where behavior is difficult to understand.
- Do not change firmware logic.
- Follow existing code style and avoid vendor/generated files.

## Research Findings

- The repository is a bare-metal MM32SPIN0230 firmware project.
- Application ownership is concentrated in `USER/`, `MOTOR_CONTROL/`, `DRIVE/`, and `SYSTEM/`.
- No Git metadata or prior planning files were available when the task resumed.
- The Keil project compiles 16 application-owned C files: six under `USER/`, seven under `DRIVE/`, one under `SYSTEM/`, and `MOTOR_CONTROL/motor_control.c`.
- The largest and highest-value comment targets are `USER/user_control.c` (2072 lines), `MOTOR_CONTROL/motor_control.c` (1379), `USER/led.c` (966), `USER/board.c` (624), and `USER/mm32_it.c` (583).
- `DRIVE/drv_sqrt.c` and `USER/user_function.c` exist in the tree but are not listed in the Keil project, so they are secondary unless referenced elsewhere.
- `USER/main.c` needs intent comments for debug GPIO setup, boot-time sampling/calibration order, the 1 ms cooperative task chain, and the optional 10 ms speed loop.
- `USER/board.c` already documents many simple initializers, but lacks entry comments for the comparator GPIO, enable/direction pins, TIM14/TIM6 setup, and board comparator setup. The ADC rank linked list and direct comparator-register selection are the main internal maintenance hazards.
- `USER/mm32_it.c` uses the ADC EOS ISR to publish the complete scan, apply immediate and persistence-filtered peak-current protection, then invoke the motor-core ADC handler. The TIM1 update ISR is the high-frequency motor state-machine tick and derives the foreground 1 ms flag; the TIM1 break path latches hardware overcurrent.
- `USER/flash_data_save.c` implements a 10-slot append scheme in one flash page. `DATA_BLOCK_SIZE` is used as a byte stride, each valid record begins with `0x55AA`, and the page is erased only after all slots are occupied.
- `USER/led.c` multiplexes six LEDs and shared key/direction/NTC signals across a 12-step scan. Steps 1-6 drive one LED, step 7 tri-states shared pins for analog settling, step 9 exposes NTC data to the ADC consumer, step 11 restores digital levels/modes, and step 12 samples key/direction before returning the LED pins to outputs.
- LED fault selection is a strict `else-if` priority encoder, `LedError_Run` turns the selected code into repeated pulse groups, and `LedSoc_Run` uses asymmetric voltage thresholds (`deltaVolt`) to prevent SOC display chatter.
- `USER/user_control.c` temporarily stops/reconfigures ADC1 in `Vcc_5V_CAL`, validates the factory calibration word, measures the internal reference, computes a Q15 supply-correction gain, then restores the normal board ADC scan.
- The active `user_oc_handle` branch subtracts current offset and applies the Q15 supply gain. Four average-current stages use saturating persistence counters; the smart peak-current path integrates faster at higher thresholds and decays differently while running versus stopped.
- `Volt_Handler_fast` provides unfiltered startup flags, while `Volt_Handler` applies per-fault persistence/recovery counters and converts the corrected ADC value to `VBus_Vx100`. MOS NTC protection likewise uses separate trip and recovery thresholds/counters.
- `power_on_fly_machine_handle` temporarily disables the regular ADC/TIM1 paths, discards two conversions, averages eight samples of `FLY_CHANNEL`, latches the fly-machine condition, then rebuilds the regular ADC scan.
- Direction input must be stable for more than 10 foreground ticks and is only committed while the motor is stopped or braking. `key_scan` reports a short event on release (51-999 ticks) and a single long event at 1000 ticks.
- `pwm_duty_aim_deal` maps the trigger ADC range to requested PWM duty. In the active configuration, `user_gears_handle` passes that mapped request through unchanged; legacy pulsing/gear scaling is compiled out.
- `user_WrenchKey_Handler` cycles three forward gears when not in an active motor state, toggles reverse auto-stop mode, persists the selection, applies direction/gear duty limits, and enforces a minimum duty while restarting a detected load.
- Reverse auto-stop ignores startup transients, captures and clamps a load-dependent current threshold at the mask boundary, then requires the measured average current to remain below that threshold for `AUTO_STOP_Sector_CNT` foreground ticks.
- `motor_on_off_control` implements trigger hysteresis with separate on/off counters and a 100-count ADC gap. `error_hold_updata` OR-latches instantaneous errors for later state-machine handling.
- `user_state_control` controls tool enable, sleep/power-down, fault recovery, and auto-stop. Releasing the trigger is required before recoverable errors clear; hardware peak overcurrent is only cleared after the motor reaches stop, and the block error remains latched until the motor core releases it.
- `MOTOR_CONTROL/motor_control.c::mc_init` transfers compile-time timing/protection constants into the prebuilt motor-core parameter structure and resets all wrapper/core shared state before calling `mc_core_list_init`.
- Block protection combines elapsed run/open-loop time with four independent peak-current persistence counters. Leaving run/open-loop resets the trip counters and only clears the block error after a separate recovery delay.
- `pwm_set` is a two-stage ramp: requested duty becomes `pwm_duty_virtual` through the slow ramp, then actual PWM follows through the fast ramp. Peak/average current limiting suppresses increases and forces decreases; direction and gear select the average-current threshold.
- The motor state machine performs motion detection before startup, optionally brakes a moving rotor before IPD, open-loop drags until its commutation interval reaches the handoff threshold, and then runs closed-loop. Stop handling can pass through brake-wait, soft-brake, hard-brake, and brake-to-stop states depending on configuration and faults.
- Speed measurement applies a Q15 0.3/0.7 first-order filter to the 60-degree period. The optional PI is incremental and conditionally disables integration at output limits unless the error drives the output back into range.

## Technical Decisions

| Decision | Rationale |
|----------|-----------|
| Comment intent and hardware constraints | These remain useful when implementation details change. |
| Avoid obvious statement narration | Excess comments make embedded control code harder to scan. |
| Use the Keil source list as the primary scope | It identifies code that affects the shipped firmware and avoids spending time on dormant files. |
| Group trivial empty interrupt vectors under one explanatory comment | Individual boilerplate headers would add volume without clarifying behavior. |

## Issues Encountered

| Issue | Resolution |
|-------|------------|
| No prior task record found | Reconstructed scope from the user's clarification. |

## Resources

- `AGENTS.md`
- `USER/`
- `MOTOR_CONTROL/`
- `DRIVE/`
- `SYSTEM/`

## Visual/Browser Findings

- None.
