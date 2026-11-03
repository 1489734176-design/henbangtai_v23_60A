import argparse
import os
from pathlib import Path
import struct
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
TEST_DIR = Path(__file__).resolve().parent


def extract_function(path, signature):
    source = path.read_text(encoding="utf-8", errors="replace")
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise ValueError(f"Unbalanced function in {path}")


def build(keil, output):
    output.mkdir(parents=True, exist_ok=True)
    for name in ("board.h", "mm32_device.h", "mm32spin0230.h", "HAL_device.h", "hal_conf.h", "hal_uid.h"):
        (output / name).write_text('#include "test_hw.h"\n', encoding="utf-8")
    code = ['#include "test_hw.h"', '#include "parameter.h"', '#include "motor_config.h"']
    code.append(f'#include "{(ROOT / "USER/Bat_com.c").as_posix()}"')
    code.append(extract_function(ROOT / "USER/user_control.c", "void user_state_control(void)"))
    code.append(extract_function(ROOT / "USER/user_control.c", "void user_direction_handle(void)"))
    code.append(extract_function(ROOT / "MOTOR_CONTROL/motor_control.c", "void MC_Machine_State(void)"))
    main = (ROOT / "USER/main.c").read_text(encoding="utf-8")
    start = main.index("/* 根据状态机设置电机使能标志 */")
    end = main.index("\n\t\t}\n    }\n}", start)
    code.append("void run_main_enable_ipd(void)\n{\n" + main[start:end] + "\n}")
    code.append(f'#include "{(TEST_DIR / "test_battery_com.c").as_posix()}"')
    source = output / "test_translation_unit.c"
    source.write_text("\n\n".join(code), encoding="utf-8")
    image = output / "battery_tests.axf"
    compiler = keil / "ARM/ARMCLANG/bin/armclang.exe"
    command = [
        str(compiler), "--target=arm-arm-none-eabi", "-mcpu=cortex-m0", "-std=c99", "-O1", "-g",
        "-fshort-enums", "-ffunction-sections", "-fdata-sections", "-Wall", "-Wextra", "-Werror",
        "-Wno-unused-parameter", "-Wno-nonportable-include-path",
        "-Wl,--ro-base=0x08000000,--rw-base=0x20000000,--entry=run_tests",
        "-o", str(image), str(source),
    ]
    for include in (output, TEST_DIR, ROOT / "USER/inc", ROOT / "MOTOR_CONTROL", ROOT / "MC_CORE"):
        command.extend(["-I", str(include)])
    subprocess.run(command, check=True, cwd=ROOT)
    return image


def execute(image):
    sys.path.insert(0, str(ROOT / ".tmp_protocol/test_deps"))
    from elftools.elf.elffile import ELFFile
    from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS
    from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

    emulator = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
    emulator.mem_map(0x08000000, 0x100000)
    emulator.mem_map(0x20000000, 0x100000)
    with image.open("rb") as stream:
        elf = ELFFile(stream)
        for segment in elf.iter_segments():
            if segment["p_type"] == "PT_LOAD" and segment["p_filesz"]:
                emulator.mem_write(segment["p_vaddr"], segment.data())
        symbols = {symbol.name: symbol["st_value"] for symbol in elf.get_section_by_name(".symtab").iter_symbols()}
    stop = 0x080F0000
    emulator.reg_write(UC_ARM_REG_SP, 0x200FFFF0)
    emulator.reg_write(UC_ARM_REG_LR, stop | 1)
    emulator.emu_start(symbols["run_tests"] | 1, stop, timeout=10_000_000, count=10_000_000)
    if emulator.reg_read(UC_ARM_REG_PC) != stop:
        raise RuntimeError("Test did not return within the instruction/time limit")

    def word(name):
        return struct.unpack("<I", emulator.mem_read(symbols[name], 4))[0]

    line = word("test_failure_line")
    case = word("test_case")
    checks = word("test_checks")
    if line:
        raise AssertionError(f"Case {case} failed at test_battery_com.c:{line} after {checks} checks")
    print(f"PASS: {case} cases, {checks} checks (real ARM C code, stubbed hardware; not a bench test)")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--keil", type=Path, default=os.environ.get("KEIL_ROOT"), required="KEIL_ROOT" not in os.environ)
    parser.add_argument("--build-dir", type=Path, default=ROOT / ".tmp_protocol/battery-tests")
    parser.add_argument("--compile-only", action="store_true")
    args = parser.parse_args()
    image = build(args.keil, args.build_dir.resolve())
    if not args.compile_only:
        execute(image)


if __name__ == "__main__":
    main()
