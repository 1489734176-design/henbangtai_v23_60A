/**
 * @file     mm32_it.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides the ITR functions and test samples.
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
#define _MM32_IT_C_

/** Files includes */
#include "drv_inc.h"
#include "systick.h"
#include "parameter.h"
#include "user_control.h"

#include "mc_core.h"
#include "board.h"
#include "drv_adc.h"
#include "motor_control.h"
#include "led.h"
#include "Bat_com.h"

#include "motor_config.h"
#include "drv_comp.h"
extern void GetElePeriod(void);
extern void Onfly_BEMF_detect(void);
extern void CheckBMF(uint8_t RunHall);
extern void BLDC_BEMF_Control(void);
extern void MotorForceCommutation(void);

/**
 * @addtogroup MM32_User_Layer
 * @{
 */

/**
 * @addtogroup User_Main
 * @{
 */

void NVIC_Configure(uint8_t ch, uint8_t pri)
{
    NVIC_InitTypeDef  NVIC_InitStruct;

    /** Initialization ADC interrupt */
    NVIC_InitStruct.NVIC_IRQChannel = ch;
    NVIC_InitStruct.NVIC_IRQChannelPriority = pri;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void Interrupt_Init(void)
{
//		/** Initialization Systick interrupt */
//    NVIC_SetPriority(SysTick_IRQn, 2);
	
    /** Initialization ADC interrupt */
    NVIC_Configure(ADC_IRQn, 1);
    /** ADC EOF interrupt enabled */
    ADC_ITConfig(ADC1, ADC_IT_EOS, ENABLE);
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOS);
	
    /** Initialization TIM interrupt */
    NVIC_Configure(TIM1_BRK_UP_TRG_COM_IRQn, 1);
    /** TIM Break interrupt enabled */
    TIM_ClearFlag(TIM1, TIM_FLAG_Break);
		TIM_ITConfig(TIM1, TIM_IT_Break, ENABLE);
	
    /** TIM Update interrupt enabled */
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	
		/** Initialization COMP interrupt */
		NVIC_Configure(TIM14_IRQn, 1);
		/** TIM14 Update interrupt enabled */
		TIM_ClearFlag(TIM14, TIM_FLAG_Update);
		TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
		
			NVIC_Configure(TIM6_IRQn, 2);
		/** TIM6 Update interrupt enabled */
		TIM_ClearFlag(TIM6, TIM_FLAG_Update);
		TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
		
		
		
		
		#if EN_MOTOR_LOCK_DETECT
			/** Initialization COMP interrupt */
		#endif
}

/**
  * @brief Get ADC_x channel_x conversion value
  * @retval None
  */
//void Get_ADC_Result()
//{		
//	Motor_1st.ADC.u16BemfU 			= (uint16_t)GET_ADC_VALUE(BEMF_U_CHANNEL);
//	Motor_1st.ADC.u16BemfV 			= (uint16_t)GET_ADC_VALUE(BEMF_V_CHANNEL);
//	Motor_1st.ADC.u16BemfW 			= (uint16_t)GET_ADC_VALUE(BEMF_W_CHANNEL);
//	Motor_1st.ADC.u16I_OP_O 		= (uint16_t)GET_ADC_VALUE(OP_O_CHANNEL);
//	Motor_1st.ADC.u16VR 				= (uint16_t)GET_ADC_VALUE(VR_CHANNEL);
//	Motor_1st.ADC.u16Vbus	 			= (uint16_t)GET_ADC_VALUE(VBUS_CHANNEL);
//	Motor_1st.ADC.u16I_SUM_Avg 	= (uint16_t)GET_ADC_VALUE(ISUMAVG_CHANNEL);
//}
void Get_ADC_Result()
{		
	mc_core_list.Board_ADC_DATA.u16BemfU 			= (uint16_t)GET_ADC_VALUE(BEMF_U_CHANNEL);
	mc_core_list.Board_ADC_DATA.u16BemfV 			= (uint16_t)GET_ADC_VALUE(BEMF_V_CHANNEL);
	mc_core_list.Board_ADC_DATA.u16BemfW 			= (uint16_t)GET_ADC_VALUE(BEMF_W_CHANNEL);
	mc_core_list.Board_ADC_DATA.u16I_Peak 			= (uint16_t)GET_ADC_VALUE(OP_O_CHANNEL);
	mc_core_list.Board_ADC_DATA.u16VR 				= (uint16_t)GET_ADC_VALUE(VR_CHANNEL);
	
	mc_core_list.Board_ADC_DATA.u16I_SUM_Avg 		= (uint16_t)GET_ADC_VALUE(ISUMAVG_CHANNEL);
	mc_core_list.Board_ADC_DATA.u16Vbus	 			= (uint16_t)GET_ADC_VALUE(VBUS_CHANNEL);

	mc_core_list.Board_ADC_DATA.u16NTC_MOS 			= (uint16_t)GET_ADC_VALUE(T_MOS_CHANNEL);
	
	/*if(user_list.led_reuse_adc_flag==1&&mc_core_list.ipd_star_flag==0)  // LED复用ADC时采集温度
	{
			mc_core_list.Board_ADC_DATA.u16NTC_MOS 	= (uint16_t)GET_ADC_VALUE(T_MOS_CHANNEL);	

	}*/
}

//void LackPhase_Detect()
//{	
//	static uint32_t Frist_I_Rms_value;
//	static uint32_t Second_I_Rms_value;
//	
//	if(Motor_1st.SNLS.u8Onflying_Delay_LackPhase_flag == 1)
//	{
//			Motor_1st.SNLS.u16Onflying_Delay_LackPhase_cnt++;
//			if(Motor_1st.SNLS.u16Onflying_Delay_LackPhase_cnt > DELAY_200ms_PHASELOSS_DETECT)
//			{
//				Motor_1st.SNLS.u16Onflying_Delay_LackPhase_cnt = 0;
//				Motor_1st.SNLS.u8Onflying_Delay_LackPhase_flag = 0;
//			}
//	}
//	
//	if(Motor_1st.SNLS.u8ADC_ControlStep != 0)
//	{
//		if(Motor_1st.SNLS.u16SNLSCloseloopcnt > 10)
//		{
//			switch(Motor_1st.SNLS.u8HallValue_ADC)
//			{
//				case STEP_5_UV:
//					Motor_1st.ADC.u32I_Rms_STEP_5_UV += Motor_1st.ADC.u16I_SUM_Avg;
//				
//					Motor_1st.ADC.u32I_Rms_STEP_2_VU = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_3_WU;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_1_WV;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_6_VW;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_4_UW;
//					}
//				break;
//			
//				case STEP_4_UW:
//					Motor_1st.ADC.u32I_Rms_STEP_4_UW += Motor_1st.ADC.u16I_SUM_Avg;
//				
//					Motor_1st.ADC.u32I_Rms_STEP_3_WU = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_1_WV;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_5_UV;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_2_VU;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_6_VW;
//					}
//				break;
//					
//				case STEP_6_VW:
//					Motor_1st.ADC.u32I_Rms_STEP_6_VW += Motor_1st.ADC.u16I_SUM_Avg;
//					
//					Motor_1st.ADC.u32I_Rms_STEP_1_WV = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_5_UV;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_4_UW;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_3_WU;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_2_VU;
//					}
//				break;
//							
//				case STEP_2_VU:
//					Motor_1st.ADC.u32I_Rms_STEP_2_VU += Motor_1st.ADC.u16I_SUM_Avg;
//				
//					Motor_1st.ADC.u32I_Rms_STEP_5_UV = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_4_UW;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_6_VW;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_1_WV;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_3_WU;
//					}
//				break;
//				
//				case STEP_3_WU:
//					Motor_1st.ADC.u32I_Rms_STEP_3_WU += Motor_1st.ADC.u16I_SUM_Avg;
//				
//					Motor_1st.ADC.u32I_Rms_STEP_4_UW = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_6_VW;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_2_VU;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_5_UV;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_1_WV;
//					}
//				break;
//							
//				case STEP_1_WV:
//					Motor_1st.ADC.u32I_Rms_STEP_1_WV += Motor_1st.ADC.u16I_SUM_Avg;
//				
//					Motor_1st.ADC.u32I_Rms_STEP_6_VW = 0;
//				
//					if(Motor_1st.BLDC.u8Motor_Direction == 0)
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_2_VU;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_3_WU;
//					}
//					else
//					{
//						Frist_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_4_UW;
//						Second_I_Rms_value = Motor_1st.ADC.u32I_Rms_STEP_5_UV;
//					}
//				break;
//			}
//		}
//		
//		if(Motor_1st.SNLS.u16SNLSCloseloopcnt > 30)
//		{
//			if(Motor_1st.SNLS.u8Onflying_Delay_LackPhase_flag == 0)		// ˳��������, �����󴥷�
//			{
//				if(Frist_I_Rms_value > (Second_I_Rms_value * 3))
//				{
//					DiagnoseFAULT.bit.LackPhaseFlag = 1;
//				}
//				else if((Frist_I_Rms_value * 3) < Second_I_Rms_value)
//				{
//					DiagnoseFAULT.bit.LackPhaseFlag = 1;			
//				}
////				else if(Motor_1st.ADC.u16I_SUM_Avg < (Motor_1st.Calib.u16I_Rms_Offset_Calib + 16))
////				{
////					DiagnoseFAULT.bit.LackPhaseFlag = 1;	
////				}
//			}
//		}
//	}
//}


/**
  * @brief
  * @retval 
  */
//void CurrentOffsetCalibration()
//{
//		if(Motor_1st.Calib.u16Calibration_cnt < CALIB_SAMPLES)
//		{
//			Motor_1st.Calib.u16Calibration_cnt++;
//			Motor_1st.Calib.u16I_Rms_Offset_Calib = 0;
//			Motor_1st.Calib.u16Vbus_Offset_Calib = 0;
////			Motor_1st.Calib.u16BemfU_Offset_Calib = 0;
////			Motor_1st.Calib.u16BemfV_Offset_Calib = 0;
////			Motor_1st.Calib.u16BemfW_Offset_Calib = 0;
////			Motor_1st.Calib.u16Bemf_COM_Offset_Calib = 0;
//			
//			// 
//			Motor_1st.Calib.u32Ibus_Offset_sum = 0;
//			Motor_1st.Calib.u32Vbus_Offset_sum = 0;	
////			Motor_1st.Calib.u32BemfU_Offset_sum = 0;	
////			Motor_1st.Calib.u32BemfV_Offset_sum = 0;	
////			Motor_1st.Calib.u32BemfW_Offset_sum = 0;
//		}
//		else if((Motor_1st.Calib.u16Calibration_cnt < CALIB_SAMPLES_DOUBLE) \
//			&&(Motor_1st.Calib.u16Calibration_cnt >= CALIB_SAMPLES)) 
//		{
//			Motor_1st.Calib.u16Calibration_cnt++;
//			
//			Motor_1st.Calib.u32Ibus_Offset_sum += Motor_1st.ADC.u16I_SUM_Avg;
//			Motor_1st.Calib.u32Vbus_Offset_sum += Motor_1st.ADC.u16Vbus;
//			
////			Motor_1st.Calib.u32BemfU_Offset_sum += Motor_1st.ADC.u16BemfU;	
////			Motor_1st.Calib.u32BemfV_Offset_sum += Motor_1st.ADC.u16BemfV;	
////			Motor_1st.Calib.u32BemfW_Offset_sum += Motor_1st.ADC.u16BemfW;	
//		}
//		else if(Motor_1st.Calib.u16Calibration_cnt == CALIB_SAMPLES_DOUBLE)
//		{
//			Motor_1st.Calib.u16Calibration_cnt++;
//			
//			Motor_1st.Calib.u16Calibration_cnt = CALIB_SAMPLES_DOUBLE;
//			
//			Motor_1st.Calib.u16I_Rms_Offset_Calib = Motor_1st.Calib.u32Ibus_Offset_sum >> CALIB_SAMPLES_K;
//			Motor_1st.Calib.u16Vbus_Offset_Calib = Motor_1st.Calib.u32Vbus_Offset_sum >> CALIB_SAMPLES_K;
//			
////			Motor_1st.Calib.u16BemfU_Offset_Calib = Motor_1st.Calib.u32BemfU_Offset_sum >> CALIB_SAMPLES_K;
////			Motor_1st.Calib.u16BemfV_Offset_Calib = Motor_1st.Calib.u32BemfV_Offset_sum >> CALIB_SAMPLES_K;
////			Motor_1st.Calib.u16BemfW_Offset_Calib = Motor_1st.Calib.u32BemfW_Offset_sum >> CALIB_SAMPLES_K;
////			Motor_1st.Calib.u16Bemf_COM_Offset_Calib = (Motor_1st.Calib.u16BemfU_Offset_Calib + Motor_1st.Calib.u16BemfV_Offset_Calib + Motor_1st.Calib.u16BemfW_Offset_Calib)/3;

//			#if EN_OFFSET_CALIB
//				if((Motor_1st.Calib.u16I_Rms_Offset_Calib < CURRENT_OFFSET_H_THD) \
//					&& (Motor_1st.Calib.u16I_Rms_Offset_Calib > CURRENT_OFFSET_L_THD))
//				{
//					Motor_1st.Calib.u8Calibration_OK_flag = 1;
//				}
//				else
//				{
//					DiagnoseFAULT.bit.OffsetFlag = 1;
//					Motor_1st.Calib.u8Calibration_OK_flag = 1;
//				}
//			#else
//				Motor_1st.Calib.u8Calibration_OK_flag = 1;
//			#endif
//		}
//}


void SysTick_Handler(void)
{
    Clear_Over_Flag();
}

void WWDG_IRQHandler(void) {}

void PVD_IRQHandler(void) {}

void PWM_IRQHandler(void) {}

void FLASH_IRQHandler(void) {}

void RCC_IRQHandler(void) {}

void EXTI0_1_IRQHandler(void) {}

void EXTI2_3_IRQHandler(void) {}

void EXTI4_15_IRQHandler(void) {}

void HWDIV_IRQHandler(void) {}

void DMA1_Channel1_IRQHandler(void) {}

void DMA1_Channel2_3_IRQHandler(void) {}

void DMA1_Channel4_5_IRQHandler(void) {}

void ADC_IRQHandler(void) 
{
	u16 IBusPeak_Adc_temp1;
		static u16 Ibus_Peak_Ocp_IT_count=0;
    if( ADC_GetITStatus(ADC1, ADC_IT_EOS))
    {						  
			ADC_ClearFlag(ADC1,ADC_FLAG_EOS);			
		

			
			
			/* todo*/
			// 读取ADC值
//			LED1_ON();
			Get_ADC_Result();
			#if IBus_Peak_OCP_ONCE_EN
			if(motor_control_list.u8Calibration_OK_flag)
			{
						if(mc_core_list.Board_ADC_DATA.u16I_Peak>mc_core_list.CurrentOffset_ad_data)
					{
						IBusPeak_Adc_temp1=mc_core_list.Board_ADC_DATA.u16I_Peak-mc_core_list.CurrentOffset_ad_data;
						IBusPeak_Adc_temp1=((((u32)IBusPeak_Adc_temp1)*user_list.Vcc_cal_gain)>>15);
						
					}
					else 
					{
						IBusPeak_Adc_temp1=0;
					}
				if(IBusPeak_Adc_temp1>mc_core_list.Ibus_Peak_Ocp_Once_AdcValue)  // 瞬时过流，立即保护
				{
							TIM_CtrlPWMOutputs(TIM1, DISABLE);
							machine_error_hold.userErr_hold.bits.gIBusAcmpOCP=1;
							Stop_Motor();//?
					
				}
				
				if(IBusPeak_Adc_temp1>IBus_Peak_IT_OCP_AdcValue)  // 超过滤波过流阈值
				{
							if(Ibus_Peak_Ocp_IT_count++>IBus_Peak_IT_COUNT)
							{
								Ibus_Peak_Ocp_IT_count=IBus_Peak_IT_COUNT;
								TIM_CtrlPWMOutputs(TIM1, DISABLE);
								machine_error_hold.userErr_hold.bits.gIBusAcmpOCP=1;
								Stop_Motor();//?
							}
					
				}
				else 
				{
						if(Ibus_Peak_Ocp_IT_count)
						{
							Ibus_Peak_Ocp_IT_count--;
						}
							
				}
				
			}
			#endif
			
			mc_core_adc_isr_handle();
    }
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	static unsigned char u16PWMTimeCnt = 0;
	max_sector_handle();
	if(READ_TIM1_UPDATE_FLAG())
	{
//		GPIO_Toggle(LED_PIN3_PORT, GPIO_Pin_10);
		if(GET_COMP2_OUT())  // 读取比较器输出（电流检测）
		{
			mc_core_list.current_comp_out=1;
//			GPIO_SetBits(LED_PIN3_PORT, GPIO_Pin_10);
		}
		else 
		{
			mc_core_list.current_comp_out=0;
//			GPIO_ResetBits(LED_PIN3_PORT, GPIO_Pin_10);
		}
		// 1ms时间片计数

		MC_Machine_State();
		if(++u16PWMTimeCnt >= SLOWLOOP_1ms_CNT_LOAD)  // 1ms时基
		{
			/* 电池通信毫秒时钟：仅递增计数，不在中断内解析或发送 */
#if BAT_COM_EN
			Bat_Com_Tick1ms();
#endif
			user_list.u8_1ms_flag=1;
			u16PWMTimeCnt = 0;

		}

		/* LED 复用扫描（LED_Scan）已随单灯硬件取消，见 led.h */
		#if (Block_Protect_En)
			motor_block_detect();
		#endif
		/** Clear UPDATE Flag */
		CLEAN_TIM1_UPDATE_FLAG();
	}

	if(READ_TIM1_BREAK_FLAG())
	{
		/** Clear TIM1_BREAK Flag */
		CLEAN_TIM1_BREAK_FLAG();
		//			Stop_Motor();
		//		  TIM_CtrlPWMOutputs(TIM1, ENABLE);
		machine_error_hold.userErr_hold.bits.gIBusAcmpOCP=1;
		/* todo*/
	}
}


void TIM1_CC_IRQHandler(void) {}

void TIM2_IRQHandler(void) {}

	
void	TIM6_IRQHandler(void)
{
		if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET) 
	{
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);
	}
}	
	
void TIM13_IRQHandler(void)
{
	/* 原位操作电池通信（GetData/SendData）已由 DAYE A1.1 USART 半双工方案取代，
	   本定时器不再承担通信时序，保留中断入口以免遗留未清标志。 */
	if(TIM_GetFlagStatus(TIM13, TIM_FLAG_Update) != RESET)
	{
		TIM_ClearFlag(TIM13, TIM_FLAG_Update);
	}
}

#if BAT_COM_EN
/**
  * @brief USART1 中断：电池包单线半双工通信收发（DAYE A1.1）
  */
void USART1_IRQHandler(void)
{
	Bat_Com_UsartIrq();
}
#endif

void TIM14_IRQHandler(void) 
{
	if(TIM_GetFlagStatus(TIM14, TIM_FLAG_Update) != RESET) 
	{   
		/** Clear UPDATE Flag */
		mc_core_timer14_isr_handle();
		TIM_ClearFlag(TIM14, TIM_FLAG_Update);
		
	}
}

void TIM16_IRQHandler(void) 
{





}
void COMP1_2_IRQHandler(void)
{

    if (SET == EXTI_GetITStatus(EXTI_Line20))
    {
	    EXTI_ClearITPendingBit(EXTI_Line20);
			#if 1
				mc_core_comp_isr_handle();
			#endif

    }
		  if (SET == EXTI_GetITStatus(EXTI_Line19))
    {


        EXTI_ClearITPendingBit(EXTI_Line19);
    }

}

void TIM17_IRQHandler(void) {}

void I2C1_IRQHandler(void) {}

void SPI1_IRQHandler(void) {}

void SPI2_IRQHandler(void) {}

void UART1_IRQHandler(void) {}

void UART2_IRQHandler(void) {}

void UART3_IRQHandler(void) {}
    
void FLEX_CAN_IRQHandler(void) {}
/**
  * @}
*/

/**
  * @}
*/
