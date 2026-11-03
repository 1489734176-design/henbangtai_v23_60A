from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[2]
BACKUP = Path(__file__).parent / 'original'

def edit(name, transform):
    path = ROOT / name
    target = BACKUP / name
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists():
        shutil.copy2(path, target)
    raw = path.read_bytes()
    text = raw.decode('utf-8', errors='surrogateescape').replace('\r\n', '\n')
    changed = transform(text)
    newline = '\r\n' if b'\r\n' in raw else '\n'
    path.write_bytes(changed.replace('\n', newline).encode('utf-8', errors='surrogateescape'))

def replace(text, old, new):
    assert old in text, old
    return text.replace(old, new)

def board_h(s):
    s=s.replace('DIR_CHANNEL', 'T_BAT_CHANNEL').replace('DIR_RANK', 'T_BAT_RANK')
    s=s.replace('ADC_Channel_2   // 方向检测', 'ADC_Channel_2   // PA3：预留电池温度采样')
    s=s.replace('#define EN_PIN                       GPIO_Pin_13', '#define EN_PIN                       GPIO_Pin_11')
    s=s.replace('#define EN_PIN_SRC                   GPIO_PinSource13', '#define EN_PIN_SRC                   GPIO_PinSource11')
    start=s.index('// 方向检测引脚：')
    end=s.index('#define KEY_RCC_CLOCKGPIO', start)
    s=s[:start]+'''// PB8 与 LED 行1复用；仅在扫描输入窗口读取，低=正转，高=反转。
#define DIR_PIN                    GPIO_Pin_8

'''+s[end:]
    start=s.index('#define KEY_ONOFF_RCC_CLOCKGPIO')
    end=s.index('#define T_MOS_PORT',start)
    s=s[:start]+'''#define KEY_PIN_SRC                GPIO_PinSource14
#define KEY_PIN_AF                 GPIO_AF_1

#define T_BAT_PORT                 GPIOA
#define T_BAT_PIN                  GPIO_Pin_3

'''+s[end:]
    s=s.replace('#define KEY_RCC_CLOCKGPIO           (RCC_AHBENR_GPIOA_Msk)\n#define KEY1_PORT                   GPIOA\n#define KEY1_PIN                    GPIO_Pin_5',
                '#define KEY1_PORT                  KEY_PORT\n#define KEY1_PIN                   KEY_PIN')
    return s

def board_c(s):
    s=s.replace('DIR_RANK','T_BAT_RANK').replace('DIR_CHANNEL','T_BAT_CHANNEL')
    s=s.replace('VBUS、DIR、T_MOS', 'VBUS、T_BAT、T_MOS').replace('Rank7: DIR（方向检测）','Rank7: T_BAT（预留电池温度）')
    marker='\t/* T_MOS（MOS温度） */'
    s=replace(s, marker, '''\t/* PA3：DS/TBAT 预留采样，不参与当前保护判定。 */
\tGPIO_InitStructure.GPIO_Pin = T_BAT_PIN;
\tGPIO_Init(T_BAT_PORT, &GPIO_InitStructure);

'''+marker)
    start=s.index('void Bsp_Key_Init(void)')
    end=s.index('/**',start)
    s=s[:start]+'''void Bsp_Key_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHBPeriphClockCmd(KEY_RCC_CLOCKGPIO, ENABLE);
    GPIO_PinAFConfig(KEY_PORT, KEY_PIN_SRC, KEY_PIN_AF);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = KEY_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &gpio);
}

'''+s[end:]
    s=s.replace('配置PA13为推挽输出', '配置PA11为推挽输出')
    s=s.replace('/* PA13 的 AF0 是 SWDIO，普通 GPIO 必须先解除调试复用。 */', '/* PA11 用作 EN，保持普通 GPIO 功能。 */')
    s=s.replace('GPIO_InitStructure.GPIO_Pin     =  DIR_PIN | RWD_PIN;', 'GPIO_InitStructure.GPIO_Pin     =  DIR_PIN;')
    s=s.replace('配置PB8/PB9为上拉输入，用于检测机械换向开关','PB8为方向/LED行1复用输入，PB9由LED COM4驱动')
    s=s.replace('//\t\tBsp_Key_Init();', '\t\tBsp_Key_Init();')
    # A disabled communication build must not reconfigure EN if a legacy caller remains.
    start=s.index('void Bsp_Com_Tx_Mode(uint8_t uart_enable)')
    end=s.index('\n}\n',start)
    body=s[start:end]
    body=body.replace('{\n', '{\n#if BAT_COM_EN\n',1)+'\n#else\n    (void)uart_enable;\n#endif'
    s=s[:start]+body+s[end:]
    return s

def parameters(s):
    s=s.replace('#define LED_SCAN_EN       (0)', '#define LED_SCAN_EN       (1)  // 新板7灯与方向/按键分时复用')
    s=s.replace('#define MOS_NTC_10K_3435K 1', '#define MOS_NTC_10K_3435K 0')
    s=s.replace('#define MOS_NTC_100K_3950K 0', '#define MOS_NTC_100K_3950K 1')
    s=s.replace('MOS_TEMP_UP_RES        \t       \t10.0f','MOS_TEMP_UP_RES        \t       \t4.7f') if False else s
    import re
    s=re.sub(r'(#define MOS_TEMP_UP_RES\s+)10\.0f', r'\g<1>4.7f', s)
    s=re.sub(r'(#define EN_BAT_TEMP_DETECT\s+)\(1\)', r'\g<1>(0)', s)
    s=s.replace('/* BAT温度保护功能使能 */','/* DS/TBAT仅预留；用户确认本版不启用电池温度保护 */')
    return s

def user_h(s):
    s=s.replace('uint8_t\t\tKey_Data;', 'volatile uint8_t\t\tKey_Data;')
    s=s.replace('uint8_t\t\tDir_Data;', 'volatile uint8_t\t\tDir_Data;')
    s=s.replace('uint8_t \tled_scan_start_flag;', 'volatile uint8_t \tled_scan_start_flag;')
    s=s.replace('\tuint16_t \tLoad_exist;', '\tvolatile uint8_t led_input_valid; // 复用输入至少完成一次稳定采样\n\tuint16_t \tLoad_exist;')
    s=s.replace('方向有效：恰好一个方向引脚生效（两同高/同低为无效）','方向已稳定确认，单线DIRCTION输入')
    return s

def user_c(s):
    start=s.index('void user_direction_handle(void)')
    end=s.index('/**',start)
    s=s[:start]+'''void user_direction_handle(void)
{
\tstatic uint8_t candidate_dir=0;
\tstatic uint8_t stable_count=0;
\tuint8_t requested_dir;

\tuser_list.direction_ready=0;
\tif(!user_list.led_input_valid)
\t{
\t\tstable_count=0;
\t\tuser_list.dir_valid=0;
\t\tuser_list.dir_retrig_need=1;
\t\treturn;
\t}
\t/* 参考板：低电平正转，高电平反转；禁止直接读取正在点灯的PB8。 */
\trequested_dir=user_list.Dir_Data ? Tool_Dir_CCW_Value : Tool_Dir_CW_Value;
\tif(requested_dir != candidate_dir)
\t{
\t\tcandidate_dir=requested_dir;
\t\tstable_count=0;
\t}
\tif(stable_count < 10)
\t{
\t\tstable_count++;
\t}
\tif(stable_count < 10)
\t{
\t\treturn;
\t}
\t/* 运行中检测到换向先停机，保持原换相方向直到停稳并松开扳机。 */
\tif(requested_dir != mc_core_list.mc_dir)
\t{
\t\tuser_list.dir_valid=0;
\t\tuser_list.dir_retrig_need=1;
\t}
\tif(motor_control_list.bldc_state != MOTOR_STOP ||
\t   mc_core_list.motor_en || user_list.flag.bits.gToolEn)
\t{
\t\treturn;
\t}
\tif(user_list.flag.bits.gToolTrigger==0)
\t{
\t\tuser_list.dir_retrig_need=0;
\t}
\tif(user_list.dir_retrig_need)
\t{
\t\treturn;
\t}
\tuser_list.flag.bits.gToolDir=requested_dir;
\tmc_core_list.mc_dir=requested_dir;
\tuser_list.dir_valid=1;
\tuser_list.direction_ready=1;
}
'''+s[end:]
    s=s.replace('//\tuser_list.key_adc=(uint16_t)GET_ADC_VALUE(DIR_CHANNEL);', '// 按键与LED行2复用，只使用扫描窗口采样值。')
    s=s.replace('\tu8 res=0;\n', '\tu8 res=0;\n\tif(!user_list.led_input_valid)\n\t{\n\t\tkey_count=0;\n\t\treturn 0;\n\t}\n',1)
    s=s.replace('运行中方向变为无效（两引脚同时生效或同时不生效）立即停机','运行中方向变化确认或复用采样失效，撤销运行许可')
    # Temperature counters belong to their enabled feature, no unused warnings in reserved mode.
    return s

edit('USER/inc/board.h', board_h)
edit('USER/board.c', board_c)
edit('USER/inc/parameter.h', parameters)
edit('USER/inc/Bat_com.h', lambda s: s.replace('#define BAT_COM_EN                     (1)', '#ifndef BAT_COM_EN\n#define BAT_COM_EN                     (0)').replace('/* 电池通信功能总开关，0 时许可恒为通过 */','/* 新板无通信电路，PA11/12/14已用于EN/照明/KEY */\n#endif'))
edit('USER/inc/user_control.h', user_h)
edit('USER/user_control.c', user_c)
edit('USER/mm32_it.c', lambda s: replace(s, '/* LED 复用扫描（LED_Scan）已随单灯硬件取消，见 led.h */', '#if LED_SCAN_EN\n\t\tLED_Scan();\n#endif').replace('\tmc_core_list.Board_ADC_DATA.u16NTC_MOS \t\t\t= (uint16_t)GET_ADC_VALUE(T_MOS_CHANNEL);', '\tmc_core_list.Board_ADC_DATA.u16NTC_MOS \t\t\t= (uint16_t)GET_ADC_VALUE(T_MOS_CHANNEL);\n\tmc_core_list.Board_ADC_DATA.u16NTC_BAT = (uint16_t)GET_ADC_VALUE(T_BAT_CHANNEL);'))
edit('USER/main.c', lambda s: s.replace('EN = PA13','EN = PA11').replace('原 PA12 现归 USART RX','PA12 现用于照明输出').replace('\t\tuser_direction_handle();\n\t}', '\t\tsystick_delay_us(1000); // 必须让复用输入产生新的采样\n\t\tuser_direction_handle();\n\t}',1))

print('Updated board IO, cached direction/key inputs, reserved TBAT and MOS hardware parameters.')
