# Findings

## Recovery — 2026-09-20
- Selected task: `2026-09-20-battery-communication`; the shared `.active_plan` still points to a different task and is not used for this recovery.
- Working-tree changes are IDE/generated outputs only; communication source edits have not started.
- Existing `USER/Bat_com.c` uses a shared mutable RX buffer, 5 ms gap framing, unqualified USART flags, and unsolicited FF reconnect traffic. `USER/inc/Bat_com.h` still disables communication.
- Protocol/polarity findings in `task_plan.md` must be preserved; verify integration before changing source.

## Implementation checks
- UART HAL `USART_GetITStatus` only checks SR. RX errors require SR/DR servicing; empty physical COM idle is inverted at RX and must not discard a published frame.
- Motor startup states could overwrite STOP after permission withdrawal; approved scope includes early exits and foreground IPD gating.
- PA14 KEY_PIN is unused; actual LED is PA10, old PA11/PA12 LED macros are commented out. Existing BatData low-voltage/temperature flags feed independent protection handlers and must remain available.
- Keil is installed at `D:/Users/mym02/AppData/Local/Keil_v5`; ARMCLANG 6.22. Target has 32 KB Flash / 4 KB RAM.
- Initial tool discovery found no host C compiler and no Python unicorn/pyelftools packages. Armclang rejects host/no-target probing; resolved by compiling ARM Cortex-M0 code and executing it with Unicorn, with both Python packages installed locally under `.tmp_protocol/test_deps` after explicit user approval.
- One exact edit failed on indentation and was corrected after a targeted read. No unrelated source changes were discarded.

## Final verification — 2026-09-20
- Real ARM C regression tests passed: 10 cases / 168 checks. The harness compiles the production communication code and actual state-machine/main gating blocks against peripheral stubs; this does not validate analog circuitry, peripheral timing or closed-library behavior.
- Final clean build: Keil ARM Compiler 6.22, 0 errors / 0 warnings; Code 21008 / RO 224 / RW 88 / ZI 1664 bytes, within the target memory limits. Evidence: `.tmp_protocol/battery-build.log`.
- Full received frames survive subsequent FE/ORE/noise errors; partial frames are discarded. Receive-mailbox overflow revokes permission rather than retaining potentially stale safe BMS data.
- Only an actually completed handshake reply plus fresh safe runtime data can grant permission; BMS bits 0..5 and timeout revoke it, and a recovered link does not clear the held fault without the existing trigger-release recovery.
- Stop-priority exits prevent startup-state logic from undoing inhibition; sleep branches now exit immediately so POWER_DOWN cannot be overwritten after communication has been disabled.
- The 6 ms wait / approximately 4 ms preamble still requires scope measurement against the protocol's window definition, including 1 ms quantization and foreground/interrupt delays. Software tests and a successful build do not establish timing compliance.

## Bench checklist — not executed
- Record hardware revision, supply voltage, load, compiled parameter set and safety behavior for every run.
- Scope COM, PA11 TX and PA12 RX: released idle, preamble, first start bit, last stop bit/TC release, and behavior under UART framing/overrun/noise errors.
- Measure TIM1 timing with communication active; verify disconnected communication, all six BMS fault bits, held-trigger inhibition, release/re-press recovery, startup/IPD permission withdrawal and sleep/wake.
- Verify existing current, undervoltage, thermal, watchdog and stall protections under controlled loaded conditions without bypassing or changing thresholds.
