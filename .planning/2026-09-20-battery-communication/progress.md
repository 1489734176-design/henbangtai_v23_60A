# Progress

## 2026-09-20 — Resume
- Resolved the explicit task directory and read its existing plan; phase 2 was recorded as in progress.
- Checked git status and diff; no application source changes are present.
- `findings.md` and `progress.md` did not exist and were initialized without changing the existing plan or shared active pointer.
- The auto-memory index was absent; recovery relies on the selected project plan and current code.
- Next: verify UART, motor-permission, fault-recovery and sleep integration before implementing.

## Implementation
- User approved the implementation plan, including startup-state early exits and IPD gating.
- Replaced shared gap-framed reception with bounded LEN framing and an immutable completed-frame mailbox; added ISR-derived timebase and foreground scheduling, GPIO/AF bus release, TC completion and interrupt-enable checks.
- Added handshake-completion/runtime freshness gating and immediate BMS fault inhibition; integration edits and tests are in progress.
- Located Keil 6.22 under `D:/Users/mym02/AppData/Local/Keil_v5`.
- Installing Unicorn/pyelftools into the project temporary directory was initially denied; the user then explicitly approved both packages from PyPI, local-only, and installation completed.
- First clean Keil rebuild passed: 0 errors, 0 warnings; Code 20848 / RO 224 / RW 88 / ZI 1664 bytes. Later edits require another final rebuild.
- ARM simulation compiles the real Bat_com.c, actual app data headers and extracted unmodified user/motor state-machine/main gating blocks. Initial run: 8 cases / 159 checks passed. Hardware/peripheral timing and closed-library behavior are not simulated.

## Final software verification — 2026-09-20
- Phase 2 implementation and integration are complete. Added stop-priority exits in five startup states, final motor/IPD permission checks, transmission cancellation on restart/sleep, and sleep-state early exits so subsequent fault/recovery logic cannot overwrite POWER_DOWN.
- Final ARM regression run passed: 10 cases / 168 checks, including reception during reply waiting and invalid runtime frames not refreshing the safety deadline. Test compilation uses `-Wall -Wextra -Werror` with hardware stubs.
- Reproduce from the repository root: `"D:/Program Files/Python314/python" tests/battery_com/run_tests.py --keil "D:/Users/mym02/AppData/Local/Keil_v5"`.
- Final Keil clean rebuild log reviewed: `.tmp_protocol/battery-build.log`; 0 errors / 0 warnings; Code 21008 / RO-data 224 / RW-data 88 / ZI-data 1664 bytes. This build includes the final source changes.
- `git diff --check -- USER MOTOR_CONTROL` passed; Git only reported LF-to-CRLF conversion notices. Generated outputs and pre-existing IDE changes were not manually edited or discarded.
- No commit, push or flashing was performed. Test dependencies and temporary outputs remain under `.tmp_protocol/` and should not be included in a source commit.
- Phase 3 remains open for bench verification: COM/RX/TX timing and polarity, TC release, UART error behavior, TIM1 jitter, communication loss, BMS faults, held-trigger recovery, startup/IPD cancellation, sleep/wake and loaded protections. No board is connected; hardware revision, supply, load, parameter set and observed safety behavior are not yet recorded.
