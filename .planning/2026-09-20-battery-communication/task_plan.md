# DAYE battery communication

## Goal
Enable and complete existing DAYE A1.1 tool-side communication; preserve PA1 trigger and all motor protections.

## Current Phase
Phase 3 verification: software tests and clean rebuild passed; bench verification remains pending.

## Next Step
On the target board, measure COM/RX/TX idle, preamble, first-start-bit timing and TC release; record hardware revision, supply voltage, load and parameter set before proceeding to fault/loaded-motor tests.

### Phase 1: Protocol and schematic verification
**Status:** complete
- User confirmed COM-side measurement and V11 schematic match.
- Valid capture: FA FB 0C A5 00 C8 14 05 01 2D 1E 54; LEN is total and CRC8 poly25/init0/MSB-first.
- IMPORTANT correction after enlarged schematic: Q13/Q14 form a current-limited sink, NOT cascaded inverters. PA11 high pulls COM low; PA11 low releases. Q12 inverts COM to RX. Both UART directions use normal polarity. Frame idle releases COM high; ~4ms COM-low preamble provides RX mark before UART start.

### Phase 2: Implement communication and integration
**Status:** complete
- Reused Bat_com.c/.h and board/main/mm32_it hooks. No changes to vendor/library or protection thresholds.
- IRQ publishes immutable LEN-framed packets; foreground validates CRC/command and schedules responses; no unsolicited FF traffic.
- ISR-derived 1 ms clock, no parsing in the motor ISR. TC releases the bus. All USART ISR flags are gated by enable bits. Errors discard partial frames, not completed frames.
- Handshake TC completion and fresh safe runtime data gate motor permission; all BMS bits0..5 immediately inhibit, with trigger-release fault recovery. 3 s timeout inhibits and latches; restart/sleep cancel transmission without clearing unrelated protections.
- Added early exits in five motor startup states, final motor/IPD gating, and sleep-state early exits to preserve stop/power-down priority.

### Phase 3: Tests, clean build and bench verification
**Status:** in_progress
- Complete: ARM regression tests passed 10 cases / 168 checks, using actual production code with hardware stubs.
- Complete: final Keil clean rebuild and warning review; 0 errors / 0 warnings; Code 21008 / RO 224 / RW 88 / ZI 1664 bytes (`.tmp_protocol/battery-build.log`).
- Complete: application source whitespace checks and bench checklist recorded in findings/progress.
- Pending: physical COM/RX/TX timing and polarity, response/preamble window, TC release, UART errors, TIM1 jitter, disconnected communication, BMS faults, recovery, startup/IPD inhibition, sleep/wake and loaded protections. No bench connection; software results do not establish hardware compliance.

## Errors / limitations
- Earlier context wrongly inferred TX polarity; corrected using enlarged schematic before any source edits.
- PIL unavailable for cropping; full-resolution pdftoppm schematic suffices.
- No bench connection available.
