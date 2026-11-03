# Findings & Decisions

## Requirements

- Add battery temperature abnormal protection inside both `User_Temperature_Handler` and `User_Temperature_Handler_Fast`.
- Reference the existing MOS over-temperature protection style.
- Judge only `BatData.TempAnomaly_Flag`; do not inspect raw ADC values.
- Use the existing `UC_Error_u.gNtcBat` flag bit.

## Research Findings

- `Bat_com.c` clears `BatData.TempAnomaly_Flag` when battery state `0x03` is received and sets it after more than one `0x40` temperature-anomaly frame.
- `User_Temperature_Handler_Fast` is called during LED/SOC initialization; `User_Temperature_Handler` is called every 1ms from the main loop.
- `USER/user_control.c` has two conditionally compiled copies of the target functions. The outer `#if 0` means the current build uses the later `#else` copies at lines 1168 and 1191.
- Existing MOS protection uses direct fault mapping in the fast handler and 50ms trip / 500ms recovery counters in the slow handler.
- `UC_Error_u.gNtcBat` is already declared and consumed by the LED fault-display priority chain.
- `error_hold_updata` latches `gNtcBat`; `user_state_control` clears the latch after trigger release when all current user errors are clear.
- The final implementation uses the active `#else` branch only; battery protection executes outside `EN_MOS_TEMP_DETECT` so it remains enabled independently of MOS detection.
- `EN_BAT_TEMP_DETECT` now independently controls both the fast handler mapping and the 1ms filtered battery protection logic.

## Issues Encountered

| Issue | Resolution |
|-------|------------|
| None yet | None. |
