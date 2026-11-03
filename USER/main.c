/**
 * @file     main.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides the main functions and test samples.
 *
 * @attention
 *
 * THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
 * CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
 * TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
 * HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
 * CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
 *
 * <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
 */

/** Define to prevent recursive inclusion */
#define _MAIN_C_

/** Files includes */
#include "main.h"
#include "drv_inc.h"
#include "board.h"
#include "user_control.h"
#include "drv_pwm.h"
#include "parameter.h"
#include "mc_core.h"
#include "systick.h"
#include "motor_control.h"
#include "flash_data_save.h"
#include "led.h"
#include "Bat_com.h"

/**
 * @addtogroup MM32_User_Layer
 * @{
 */

//DiagFlag 								DiagnoseFAULT;
//Motor_TypeDef 					Motor_1st;
//UserFilt_TypeDef				Motor_User;

//LoopCMP_T 							RPValue;
//MovingAvgData 					SpeedFdk;
//NormalizationType 			RP;
//NormalizationType 			SPEEDRPM_Cmd;
//LoopCMP_T 							SPEEDRPM_Cmd_Value;

//ADC_OnflyDetect_TypeDef	ADC_OnflyDetect_lib;
//CMP_Onfly_TypeDef				CMP_Onfly_lib;
//BLDC_Control_TypeDef		BLDC_Control_lib;
//ADC_Zero_TypeDef				ADC_ZeroCross_lib;
//CMP_Zero_TypeDef				CMP_ZeroCross_lib;

/**
 * @addtogroup User_Main
 * @{
 */

/**
 * @brief  翻转指定GPIO引脚电平
 * @param  GPIOn: GPIO端口
 * @param  PINn:  引脚号
 */
void GPIO_Toggle(GPIO_TypeDef *GPIOn, uint16_t PINn)
{
    if (Bit_RESET == GPIO_ReadOutputDataBit(GPIOn, PINn))
    {
        GPIO_SetBits(GPIOn, PINn);
    }
    else
    {
        GPIO_ResetBits(GPIOn, PINn);
    }
}

/**
 * @brief  调试IO初始化（PA2推挽输出）
 */
void io_debug_init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA_Msk, ENABLE);

//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_7);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_1);
//	GPIO_WriteBit(GPIOA, LED_LIGHT , Bit_RESET);
    GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_2 ;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/**
 * @brief  主函数 - 系统初始化及主循环
 *         完成外设初始化、参数校准、电压/温度检测及电机控制状态机调度
 */
int main(void)
{
	/* Configure the clock */

	u8 i;
	//led_init();
	#if LIMIT_SPEED_EN
		u16 speed_pi_fer_count=0;
	#endif
	/* 初始化滴答定时器 */
	Systick_Init(0x00ffffff);
	
	/* 上电飞钻检测已取消（PA12 现用于照明输出） */

	
	//systick_delay_us(10000);//等待电源及ADC稳定
	
	
	//while(1);
	/* GPIO初始化（ADC、PWM、比较器、运放） */
	Bsp_Gpio_Init();

	/* 外设初始化（ADC、比较器、运放、PWM、定时器） */
	Peripheral_Init();
	/* 用户参数初始化（状态机、方向、错误标志） */
	user_para_init();

	systick_delay_us(100000);//等待ADC稳定
	/* 5V基准校准，计算VCC校准增益 */
	Vcc_5V_CAL();
	/* 电流偏置校准（在user_para_init之后执行） */
	CurrentOffsetCalibration();//需在user_para_init()之后
	/* 上电飞钻检测（在user_para_init之后执行） */
	mose_check();	//需在user_para_init()之后
	systick_delay_us(1000);
	/* 从Flash读取用户数据（挡位、自动停机设置） */
	read_user_data();//需在user_para_init()之后
	/* 电机控制核心初始化 */
	mc_init();

	/* 电源保持端口初始化（EN = PA11） */
	En_Port_init();
	GPIO_WriteBit(EN_PORT, EN_PIN , Bit_SET);
	/* LED初始化 */
	led_init();

	/* 电池通信初始化：配置端口、清零电池数据并启动通信链路 */
	Bsp_Com_init();
	memset(&BatData, 0, sizeof(BATDATA));
	Bat_Com_Restart();
	
	/* 中断初始化（ADC、TIM1、TIM14、TIM6、TIM13） */
	Interrupt_Init();
	user_list.led_reuse_adc_flag=0;
	user_list.led_scan_start_flag=1;
	systick_delay_us(5000);//等待ADC稳定
	/* 先滤波上电扳机；首次方向确认允许当前扣动启动，后续换向仍需松手。 */
	for(i=0;i<30;i++)
	{
		systick_delay_us(1000);
		motor_on_off_control();
	}
	/* 每次方向去抖之前等待新的复用输入采样。 */
	for(i=0;i<20;i++)
	{
		systick_delay_us(1000);
		user_direction_handle();
	}

	/* 上电时快速检测电压状态 */
	Volt_Handler_fast();
	Volt_Handler();
	Volt_Handler();//上电时检测电压
	/* 上电温度快速检测 */
	for(i=0;i<5;i++)
	{
		User_Temperature_Handler_Fast();
	}
	/* 更新故障锁存 */
	error_hold_updata();//更新故障
	#if LIMIT_SPEED_EN
		/* 速度PI初始化 */
		Speed_PI_init();
		Speed_PI_reset();
	#endif
//		mc_lose_step_init();
//		io_debug_init();

    while(1)
    {
		/*IWDG_ReloadCounter - 看门狗喂狗 */
		IWDG_RELOAD_COUNT();
		
		/* 电池通信前台：置于 1ms 块之前高频调用，减小应答调度延迟；
		   实际计时由 ISR 毫秒时钟决定，不依赖调用频率 */
#if BAT_COM_EN
		Bat_Com();
#endif
		/* 1ms时基标志，由TIM1更新中断置位 */
		if(user_list.u8_1ms_flag)
		{
			user_list.u8_1ms_flag=0;

			/* 方向检测 */
			user_direction_handle();
			/* 母线电压保护（欠压/过压） */
			Volt_Handler();
//			debug_motor_on_off();
			/* 电机开关控制（扳机信号检测） */
			motor_on_off_control();
			/* 平均电流过流保护 */
			user_oc_handle();
			/* MOS温度保护 */
			User_Temperature_Handler();
			/* 目标PWM占空比计算（扳机电压映射） */
			pwm_duty_aim_deal();

			#if(Wrench_EN)
				/* 电动扳手按键处理（挡位切换、自动停机） */
				user_WrenchKey_Handler();
			#else
				/* 齿轮/挡位处理 */
				user_gears_handle();
			#endif
			#if LIMIT_SPEED_EN
			/* 限速PI控制：检测转速并限制占空比 */
			if(motor_control_list.bldc_state==MOTOR_RUN&&mc_core_list.bldc_bemf_detected==1)
			{
				get_speed();
				speed_pi_fer_count++;
				if(speed_pi_fer_count>10)
				{
					speed_pi_fer_count=0;
					Speed_Pid.Limit_max_data=motor_control_list.pwm_duty_virtual;

					Speed_Pid.Limit_min_data= (PWM_PRIOD_LOAD*0.15f) ;

					Speed_Pid.refValue=27000;

					motor_control_list.pwm_duty_aim2=MC_PI_Handler(&Speed_Pid,motor_control_list.motor_speed);

				}
			}
			else
			{
				Speed_PI_reset();
				motor_control_list.time_count_60_degree_filter=0;
				motor_control_list.motor_speed=0;
			}
			#endif
			/* 工具状态机控制（停止/运行/报错/休眠） */
			user_state_control();
			/* 故障锁存更新 */
			error_hold_updata();
			/* LED显示处理（电量/挡位/故障） */
			UO_Led_Handler();
			/* 根据状态机设置电机使能标志 */
			/* 需同时满足工具许可、电池通信许可、无实时及锁存故障 */
			if(user_list.flag.bits.gToolEn && Bat_Com_CanRun() &&
			   user_list.userErr.word == 0 && mc_core_list.MC_error.word == 0 &&
			   machine_error_hold.userErr_hold.word == 0 && machine_error_hold.MC_error_hold.word == 0)
			{
				mc_core_list.motor_en=1;
			}
			else
			{
				mc_core_list.motor_en=0;
			}
			/* 初始位置检测（IPD）触发
			   同样受通信许可、电机使能、POSITION 状态及故障门控，
			   取消启动时清除残留 IPD 请求，避免旁路许可执行检测 */
			if(mc_core_list.ipd_star_flag==1)
			{
				if(Bat_Com_CanRun() && mc_core_list.motor_en &&
				   motor_control_list.bldc_state == MOTOR_POSITION &&
				   user_list.userErr.word == 0 && mc_core_list.MC_error.word == 0 &&
				   machine_error_hold.userErr_hold.word == 0 && machine_error_hold.MC_error_hold.word == 0)
				{
					ipd_detect();
				}
				mc_core_list.ipd_star_flag=0;
			}
		}
    }
}

/**
  * @}
*/

/**
  * @}
*/
