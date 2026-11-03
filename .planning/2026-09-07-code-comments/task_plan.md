# Task Plan: Add Maintainability Comments

## Goal

Add useful Chinese comments to application-owned C code at function entries and around non-obvious control, timing, and protection logic without changing firmware behavior.

## Next Step

Inventory application-owned source files and identify functions and complex blocks that lack useful comments.

## Current Phase

Phase 1

## Phases

### Phase 1: Scope and Inventory

- [x] Confirm comment style and repository constraints
- [ ] Inventory application-owned C sources and existing comment coverage
- [ ] Record target files and exclusions
- **Status:** in_progress

### Phase 2: USER Module Comments

- [ ] Add function-entry comments where missing
- [ ] Explain non-obvious state, timing, and protection logic
- **Status:** pending

### Phase 3: Control and Driver Comments

- [ ] Cover MOTOR_CONTROL, DRIVE, and SYSTEM application wrappers
- [ ] Avoid vendor and generated sources
- **Status:** pending

### Phase 4: Verification

- [ ] Confirm edits are comment-only
- [ ] Run available build or syntax checks
- [ ] Review comment accuracy and encoding
- **Status:** pending

### Phase 5: Delivery

- [ ] Summarize commented modules and verification results
- **Status:** pending

## Key Questions

1. Which application-owned functions currently lack useful entry comments?
2. Which internal blocks need intent-level explanation rather than line-by-line narration?

## Decisions Made

| Decision | Rationale |
|----------|-----------|
| Use concise Chinese comments | Existing application code primarily uses Chinese comments. |
| Exclude MM32SPIN0230, CMSIS, prebuilt libraries, and generated output | Repository instructions identify these as vendor/generated content. |
| Preserve behavior and formatting | The request is documentation-only. |

## Errors Encountered

| Error | Attempt | Resolution |
|-------|---------|------------|
| Repository has no Git metadata | 1 | Use file snapshots and content comparisons for comment-only verification. |

## Notes

- Do not translate every statement; document intent, invariants, units, state transitions, and hardware-sensitive ordering.
