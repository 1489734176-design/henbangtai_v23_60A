#include "main.h"
#include "board.h"
#include "user_control.h"
#include "parameter.h"
#include "motor_control.h"
#include "led.h"
#include "Bat_com.h"
User_list_Struct user_list;
Machine_ERROR_HOLD machine_error_hold;
Mc_core_list mc_core_list;
Motor_Control_list motor_control_list;
void user_direction_handle(void)
{
	static uint8_t candidate_dir=0;
	static uint8_t stable_count=0;
	uint8_t requested_dir;

	user_list.direction_ready=0;
	if(!user_list.led_input_valid)
	{
		stable_count=0;
		user_list.dir_valid=0;
		user_list.dir_retrig_need=1;
		return;
	}
	/* 参考板：低电平正转，高电平反转；禁止直接读取正在点灯的PB8。 */
	requested_dir=user_list.Dir_Data ? Tool_Dir_CCW_Value : Tool_Dir_CW_Value;
	if(requested_dir != candidate_dir)
	{
		candidate_dir=requested_dir;
		stable_count=0;
	}
	if(stable_count < 10)
	{
		stable_count++;
	}
	if(stable_count < 10)
	{
		return;
	}
	/* 首次上电只是同步开关方向，不是运行换向；无需松开供电扳机。
	   仍须采样有效、完成去抖且电机停止，所有故障许可由原状态机检查。 */
	if(!user_list.direction_initialized)
	{
		if(motor_control_list.bldc_state==MOTOR_STOP &&
		   !mc_core_list.motor_en && !user_list.flag.bits.gToolEn)
		{
			user_list.flag.bits.gToolDir=requested_dir;
			mc_core_list.mc_dir=requested_dir;
			user_list.direction_initialized=1;
			user_list.dir_retrig_need=0;
			user_list.dir_valid=1;
			user_list.direction_ready=1;
		}
		return;
	}
	/* 运行中检测到换向先停机，保持原换相方向直到停稳并松开扳机。 */
	if(requested_dir != mc_core_list.mc_dir)
	{
		user_list.dir_valid=0;
		user_list.dir_retrig_need=1;
	}
	if(motor_control_list.bldc_state != MOTOR_STOP ||
	   mc_core_list.motor_en || user_list.flag.bits.gToolEn)
	{
		return;
	}
	if(user_list.flag.bits.gToolTrigger==0)
	{
		user_list.dir_retrig_need=0;
	}
	if(user_list.dir_retrig_need)
	{
		return;
	}
	user_list.flag.bits.gToolDir=requested_dir;
	mc_core_list.mc_dir=requested_dir;
	user_list.dir_valid=1;
	user_list.direction_ready=1;
}
u8 key_scan(void)
{
	static u16 key_count=0;
	u8 res=0;
	if(!user_list.led_input_valid)
	{
		key_count=0;
		return 0;
	}
// 按键与LED行2复用，只使用扫描窗口采样值。
		user_list.key_adc =user_list.Key_Data;
	if(!user_list.key_adc)
	{
		if(key_count<2000)
		{
			key_count++;
		}

		if(key_count==1000)  // 长按1000ms判定
		{
			res=2;
		}
	}
	else 
	{
		if(key_count>50&&key_count<1000)  // 短按判定(50ms~1000ms)
		{
			res=1;
		}
		key_count=0;
	
	}
	return res;


}
void Bsp_Op_Init(void) {}
void Bsp_Comp_Init(void) {}
void Bsp_Adc_Init(void) {}
void Bsp_Pwm_Init(void) {}
void Bsp_Key_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHBPeriphClockCmd(KEY_RCC_CLOCKGPIO, ENABLE);
    GPIO_PinAFConfig(KEY_PORT, KEY_PIN_SRC, KEY_PIN_AF);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = KEY_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &gpio);
}
void Bsp_dir_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);



	/* LED Clock Enable */
    RCC_AHBPeriphClockCmd(DIR_RCC_CLOCKGPIO, ENABLE);		// OK
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_1);
	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStructure.GPIO_Pin     =  DIR_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_Init(DIR_PORT, &GPIO_InitStructure);


}
void Bsp_Gpio_Init(void)
{
    Bsp_Op_Init();
		Bsp_Comp_Init();
    Bsp_Adc_Init();
		Bsp_Pwm_Init();
//		led_init();

//		Bsp_LED_Init();
//		LED1_ON();
		/* DIR/KEY共享灯脚由校准后的led_init接管，先保留PA14的SWD启动窗口。 */
}
uint8_t Bat_Com_CanRun(void)
{
#if BAT_COM_EN
    return (uint8_t)(ComActive && HandshakeAck && RuntimeValid &&
                     BatComState == BAT_ST_LINKED &&
                     (uint32_t)(ComTickMs - SessionLastMs) < BAT_LINK_TIMEOUT_MS &&
                     !(BatData.Status & BAT_FAULT_MASK) &&
                     !user_list.userErr.bits.gBatComFault &&
                     !machine_error_hold.userErr_hold.bits.gBatComFault);
#else
    return 1;
#endif
}
void Bat_Com_Sleep(void)
{
#if BAT_COM_EN
    NVIC_DisableIRQ(USART1_IRQn);
    ComActive = 0;
    USART1->CR1 &= ~(USART_IT_RXNE | USART_IT_TXE | USART_IT_TC | USART_CR1_RE_Msk);
    USART_ITConfig(USART1, USART_IT_ERR, DISABLE);
    USART_Cmd(USART1, DISABLE);
    Bsp_Com_Tx_Mode(0);
    COM_EN_LOW();
    TxState = BAT_TX_IDLE;
    RxReady = 0;
    RxOverflow = 0;
    RxCnt = 0;
    HandshakeAck = 0;
    RuntimeValid = 0;
    BatComState = BAT_ST_IDLE;
    user_list.flag.bits.gToolEn = 0;
    mc_core_list.motor_en = 0;
    NVIC_ClearPendingIRQ(USART1_IRQn);
#endif
}
void Bat_Com_Restart(void)
{
#if BAT_COM_EN
    NVIC_DisableIRQ(USART1_IRQn);
    ComActive = 0;
    HandshakeAck = 0;
    RuntimeValid = 0;
    BatComState = BAT_ST_IDLE;
    SessionLastMs = ComTickMs;
    user_list.flag.bits.gToolEn = 0;
    mc_core_list.motor_en = 0;
    COM_EN_HIGH();
    Bat_ResetTransport();
    ComActive = 1;
    NVIC_EnableIRQ(USART1_IRQn);
#endif
}
void user_state_control(void)
{
	/* 电池通信许可撤销时立即清工具使能，运行中则回到停止态 */
	if(!Bat_Com_CanRun())
	{
		user_list.flag.bits.gToolEn=0;
		if(user_list.user_state == TOOL_RUN)
		{
			user_list.user_state=TOOL_STOP;
		}
	}
	switch(user_list.user_state)
	{
		case TOOL_STOP:
			user_list.flag.bits.gToolEn=0;
			user_list.Load_exist=0;
			if(++user_list.sleep_cnt>SLEEP_TIME)  // 休眠超时，进入低功耗
			{
				/* 休眠前先停用通信释放总线，随后 break 防止后续逻辑覆盖 POWER_DOWN */
				Bat_Com_Sleep();
				user_list.user_state=TOOL_POWER_DOWN;
				user_list.sleep_cnt=0;
				break;
			}
//			if(!user_list.key_adc)
//			{
//				user_list.sleep_cnt=0;
//			}
			if(machine_error_hold.MC_error_hold.word!=0||machine_error_hold.userErr_hold.word!=0)
			{
				user_list.user_state=TOOL_ERROR_STOP;
			}
			else if(user_list.flag.bits.gAutoStop == 1)
			{
				if(user_list.flag.bits.gToolTrigger==0)
				{
					user_list.flag.bits.gAutoStop =0;
				}
			}
//			else if(user_list.flag.bits.gToolTrigger && user_list.direction_ready && Bat_Com_CanRun())
			else if(user_list.flag.bits.gToolTrigger &&
					user_list.dir_valid && !user_list.dir_retrig_need &&
					(user_list.direction_ready || motor_control_list.bldc_state != MOTOR_STOP) &&
					Bat_Com_CanRun())
			{
				user_list.flag.bits.gToolEn=1;
				user_list.user_state=TOOL_RUN;
				user_list.sleep_cnt=0;
			}
		break;
			
		case TOOL_ERROR_STOP:
			user_list.flag.bits.gToolEn=0;
//			user_list.Load_exist=0;
			if(++user_list.sleep_cnt>SLEEP_TIME)
			{
				/* 报错停机下同样先停用通信再进入休眠，break 保护 POWER_DOWN 不被覆盖 */
				Bat_Com_Sleep();
				user_list.user_state=TOOL_POWER_DOWN;
				user_list.sleep_cnt=0;
				break;
			}
//			if(!user_list.key_adc)
//			{
//				user_list.sleep_cnt=0;
//			}
			if(user_list.flag.bits.gToolTrigger==0)
			{
				if(user_list.userErr.word==0)
				{
					if(	machine_error_hold.userErr_hold.bits.gIBusAcmpOCP==1&&motor_control_list.bldc_state==MOTOR_STOP)
					{
						machine_error_hold.userErr_hold.bits.gIBusAcmpOCP=0;
						TIM_CtrlPWMOutputs(TIM1, ENABLE);
					}
					#if 0
					mc_core_list.MC_error.word=0;
					machine_error_hold.MC_error_hold.word=0;
					#else
					mc_core_list.MC_error.word&=(0x00000004);//��ת������ֱ����
					machine_error_hold.MC_error_hold.word&=(0x00000004);	
					if(mc_core_list.MC_error.bits.gBlock==0) //mc_core_list.MC_error.bits.gBlockΪ0�����
					{
						machine_error_hold.MC_error_hold.bits.gBlock=0;
					}
					#endif
					machine_error_hold.userErr_hold.word&=0x00000001;
				}
				if(	machine_error_hold.MC_error_hold.word==0&&machine_error_hold.userErr_hold.word==0)
				{
					user_list.user_state=TOOL_STOP;
				}
//				user_list.userErr.word=0;
			}
			else
			{
				user_list.sleep_cnt=0;
			}
		break;
			
		case TOOL_RUN:
			#if(RESTAR_EN)
			if(machine_error_hold.userErr_hold.word!=0||((machine_error_hold.MC_error_hold.word&0xfffb) != 0)||\
			(machine_error_hold.MC_error_hold.bits.gBlock&&(user_list.Load_exist>=RESTAR_COUNT)))
			#else
			if(machine_error_hold.userErr_hold.word!=0||((machine_error_hold.MC_error_hold.word) != 0))
			#endif
			{
				user_list.user_state=TOOL_ERROR_STOP;
			}
			if(user_list.flag.bits.gToolTrigger==0)
			{
				user_list.user_state=TOOL_STOP;
			}
			/* 运行中方向变化确认或复用采样失效，撤销运行许可 */
			if(!user_list.dir_valid)
			{
				user_list.flag.bits.gToolEn=0;
				user_list.user_state=TOOL_STOP;
			}

			if(user_list.flag.bits.gAutoStop == 1)
			{
				user_list.flag.bits.gToolEn = 0;   /**/
//				user_list.ToolState_Cnt = 0;
				user_list.user_state = TOOL_STOP;    
			}
		break;
		
		case TOOL_POWER_DOWN:
			user_list.sleep_cnt=0;
			GPIO_ResetBits(EN_PORT, EN_PIN);  // 关闭电源使能（进入低功耗）
			if(user_list.flag.bits.gToolTrigger)
			{
				if(machine_error_hold.MC_error_hold.word!=0||machine_error_hold.userErr_hold.word!=0)
				{
					user_list.user_state = TOOL_ERROR_STOP;
				}
				else
				{
					user_list.user_state = TOOL_STOP;
				}
				GPIO_SetBits(EN_PORT, EN_PIN);  // 重新使能电源
				Bat_Com_Restart();             // 唤醒后重启通信，重新握手前电机不得运行
			}
		break;
			
		default:
		break;
	}
}
#include "D:/mym/NXN/9.henbangtai/dianzuan/core/NX32F002-V1_01-lib-release/tests/seven_led/test_seven_led.c"