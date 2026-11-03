# Task Plan: Battery Temperature Protection

## Goal

Add battery temperature anomaly protection to `User_Temperature_Handler` and `User_Temperature_Handler_Fast` by reacting only to `BatData.TempAnomaly_Flag`, following the existing MOS over-temperature protection behavior.

## Next Step

None. All phases are complete.

## Current Phase

Phase 5

## Phases

### Phase 1: Research Existing Protection Flow

- [x] Locate both target functions
- [x] Locate MOS over-temperature handling and `gNtcBat` error-bit usage
- [x] Confirm `BatData.TempAnomaly_Flag` type and lifecycle
- **Status:** complete

### Phase 2: Implement Battery Temperature Protection

- [x] Add the anomaly check to both requested handlers
- [x] Use `UC_Error_u.gNtcBat` rather than duplicating ADC checks
- [x] Preserve existing control flow and style
- **Status:** complete

### Phase 3: Verification

- [x] Review the patch for unintended changes
- [x] Run the clean Keil rebuild if available
- [x] Record build or validation result
- **Status:** complete

### Phase 4: Delivery

- [x] Update planning records
- [x] Summarize changed behavior and validation
- **Status:** complete

### Phase 5: Battery Protection Enable Macro

- [x] Add `EN_BAT_TEMP_DETECT` to the temperature-protection parameter section
- [x] Isolate the fast-handler battery logic
- [x] Isolate the 1ms-handler battery logic and its counters
- [x] Verify with a clean Keil rebuild
- **Status:** complete

## Decisions Made

| Decision | Rationale |
|----------|-----------|
| Create an isolated planning session | This is a behavior change, unlike the active comment-only task. |
| Check only `BatData.TempAnomaly_Flag` | Explicit user requirement; ADC anomaly detection is already centralized. |
| Patch only the active `#else` implementations | `#if 0` excludes the duplicate function bodies at lines 584 and 609 from the current build. |
| Reuse MOS 50ms trip and 500ms recovery filtering | No battery-specific timing macros exist, and this matches the referenced protection behavior. |
| Add `EN_BAT_TEMP_DETECT` as an independent switch | Keeps MOS and battery temperature protection separately configurable. |

## Errors Encountered

| Error | Attempt | Resolution |
|-------|---------|------------|
| PowerShell parsed the patch body as commands | 1 | Pipe the patch as standard input through a here-string. |
| `apply_patch` returned Access denied in PowerShell | 2 | Invoke the same tool through Bash. |
