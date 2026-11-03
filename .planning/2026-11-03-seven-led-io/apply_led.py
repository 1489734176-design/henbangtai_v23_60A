from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).parent))
# Import only the helpers, without executing the IO migration again.
namespace = {}
exec(Path(__file__).with_name('apply_io.py').read_text(encoding='utf-8').split("edit('USER/inc/board.h'")[0], {'__file__': __file__}, namespace)
# The helper globals need a single namespace.
ns = {'__file__': __file__}
exec(Path(__file__).with_name('apply_io.py').read_text(encoding='utf-8').split("edit('USER/inc/board.h'")[0], ns)
edit = ns['edit']
ROOT = ns['ROOT']
old = (ROOT/'USER/led.c').read_text(encoding='utf-8')
light = old[old.index('static uint8_t Light_Fault_Active'):old.index('/**\n * @brief  LED 显示调度')]
blink = old[old.index('void LedError_Run(void)'):]
ref = Path('D:/mym/NXN/1.shenghong/source code/20260909/NX32F002-V1_01-lib-release/USER/led.c').read_text(encoding='utf-8', errors='surrogateescape')
soc = ref[ref.index('void LedSoc_Run(uint32_t'):]
header = '''#ifndef __LED_H
#define __LED_H
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/* 新板：PB8=LED行1/DIRCTION，PA14=LED行2/KEY。
 * 阴极COM1/2/3/4=PA13/PA0/PA10/PB9；PA12独立照明。
 * 逻辑灯1..3对应物理LED4..6（电量/报警），逻辑灯4..6对应物理LED1..3（挡位）。 */
#define LED_LIGHT_RCC_CLOCKGPIO (RCC_AHBENR_GPIOA_Msk)
#define LED_LIGHT_PORT          GPIOA
#define LED_LIGHT_PIN           GPIO_Pin_12
#define LED_ROW1_PORT           GPIOB
#define LED_ROW1_PIN            GPIO_Pin_8
#define LED_ROW2_PORT           GPIOA
#define LED_ROW2_PIN            GPIO_Pin_14
#define LED_COM1_PIN            GPIO_Pin_13
#define LED_COM2_PIN            GPIO_Pin_0
#define LED_COM3_PIN            GPIO_Pin_10
#define LED_COM4_PIN            GPIO_Pin_9

typedef union
{
    uint8_t Word;
    struct
    {
        uint8_t gB0:1, gB1:1, gB2:1, gB3:1, gB4:1, gB5:1, gB6:1, gB7:1;
    } bits;
} BITS_UNION;
extern volatile BITS_UNION ledData;
'''
for n in range(1,8):
    header += f'#define LED{n}          ledData.bits.gB{n}\n#define LED{n}_ON       (LED{n}=1)\n#define LED{n}_OFF      (LED{n}=0)\n#define LED{n}_FLASH    (LED{n}=!LED{n})\n'
header += '''
#define LED_ERROR_ON()  do { LED1_ON; LED2_ON; LED3_ON; } while(0)
#define LED_ERROR_OFF() do { LED1_OFF; LED2_OFF; LED3_OFF; } while(0)

/* 1..12保持20260909参考程序编号；新增报警在12之后，兜底最后。 */
#define LED_FAULT_HW_OCP       (1)
#define LED_FAULT_BAT_TEMP     (2)
#define LED_FAULT_MOS_TEMP     (3)
#define LED_FAULT_BLOCK        (4)
#define LED_FAULT_SW_OCP       (5)
#define LED_FAULT_UVP          (6)
#define LED_FAULT_FLY_MACHINE  (7)
#define LED_FAULT_OVP          (8)
#define LED_FAULT_MOS_BRIDGE   (9)
#define LED_FAULT_LACK_PHASE   (10)
#define LED_FAULT_LOSE_STEP    (11)
#define LED_FAULT_BAT_COM      (12) /* 新板无通信，保留原编号 */
#define LED_FAULT_IBUS_OCP4    (13)
#define LED_FAULT_OTHER        (14)

typedef struct
{
    uint8_t LedDeal;
    uint8_t LedRunMode;
    uint16_t LedRunTimes;
    uint16_t LedRunCnt_Once;
    uint8_t gSocUpdate;
} User_LED_Control_t;

typedef enum { LIGHT_OFF=0, LIGHT_ON, LIGHT_WAIT_10S, LIGHT_WAIT_30S } Light_State_t;
typedef struct { Light_State_t state; uint32_t cnt; } Light_Ctrl_t;
extern User_LED_Control_t user_led_control;
extern Light_Ctrl_t light_ctrl;
void led_init(void);
void LED_Scan(void);
void UO_Led_Handler(void);
void LedSoc_Run(uint32_t volt_vx100, uint8_t display_en, uint8_t fast_mode);
uint8_t LedError_Control(void);
void LedError_Run(void);
void Light_Handler(void);
#endif
'''
code = '''#include "board.h"
#include "led.h"
#include "user_control.h"
#include "parameter.h"

User_LED_Control_t user_led_control;
Light_Ctrl_t light_ctrl;
volatile BITS_UNION ledData;
static uint8_t led_scan_index;

#define LIGHT_DELAY_NORMAL_MS (10000u)
#define LIGHT_DELAY_FAULT_MS  (30000u)

/* 换行前先断开所有阳极，再将公共阴极全拉高，避免切换瞬间串灯。 */
static void led_matrix_blank(void)
{
\tMODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 0);
\tMODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 0);
\tGPIO_SetBits(GPIOA, LED_COM1_PIN | LED_COM2_PIN | LED_COM3_PIN);
\tGPIO_SetBits(GPIOB, LED_COM4_PIN);
}

static void led_inputs_prepare(void)
{
\tGPIO_SetBits(GPIOB, LED_ROW1_PIN);
\tGPIO_SetBits(GPIOA, LED_ROW2_PIN);
\tMODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 2u << GPIO_CRH_CNF8_Pos);
\tMODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 2u << GPIO_CRH_CNF14_Pos);
}

void led_init(void)
{
\tGPIO_InitTypeDef gpio;
\tRCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk, ENABLE);
\t/* PA13/14退出SWD复用；PA11电源保持独立，不得在扫描中修改。 */
\tGPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_1);
\tGPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_1);
\tled_matrix_blank();
\tGPIO_StructInit(&gpio);
\tGPIO_ResetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
\tgpio.GPIO_Pin=LED_COM1_PIN | LED_COM2_PIN | LED_COM3_PIN | LED_LIGHT_PIN;
\tgpio.GPIO_Mode=GPIO_Mode_Out_PP;
\tgpio.GPIO_Speed=GPIO_Speed_High;
\tGPIO_Init(GPIOA, &gpio);
\tgpio.GPIO_Pin=LED_COM4_PIN;
\tGPIO_Init(GPIOB, &gpio);
\tled_inputs_prepare();
\tledData.Word=0;
\tled_scan_index=0;
\tuser_list.led_input_valid=0;
\tuser_list.Key_Data=1;
\tuser_list.Dir_Data=1;
\tlight_ctrl.state=LIGHT_OFF;
\tlight_ctrl.cnt=0;
}

/* TIM1每个PWM周期调用：7个点灯槽+输入准备槽+输入采样槽。
 * PWM=16kHz时，输入上拉稳定62.5us；整轮562.5us，不占用ADC引脚。 */
void LED_Scan(void)
{
\tuint8_t col;
\tif(!user_list.led_scan_start_flag)
\t{
\t\tled_matrix_blank();
\t\tled_inputs_prepare();
\t\tled_scan_index=0;
\t\tuser_list.led_input_valid=0;
\t\treturn;
\t}
\tif(++led_scan_index>9)
\t{
\t\tled_scan_index=1;
\t}
\tif(led_scan_index==9)
\t{
\t\tuser_list.Dir_Data=GPIO_ReadInputDataBit(DIR_PORT, DIR_PIN);
\t\tuser_list.Key_Data=GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN);
\t\tuser_list.led_input_valid=1;
\t\treturn;
\t}
\tled_matrix_blank();
\tif(led_scan_index==8)
\t{
\t\tled_inputs_prepare();
\t\treturn;
\t}
\tif(user_list.user_state==TOOL_POWER_DOWN || !(ledData.Word & (1u<<led_scan_index)))
\t{
\t\treturn;
\t}
\tif(led_scan_index>=4 && led_scan_index<=6)
\t{
\t\tGPIO_SetBits(GPIOB, LED_ROW1_PIN);
\t\tMODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 1u<<GPIO_CRH_MODE8_Pos);
\t\tcol=led_scan_index-4;
\t}
\telse
\t{
\t\tGPIO_SetBits(GPIOA, LED_ROW2_PIN);
\t\tMODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 1u<<GPIO_CRH_MODE14_Pos);
\t\tcol=(led_scan_index==7) ? 3 : led_scan_index-1;
\t}
\tswitch(col)
\t{
\t\tcase 0: GPIO_ResetBits(GPIOA, LED_COM1_PIN); break;
\t\tcase 1: GPIO_ResetBits(GPIOA, LED_COM2_PIN); break;
\t\tcase 2: GPIO_ResetBits(GPIOA, LED_COM3_PIN); break;
\t\tdefault: GPIO_ResetBits(GPIOB, LED_COM4_PIN); break;
\t}
}

'''+light+'''
void UO_Led_Handler(void)
{
\tLight_Handler();
\tif(light_ctrl.state==LIGHT_ON || light_ctrl.state==LIGHT_WAIT_10S ||
\t   (light_ctrl.state==LIGHT_WAIT_30S && light_ctrl.cnt<LIGHT_DELAY_FAULT_MS))
\t{
\t\tGPIO_SetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
\t}
\telse
\t{
\t\tGPIO_ResetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
\t}
\tif(user_list.user_state==TOOL_POWER_DOWN)
\t{
\t\tledData.Word=0;
\t\tuser_led_control.LedRunTimes=0;
\t\tuser_led_control.LedDeal=0;
\t\treturn;
\t}
\tif(LedError_Control() && user_list.flag.bits.gToolTrigger)
\t{
\t\tLedError_Run();
\t}
\telse
\t{
\t\tuser_led_control.LedDeal=0;
\t\tuser_led_control.LedRunTimes=0;
\t\tLedSoc_Run(user_list.VBus_Vx100, 1, 0);
\t}
#if Wrench_EN
\tif(mc_core_list.mc_dir==Tool_Dir_CW_Value)
\t{
\t\tLED4_ON;
\t\tLED5=(user_list.gears_level>=1);
\t\tLED6=(user_list.gears_level>=2);
\t}
\telse
\t{
\t\tLED4_OFF;
\t\tLED5=(user_list.flag.bits.gAutoStopEn!=0);
\t\tLED6_OFF;
\t}
#else
\tLED4_OFF;
\tLED5_OFF;
\tLED6_OFF;
#endif
\t/* LED7扫描已接入；正常显示用途待确认，暂不增加产品规则。 */
\tLED7_OFF;
}

uint8_t LedError_Control(void)
{
#if RESTAR_EN
\tif(machine_error_hold.userErr_hold.word==0 &&
\t   (machine_error_hold.MC_error_hold.word & 0xfffb)==0 &&
\t   (!machine_error_hold.MC_error_hold.bits.gBlock || user_list.Load_exist<RESTAR_COUNT))
#else
\tif(machine_error_hold.userErr_hold.word==0 && machine_error_hold.MC_error_hold.word==0)
#endif
\t{
\t\tuser_led_control.LedRunCnt_Once=0;
\t\tuser_led_control.LedDeal=0;
\t\tuser_led_control.LedRunTimes=0;
\t\treturn 0;
\t}
\tif(!user_led_control.LedDeal && !user_led_control.LedRunTimes)
\t{
\t\tuser_led_control.LedDeal=1;
\t\tuser_led_control.LedRunTimes=10;
\t\tuser_led_control.LedRunMode=1;
'''
conditions=[
('machine_error_hold.userErr_hold.bits.gIBusAcmpOCP','HW_OCP'),
('machine_error_hold.userErr_hold.bits.gNtcBat','BAT_TEMP'),
('machine_error_hold.userErr_hold.bits.gNtcMos || machine_error_hold.userErr_hold.bits.gMos_overload','MOS_TEMP'),
('machine_error_hold.MC_error_hold.bits.gBlock','BLOCK'),
('machine_error_hold.userErr_hold.bits.gIBusPeakOCP1 || machine_error_hold.userErr_hold.bits.gIBusAvgOCP1 ||\n\t\t        machine_error_hold.userErr_hold.bits.gIBusAvgOCP2 || machine_error_hold.userErr_hold.bits.gIBusAvgOCP3','SW_OCP'),
('machine_error_hold.userErr_hold.bits.gVBusStbUVP || machine_error_hold.userErr_hold.bits.gVBusRunUVP1 ||\n\t\t        machine_error_hold.userErr_hold.bits.gVBusRunUVP2','UVP'),
('machine_error_hold.userErr_hold.bits.fly_machine','FLY_MACHINE'),
('machine_error_hold.userErr_hold.bits.gVBusOVP','OVP'),
('machine_error_hold.userErr_hold.bits.gMosBridgeH || machine_error_hold.userErr_hold.bits.gMosBridgeL','MOS_BRIDGE'),
('machine_error_hold.MC_error_hold.bits.gLackPhase','LACK_PHASE'),
('machine_error_hold.MC_error_hold.bits.gLosStep','LOSE_STEP'),
('machine_error_hold.userErr_hold.bits.gBatComFault','BAT_COM'),
('machine_error_hold.userErr_hold.bits.gIBusAvgOCP4','IBUS_OCP4')]
for i,(condition,fault) in enumerate(conditions):
    code += f'\t\t{"if" if i==0 else "else if"}({condition})\n\t\t{{\n\t\t\tuser_led_control.LedRunCnt_Once=LED_FAULT_{fault};\n\t\t}}\n'
code+='''\t\telse
\t\t{
\t\t\tuser_led_control.LedRunCnt_Once=LED_FAULT_OTHER;
\t\t}
\t}
\treturn (user_led_control.LedRunCnt_Once!=0);
}

'''+blink+'\nstatic const uint16_t deltaVolt=20; /* 0.2V滞回，沿用参考程序 */\n'+soc
edit('USER/inc/led.h', lambda s: header)
edit('USER/led.c', lambda s: code)
print('Updated 7 LED matrix scanning, reference display and 1..14 fault mapping.')
