#include "board.h"
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
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 0);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 0);
	GPIO_SetBits(GPIOA, LED_COM1_PIN | LED_COM2_PIN | LED_COM3_PIN);
	GPIO_SetBits(GPIOB, LED_COM4_PIN);
}

static void led_inputs_prepare(void)
{
	GPIO_SetBits(GPIOB, LED_ROW1_PIN);
	GPIO_SetBits(GPIOA, LED_ROW2_PIN);
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 2u << GPIO_CRH_CNF8_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 2u << GPIO_CRH_CNF14_Pos);
}

void led_init(void)
{
	GPIO_InitTypeDef gpio;
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk, ENABLE);
	/* PA13/14退出SWD复用；PA11电源保持独立，不得在扫描中修改。 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_1);
	led_matrix_blank();
	GPIO_StructInit(&gpio);
	GPIO_ResetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
	gpio.GPIO_Pin=LED_COM1_PIN | LED_COM2_PIN | LED_COM3_PIN | LED_LIGHT_PIN;
	gpio.GPIO_Mode=GPIO_Mode_Out_PP;
	gpio.GPIO_Speed=GPIO_Speed_High;
	GPIO_Init(GPIOA, &gpio);
	gpio.GPIO_Pin=LED_COM4_PIN;
	GPIO_Init(GPIOB, &gpio);
	led_inputs_prepare();
	ledData.Word=0;
	led_scan_index=0;
	user_list.led_input_valid=0;
	user_list.Key_Data=1;
	user_list.Dir_Data=1;
	light_ctrl.state=LIGHT_OFF;
	light_ctrl.cnt=0;
}

/* TIM1每个PWM周期调用：7个点灯槽+输入准备槽+输入采样槽。
 * PWM=16kHz时，输入上拉稳定62.5us；整轮562.5us，不占用ADC引脚。 */
void LED_Scan(void)
{
	uint8_t col;
	if(!user_list.led_scan_start_flag)
	{
		led_matrix_blank();
		led_inputs_prepare();
		led_scan_index=0;
		user_list.led_input_valid=0;
		return;
	}
	if(++led_scan_index>9)
	{
		led_scan_index=1;
	}
	if(led_scan_index==9)
	{
		user_list.Dir_Data=GPIO_ReadInputDataBit(DIR_PORT, DIR_PIN);
		user_list.Key_Data=GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN);
		user_list.led_input_valid=1;
		return;
	}
	led_matrix_blank();
	if(led_scan_index==8)
	{
		led_inputs_prepare();
		return;
	}
	if(user_list.user_state==TOOL_POWER_DOWN || !(ledData.Word & (1u<<led_scan_index)))
	{
		return;
	}
	if(led_scan_index>=4 && led_scan_index<=6)
	{
		GPIO_SetBits(GPIOB, LED_ROW1_PIN);
		MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8, 1u<<GPIO_CRH_MODE8_Pos);
		col=led_scan_index-4;
	}
	else
	{
		GPIO_SetBits(GPIOA, LED_ROW2_PIN);
		MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE14 | GPIO_CRH_CNF14, 1u<<GPIO_CRH_MODE14_Pos);
		col=(led_scan_index==7) ? 3 : led_scan_index-1;
	}
	switch(col)
	{
		case 0: GPIO_ResetBits(GPIOA, LED_COM1_PIN); break;
		case 1: GPIO_ResetBits(GPIOA, LED_COM2_PIN); break;
		case 2: GPIO_ResetBits(GPIOA, LED_COM3_PIN); break;
		default: GPIO_ResetBits(GPIOB, LED_COM4_PIN); break;
	}
}

static uint8_t Light_Fault_Active(void)
{
    return (uint8_t)((machine_error_hold.userErr_hold.word != 0) ||
                     (machine_error_hold.MC_error_hold.word != 0));
}

/**
 * @brief  照明功能状态机
 *        故障时松开开关  -> 立即熄灭
 *        无故障时松开开关-> 延时 10s 熄灭
 *        故障时不松开开关-> 延时 30s 熄灭
 *        无故障且按住    -> 点亮
 */
void Light_Handler(void)
{
    uint8_t fault = Light_Fault_Active();
    uint8_t trig  = user_list.flag.bits.gToolTrigger;

    if(user_list.user_state == TOOL_POWER_DOWN)
    {
        light_ctrl.state = LIGHT_OFF;
        light_ctrl.cnt   = 0;
        return;
    }

    if(trig)
    {
        if(fault)
        {
            if(light_ctrl.state != LIGHT_WAIT_30S)
            {
                light_ctrl.state = LIGHT_WAIT_30S;
                light_ctrl.cnt   = 0;
            }
            if(light_ctrl.cnt < LIGHT_DELAY_FAULT_MS)
            {
                light_ctrl.cnt++;
            }
        }
        else
        {
            light_ctrl.state = LIGHT_ON;
            light_ctrl.cnt   = 0;
        }
    }
    else if(fault || light_ctrl.state == LIGHT_WAIT_30S)
    {
        /* 松手时故障锁存可能已清除，仍须立即结束故障显示。 */
        light_ctrl.state = LIGHT_OFF;
        light_ctrl.cnt   = 0;
    }
    else
    {
        if(light_ctrl.state == LIGHT_ON)
        {
            light_ctrl.state = LIGHT_WAIT_10S;
            light_ctrl.cnt   = 0;
        }
        if(light_ctrl.state == LIGHT_WAIT_10S)
        {
            if(++light_ctrl.cnt >= LIGHT_DELAY_NORMAL_MS)
            {
                light_ctrl.state = LIGHT_OFF;
            }
        }
    }
}


void UO_Led_Handler(void)
{
	Light_Handler();
	if(light_ctrl.state==LIGHT_ON || light_ctrl.state==LIGHT_WAIT_10S ||
	   (light_ctrl.state==LIGHT_WAIT_30S && light_ctrl.cnt<LIGHT_DELAY_FAULT_MS))
	{
		GPIO_SetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
	}
	else
	{
		GPIO_ResetBits(LED_LIGHT_PORT, LED_LIGHT_PIN);
	}
	if(user_list.user_state==TOOL_POWER_DOWN)
	{
		ledData.Word=0;
		user_led_control.LedRunTimes=0;
		user_led_control.LedDeal=0;
		return;
	}
	if(LedError_Control() && user_list.flag.bits.gToolTrigger)
	{
		LedError_Run();
	}
	else
	{
		user_led_control.LedDeal=0;
		user_led_control.LedRunTimes=0;
		LedSoc_Run(user_list.VBus_Vx100, 1, 0);
	}
#if Wrench_EN
	if(mc_core_list.mc_dir==Tool_Dir_CW_Value)
	{
		LED4_ON;
		LED5=(user_list.gears_level>=1);
		LED6=(user_list.gears_level>=2);
	}
	else
	{
		LED4_OFF;
		LED5=(user_list.flag.bits.gAutoStopEn!=0);
		LED6_OFF;
	}
#else
	LED4_OFF;
	LED5_OFF;
	LED6_OFF;
#endif
	/* LED7扫描已接入；正常显示用途待确认，暂不增加产品规则。 */
	LED7_OFF;
}

uint8_t LedError_Control(void)
{
#if RESTAR_EN
	if(machine_error_hold.userErr_hold.word==0 &&
	   (machine_error_hold.MC_error_hold.word & 0xfffb)==0 &&
	   (!machine_error_hold.MC_error_hold.bits.gBlock || user_list.Load_exist<RESTAR_COUNT))
#else
	if(machine_error_hold.userErr_hold.word==0 && machine_error_hold.MC_error_hold.word==0)
#endif
	{
		user_led_control.LedRunCnt_Once=0;
		user_led_control.LedDeal=0;
		user_led_control.LedRunTimes=0;
		return 0;
	}
	if(!user_led_control.LedDeal && !user_led_control.LedRunTimes)
	{
		user_led_control.LedDeal=1;
		user_led_control.LedRunTimes=10;
		user_led_control.LedRunMode=1;
		if(machine_error_hold.userErr_hold.bits.gIBusAcmpOCP)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_HW_OCP;
		}
		else if(machine_error_hold.userErr_hold.bits.gNtcBat)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_BAT_TEMP;
		}
		else if(machine_error_hold.userErr_hold.bits.gNtcMos || machine_error_hold.userErr_hold.bits.gMos_overload)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_MOS_TEMP;
		}
		else if(machine_error_hold.MC_error_hold.bits.gBlock)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_BLOCK;
		}
		else if(machine_error_hold.userErr_hold.bits.gIBusPeakOCP1 || machine_error_hold.userErr_hold.bits.gIBusAvgOCP1 ||
		        machine_error_hold.userErr_hold.bits.gIBusAvgOCP2 || machine_error_hold.userErr_hold.bits.gIBusAvgOCP3)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_SW_OCP;
		}
		else if(machine_error_hold.userErr_hold.bits.gVBusStbUVP || machine_error_hold.userErr_hold.bits.gVBusRunUVP1 ||
		        machine_error_hold.userErr_hold.bits.gVBusRunUVP2)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_UVP;
		}
		else if(machine_error_hold.userErr_hold.bits.fly_machine)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_FLY_MACHINE;
		}
		else if(machine_error_hold.userErr_hold.bits.gVBusOVP)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_OVP;
		}
		else if(machine_error_hold.userErr_hold.bits.gMosBridgeH || machine_error_hold.userErr_hold.bits.gMosBridgeL)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_MOS_BRIDGE;
		}
		else if(machine_error_hold.MC_error_hold.bits.gLackPhase)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_LACK_PHASE;
		}
		else if(machine_error_hold.MC_error_hold.bits.gLosStep)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_LOSE_STEP;
		}
		else if(machine_error_hold.userErr_hold.bits.gBatComFault)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_BAT_COM;
		}
		else if(machine_error_hold.userErr_hold.bits.gIBusAvgOCP4)
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_IBUS_OCP4;
		}
		else
		{
			user_led_control.LedRunCnt_Once=LED_FAULT_OTHER;
		}
	}
	return (user_led_control.LedRunCnt_Once!=0);
}

void LedError_Run(void)
{
    static uint16_t bzCnt  = 0;     /* 当前剩余闪烁段数 */
    static uint16_t bzTime = 0;     /* 段内计时 */

    if(user_led_control.LedDeal)
    {
        LED_ERROR_OFF();

        bzCnt  = user_led_control.LedRunCnt_Once;
        bzTime = 0;

        user_led_control.LedDeal = 0;
    }

    if(!user_led_control.LedRunTimes)
    {
        LED_ERROR_OFF();

        bzCnt  = 0;
        bzTime = 0;
        return;
    }

    if(bzCnt)
    {
        bzTime++;

        if((bzTime >= 1) && (bzTime <= 500))        /* 灭 500ms */
        {
            LED_ERROR_OFF();
        }
        else if(bzTime < 700)                       /* 亮 200ms */
        {
            LED_ERROR_ON();
        }

        if(user_led_control.LedRunMode == 0)
        {
            if((bzTime >= 700) && (bzTime <= 900))
            {
                LED_ERROR_OFF();
            }
            else if((bzTime >= 900) && (bzTime <= 1100))
            {
                LED_ERROR_ON();
            }

            if(bzTime > 1100)
            {
                bzTime = 0;
                bzCnt--;
            }
        }
        else
        {
            if(bzTime > 700)
            {
                bzTime = 0;
                bzCnt--;
            }
        }
    }
    else
    {
        LED_ERROR_OFF();

        if(bzTime++ >= 1500)                        /* 轮间隔 1500ms */
        {
            bzTime = 0;
            bzCnt  = user_led_control.LedRunCnt_Once;
            user_led_control.LedRunTimes--;
        }
    }
}

static const uint16_t deltaVolt=20; /* 0.2V滞回，沿用参考程序 */
void LedSoc_Run(uint32_t volt_vx100,uint8_t display_en,uint8_t fast_mode)
{
	static uint16_t ms500cnt = 0;    // 500ms时间基准计数器
	static uint16_t msUpdateCnt = 0; // SOC更新定时器
	static uint8_t PackSocStatus = 0;// 当前SOC等级（0~4）

	//10次滤波增加稳定性
	ms500cnt++;

	/* 扳机上电后可能直接进入运行，首次电量不能等待停机刷新。
	   用当前电压直接初始化等级；后续仍按参考的停机更新/滞回规则处理。 */
	if(PackSocStatus == 0)
	{
		if(volt_vx100 > SOC3_MV)
		{
			PackSocStatus = 4;
		}
		else if(volt_vx100 > SOC2_MV)
		{
			PackSocStatus = 3;
		}
		else if(volt_vx100 > SOC1_MV)
		{
			PackSocStatus = 2;
		}
		else
		{
			PackSocStatus = 1;
		}
	}

	// 需要更新SOC或快速刷新模式
	if(user_led_control.gSocUpdate||fast_mode)
	{
		// 滞回比较状态机，根据电压判断SOC等级
		switch(PackSocStatus)
		{
			case 4:

				if(volt_vx100< (SOC3_MV -deltaVolt ))  // 滞回比较（防止边界抖动）
				{
					PackSocStatus = 3;
				}

				break;

			case 3:
		        if(volt_vx100> (SOC3_MV  ))
		        {
		            PackSocStatus = 4;
		        }
		        if(volt_vx100< (SOC2_MV -deltaVolt ))
		        {
		            PackSocStatus = 2;
		        }
		        break;

			case 2:
		        if(volt_vx100> (SOC2_MV  ))
		        {
		            PackSocStatus = 3;
		        }
		        if(volt_vx100< (SOC1_MV -deltaVolt ))
		        {
		            PackSocStatus = 1;
		        }
		        break;

			case 1:
		        if(volt_vx100> (SOC1_MV  ))
		        {
		            PackSocStatus = 2;
		        }

		        break;
		}

		user_led_control.gSocUpdate = 0; // 清除更新标志

	}
	else
	{
		// 非运行状态下累计计时
		if(user_list.user_state!=TOOL_RUN)
		{
			msUpdateCnt++;
		}
		else
		{
			msUpdateCnt=0;
		}
		// 每20个周期触发一次SOC更新
		if(msUpdateCnt> 20)
		{
				user_led_control.gSocUpdate = 1;
		    msUpdateCnt =0;
		}
	}

	// 500ms基准计时归零
	if(ms500cnt >= 500)  //500ms
	{
		ms500cnt  = 0;
	}


	/*soc display*/
	// 显示未使能，关闭所有LED并返回
	if(display_en == 0)
	{
		LED3_OFF;
		LED2_OFF;
		LED1_OFF;

		return;
	}

	// 根据SOC等级控制LED显示
	switch(PackSocStatus)
	{
		case 4 :
		    LED1_ON;
		    LED2_ON;
			LED3_ON;
		break;

		case 3 :
		    LED1_ON;
		    LED2_ON;
			LED3_OFF;
		break;

		case 2 :
		    LED1_ON;
		    LED2_OFF;
			LED3_OFF;
		break;

		case 1 :
			if(ms500cnt == 250)
			{
				LED1_FLASH;
			}
			LED2_OFF;
			LED3_OFF;
		break;

		case 0 :
		        LED1_OFF;
		        LED2_OFF;
		        LED3_OFF;
		break;

		default :
		break;

	}
}
