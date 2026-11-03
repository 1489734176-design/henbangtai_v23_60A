from pathlib import Path
root=Path(__file__).resolve().parents[2]
ns={'__file__': __file__}
exec(Path(__file__).with_name('apply_io.py').read_text(encoding='utf-8').split("edit('USER/inc/board.h'")[0], ns)
edit=ns['edit']

def main_c(s):
    start=s.index('\t/* 方向IO滤波初始化')
    end=s.index('\t/* 上电时快速检测电压状态 */',start)
    return s[:start]+'''\t/* 先确认扳机状态，防止初始gToolTrigger=0提前解除上电重扣锁定。 */
\tfor(i=0;i<30;i++)
\t{
\t\tsystick_delay_us(1000);
\t\tmotor_on_off_control();
\t}
\t/* 每次方向去抖之前等待新的复用输入采样。 */
\tfor(i=0;i<20;i++)
\t{
\t\tsystick_delay_us(1000);
\t\tuser_direction_handle();
\t}

'''+s[end:]
edit('USER/main.c',main_c)

def battery_test(s):
    s=s.replace('    user_list.dir_valid = 1;','    user_list.dir_valid = 1;\n    user_list.led_input_valid = 1;\n    user_list.Dir_Data = 0;')
    start=s.index('static void test_direction_interlock(void)')
    end=s.index('uint32_t run_tests(void)',start)
    return s[:start]+'''static void test_direction_interlock(void)
{
    uint8_t i;
    reset_fixture();
    link_safe();
    CHECK(Bat_Com_CanRun());
    user_list.led_input_valid=0;
    for(i=0;i<12;i++) user_direction_handle();
    CHECK(!user_list.dir_valid && !user_list.direction_ready);
    user_list.led_input_valid=1;
    user_list.Dir_Data=0;
    user_list.flag.bits.gToolTrigger=1;
    for(i=0;i<12;i++) user_direction_handle();
    CHECK(user_list.dir_retrig_need && !user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state!=TOOL_RUN && !user_list.flag.bits.gToolEn);
    user_list.flag.bits.gToolTrigger=0;
    user_direction_handle();
    CHECK(user_list.dir_valid && user_list.direction_ready);
    user_list.flag.bits.gToolTrigger=1;
    user_direction_handle();
    user_state_control();
    CHECK(user_list.user_state==TOOL_RUN && user_list.flag.bits.gToolEn);
    user_list.Dir_Data=1;
    for(i=0;i<12;i++) user_direction_handle();
    CHECK(!user_list.dir_valid && mc_core_list.mc_dir==0);
    user_state_control();
    CHECK(user_list.user_state==TOOL_STOP && !user_list.flag.bits.gToolEn);
    for(i=0;i<12;i++) user_direction_handle();
    CHECK(user_list.dir_retrig_need && !user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state!=TOOL_RUN);
    user_list.flag.bits.gToolTrigger=0;
    user_direction_handle();
    CHECK(mc_core_list.mc_dir==1 && user_list.direction_ready);
    user_list.flag.bits.gToolTrigger=1;
    user_direction_handle();
    user_state_control();
    CHECK(user_list.user_state==TOOL_RUN && user_list.flag.bits.gToolEn);
}

'''+s[end:]
edit('tests/battery_com/test_battery_com.c', battery_test)
edit('tests/battery_com/run_tests.py', lambda s:s.replace('"-fshort-enums", "-ffunction-sections",', '"-DBAT_COM_EN=1", "-fshort-enums", "-ffunction-sections",'))
print('Startup interlock and protocol regression adapted to cached single-wire direction.')
