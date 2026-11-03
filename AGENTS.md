# Repository Guidelines

## Project Structure & Module Organization

This repository contains bare-metal C firmware for the MM32SPIN0230B1NV (Cortex-M0). Application entry points, board setup, and product behavior live in `USER/`, with public headers in `USER/inc/`. Motor state and control integration are in `MOTOR_CONTROL/`; peripheral wrappers are in `DRIVE/`; system timing support is in `SYSTEM/`. Treat `MM32SPIN0230/` and `CMSIS/` as vendor code and avoid unrelated edits there. `MC_CORE/` exposes a header plus the prebuilt `002_mc_core_V1_01.lib`. The Keil project is `KEIL_PRJ/MM32SPIN0230_Demo.uvprojx`; `KEIL_PRJ/Objects/` and `KEIL_PRJ/Listings/` contain generated artifacts.

## Build, Flash, and Development Commands

Use Keil uVision with Arm Compiler 6. From a Developer Command Prompt:

```powershell
UV4.exe -b KEIL_PRJ\MM32SPIN0230_Demo.uvprojx -t MM32SPIN0230
UV4.exe -r KEIL_PRJ\MM32SPIN0230_Demo.uvprojx -t MM32SPIN0230
```

The first command performs an incremental build; the second rebuilds all sources. Successful builds produce `SPIN0230_DEMO.axf` and `.hex` under `KEIL_PRJ/Objects/`. Flash and debug through the project's configured J-Link target in uVision. Do not hand-edit generated files or include incidental build-output churn in source changes.

## Coding Style & Naming Conventions

Match the file being edited: application modules commonly use tabs, while vendor drivers generally use four spaces. Keep braces on their own lines. Use `snake_case` for application functions and variables, uppercase snake case for configuration macros, and established suffixes such as `_t`, `_TypeDef`, and `_Struct` for types. Pair new modules as `name.c` and `USER/inc/name.h` (or the owning module's `inc/` directory). Preserve existing Doxygen blocks and include guards. Avoid broad reformatting, especially in vendor sources.

## Testing Guidelines

There is no host-side test framework or coverage gate. Every change must complete a warning-reviewed clean rebuild. For motor-control, ADC, PWM, direction, voltage, or protection changes, test on the intended board and record hardware revision, supply voltage, load, parameter set, and observed safety behavior in the pull request. Never bypass current, thermal, undervoltage, watchdog, or stall protections merely to make a bench test pass.

## Commit & Pull Request Guidelines

This source export contains no Git metadata, so no historical commit convention can be verified. Use short imperative subjects, optionally scoped, for example `motor: correct stall recovery timing`. Keep generated artifacts separate from source changes. Pull requests should explain behavior and risk, list changed configuration macros, provide build results and bench-test evidence, and link the issue. Add logs or waveforms when timing or electrical behavior changes; screenshots are only useful for debugger or configuration changes.
