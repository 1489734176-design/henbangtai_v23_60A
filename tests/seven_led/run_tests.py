"""Run the real ARM C alarm/IO code under Unicorn, without a physical board."""
import importlib.util
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / '.planning/2026-11-03-seven-led-io/tests'
KEIL = Path('D:/Users/mym02/AppData/Local/Keil_v5')
spec = importlib.util.spec_from_file_location('battery_runner', ROOT / 'tests/battery_com/run_tests.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    code = ['#include "main.h"', '#include "board.h"', '#include "user_control.h"', '#include "parameter.h"', '#include "motor_control.h"', '#include "led.h"', '#include "Bat_com.h"']
    code += ['User_list_Struct user_list;', 'Machine_ERROR_HOLD machine_error_hold;',
             'Mc_core_list mc_core_list;', 'Motor_Control_list motor_control_list;']
    code.append(runner.extract_function(ROOT / 'USER/user_control.c', 'void user_direction_handle(void)'))
    code.append(runner.extract_function(ROOT / 'USER/user_control.c', 'u8 key_scan(void)'))
    code += ['void Bsp_Op_Init(void) {}', 'void Bsp_Comp_Init(void) {}',
             'void Bsp_Adc_Init(void) {}', 'void Bsp_Pwm_Init(void) {}']
    for signature in ['void Bsp_Key_Init(void)', 'void Bsp_dir_init(void)', 'void Bsp_Gpio_Init(void)']:
        code.append(runner.extract_function(ROOT / 'USER/board.c', signature))
    for signature in ['uint8_t Bat_Com_CanRun(void)', 'void Bat_Com_Sleep(void)', 'void Bat_Com_Restart(void)']:
        code.append(runner.extract_function(ROOT / 'USER/Bat_com.c', signature))
    code.append(runner.extract_function(ROOT / 'USER/user_control.c', 'void user_state_control(void)'))
    code.append('#include "' + (ROOT / 'tests/seven_led/test_seven_led.c').as_posix() + '"')
    source = OUT / 'test_main.c'
    source.write_text('\n'.join(code), encoding='utf-8')
    image = OUT / 'seven_led_tests.axf'
    command = [str(KEIL / 'ARM/ARMCLANG/bin/armclang.exe'), '--target=arm-arm-none-eabi',
               '-mcpu=cortex-m0', '-std=c99', '-O1', '-g', '-fshort-enums', '-ffunction-sections',
               '-fdata-sections', '-DUSE_STDPERIPH_DRIVER', '-DUSE_5V',
               '-Wno-invalid-source-encoding', '-Wno-nonportable-include-path',
               '-Wl,--ro-base=0x08000000,--rw-base=0x20000000,--entry=run_tests',
               '-o', str(image), str(source), str(ROOT / 'USER/led.c'),
               str(ROOT / 'MM32SPIN0230/HAL_Lib/src/hal_gpio.c'),
               str(ROOT / 'MM32SPIN0230/HAL_Lib/src/hal_rcc.c'),
               str(ROOT / 'MM32SPIN0230/HAL_Lib/src/hal_tim.c')]
    for path in ['USER/inc', 'MOTOR_CONTROL', 'MC_CORE', 'DRIVE/inc', 'SYSTEM/inc',
                 'MM32SPIN0230/Include', 'MM32SPIN0230/HAL_Lib/inc', 'CMSIS/Core/Include']:
        command += ['-I', str(ROOT / path)]
    subprocess.run(command, check=True, cwd=ROOT)
    failures = []
    for scenario in range(6):
        try:
            execute(image, scenario)
        except AssertionError as error:
            failures.append(str(error))
    if failures:
        raise AssertionError('; '.join(failures))

def execute(image, boot_scenario=0):
    import struct
    sys.path.insert(0, str(ROOT / '.tmp_protocol/test_deps'))
    from elftools.elf.elffile import ELFFile
    from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS, UC_HOOK_MEM_WRITE
    from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC
    uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
    uc.mem_map(0x08000000, 0x100000)
    uc.mem_map(0x20000000, 0x100000)
    uc.mem_map(0x40000000, 0x40000)
    def gpio_write(uc, access, address, size, value, data):
        # Model GPIO set/reset effects. GPIO base addresses are verified in reg_common.h.
        for base in (0x48000000, 0x48000400):
            if address in (base + 0x10, base + 0x14):
                odr = struct.unpack('<I', uc.mem_read(base + 0x0c, 4))[0]
                odr = ((odr | (value & 0xffff)) & ~(value >> 16)) if address == base + 0x10 else odr & ~value
                uc.mem_write(base + 0x0c, struct.pack('<I', odr))
    uc.mem_map(0x48000000, 0x10000)
    uc.hook_add(UC_HOOK_MEM_WRITE, gpio_write)
    with image.open('rb') as stream:
        elf = ELFFile(stream)
        for segment in elf.iter_segments():
            if segment['p_type'] == 'PT_LOAD' and segment['p_filesz']:
                uc.mem_write(segment['p_vaddr'], segment.data())
        symbols = {s.name: s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols()}
    if boot_scenario:
        uc.mem_write(symbols['test_boot_scenario'], struct.pack('<I', boot_scenario))
    stop = 0x080f0000
    uc.reg_write(UC_ARM_REG_SP, 0x200ffff0)
    uc.reg_write(UC_ARM_REG_LR, stop | 1)
    uc.emu_start(symbols['run_tests'] | 1, stop, timeout=30_000_000, count=40_000_000)
    assert uc.reg_read(UC_ARM_REG_PC) == stop, 'test did not return'
    def word(name):
        return struct.unpack('<I', uc.mem_read(symbols[name], 4))[0]
    line, case, checks = map(word, ['test_failure_line', 'test_case', 'test_checks'])
    if line:
        raise AssertionError(f'case {case}, test_seven_led.c:{line}, after {checks} checks')
    label = ('SWD startup profile' if boot_scenario == 5 else
             f'SOC boot profile {boot_scenario}' if boot_scenario else f'{case} cases')
    print(f'PASS: {label}, {checks} checks (ARM C / simulated GPIO, not a bench test)')

if __name__ == '__main__':
    main()
