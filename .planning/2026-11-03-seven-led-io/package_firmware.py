from pathlib import Path
import hashlib
import json
import shutil

root=Path(__file__).resolve().parents[2]
build=Path(__file__).parent/'build'
dest=root/'output/firmware/seven-led'
log=(build/'rebuild.log').read_text(encoding='utf-8',errors='replace')
assert '0 Error(s), 0 Warning(s)' in log
memory={}
base=0
eof=False
for line in (build/'Objects/SPIN0230_DEMO.hex').read_text().splitlines():
    assert line.startswith(':')
    record=bytes.fromhex(line[1:])
    assert sum(record)%256==0 and len(record)==record[0]+5
    count=record[0]
    address=int.from_bytes(record[1:3],'big')
    kind=record[3]
    data=record[4:4+count]
    if kind==0:
        for offset,value in enumerate(data):
            target=base+address+offset
            assert 0x08000000<=target<0x08007c00, 'firmware overlaps saved-parameter page'
            memory[target]=value
    elif kind==4:
        base=int.from_bytes(data,'big')<<16
    elif kind==1:
        eof=True
assert eof and memory
binary=(build/'SPIN0230_DEMO.bin').read_bytes()
assert len(binary)==max(memory)-min(memory)+1
assert all(binary[address-min(memory)]==value for address,value in memory.items())
dest.mkdir(parents=True,exist_ok=True)
for suffix in ('hex','axf'):
    shutil.copy2(build/f'Objects/SPIN0230_DEMO.{suffix}',dest/f'SPIN0230_DEMO.{suffix}')
shutil.copy2(build/'SPIN0230_DEMO.bin',dest/'SPIN0230_DEMO.bin')
shutil.copy2(build/'rebuild.log',dest/'rebuild.log')
source_files=[*root.glob('USER/*.c'),*root.glob('USER/inc/*.h'),*root.glob('MOTOR_CONTROL/*.c'),*root.glob('MOTOR_CONTROL/*.h'),root/'KEIL_PRJ/MM32SPIN0230_Demo.uvprojx']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
manifest={
    'date':'2026-11-03',
    'compiler':'Arm Compiler 6.22',
    'target':'MM32SPIN0230',
    'build_result':'0 errors, 0 warnings',
    'flash_range':[hex(min(memory)),hex(max(memory)+1)],
    'bytes':len(binary),
    'hardware_tested':False,
    'tests':['seven_led: 8 cases, 122386 checks; 4 cold-start SOC profiles, 5 checks; SWD startup profile, 7 checks','battery_com: 12 cases, 220 checks (BAT_COM_EN=1 test build only)'],
    'firmware_sha256':{p.name:sha(p) for p in dest.glob('SPIN0230_DEMO.*')},
    'source_sha256':{p.relative_to(root).as_posix():sha(p) for p in source_files},
    'notes':['DS/TBAT reserved; battery communication and battery-temperature detection disabled.',
             'Trigger powers the board: first stable direction confirmation permits the initial press; subsequent direction changes still require release.',
             'SOC is initialized on first display, including an immediate motor run; running voltage sag does not change the displayed level.',
             'Removed separate 10s/30s illumination timers: matrix and PA12 illumination turn off together at the reference 5s sleep timeout after trigger release. Held faults keep displaying without a 30s cutoff.',
             'Reference scan sequencing: 7 LED slots, 4 fully high-impedance slots, input preparation and sampling = 13 slots. Matrix starts floating; PA14 GPIO remap is deferred until after startup calibration.',
             'The reference also remaps PA13/PA14 away from SWD AF0. High impedance does not restore AF0; reprogramming requires MCU reset and timely programmer connection. LCD_ALL_PRGMR continuous programming is not bench-verified.',
             'LED7 has a scan slot; automatic display defaults off pending its intended use.',
             'See docs/seven-led-board.md for IO, alarm mapping and required bench checks.']
}
(dest/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'Packaged {len(binary)} bytes; HEX checksums and HEX/BIN equality verified; parameter page untouched.')
print('HEX SHA256: '+manifest['firmware_sha256']['SPIN0230_DEMO.hex'])
