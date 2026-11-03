/**
 * @file     user_function.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the functions for the board level support package.
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
#define _USER_FUNCTION_C_

/** Files includes */
#include "main.h"
#include "board.h"
#include "hal_gpio.h"
#include "parameter.h"
#include "user_function.h"
#include "my_mc.h"
User_list_Struct user_list;

Machine_ERROR_HOLD machine_error_hold;


void user_oc_handle(void)
{
		static u16 IBusAvgOCP1_count=0;
		if(bldc_list.Board_ADC_DATA.u16I_SUM_Avg>bldc_list.CurrentOffset_ad_data)
		{
			user_list.IBusAvgOCP1_Adc=bldc_list.Board_ADC_DATA.u16I_SUM_Avg-bldc_list.CurrentOffset_ad_data;  // 减去偏置得到实际电流
		}
		else 
		{
			user_list.IBusAvgOCP1_Adc=0;
		
		}
		if(user_list.IBusAvgOCP1_Adc>IBus_Avg_OCP1_AdcValue)  // 超过OCP1阈值
		{
				
			IBusAvgOCP1_count++;
			if(IBusAvgOCP1_count>1000)
			{
				user_list.userErr.bits.gIBusAvgOCP1=1;
			}
		}
		else 
		{
			IBusAvgOCP1_count=0;
		
		
		}
	
	
	

}

void user_direction_handle(void)
{
	
	static u8 dir_cw_count=0;
	static u8 dir_ccw_count=0;
	if(GPIO_ReadInputDataBit(DIR_PORT,DIR_PIN)) 
	{
			dir_ccw_count=0;
			if(++dir_cw_count>10)
			{
			
				bldc_list.mc_dir=0;
				dir_cw_count=10;
			}
			
			

	}
	else 
	{
			dir_cw_count=0;
			if(++dir_ccw_count>10)
			{
			
				bldc_list.mc_dir=1;
				dir_ccw_count=10;
			}
	
	}

}

void motor_on_off_control(void)
{
	
 static 	u8 gToolTrigger_off_count=0;
	static 	u8 gToolTrigger_on_count=0;
		user_list.flag.bits.gToolTrigger=1;
//	if(bldc_list.Board_ADC_DATA.u16VR<100)
//	{
//		gToolTrigger_off_count++;
//		gToolTrigger_on_count=0;
//		if(gToolTrigger_off_count>10)
//		{
//			user_list.flag.bits.gToolTrigger=0;
//		}
//	
//	}
//	if(bldc_list.Board_ADC_DATA.u16VR>200)
//	{
//				gToolTrigger_off_count=0;
//	   	gToolTrigger_on_count++;
//			if(gToolTrigger_on_count>10)
//			{
//			user_list.flag.bits.gToolTrigger=1;
//			}
//	}

}

void error_hold_updata(void)
{
	machine_error_hold.MC_error_hold.word|=bldc_list.MC_error.word;
	machine_error_hold.userErr_hold.word|=user_list.userErr.word;

}

void user_state_control(void)
{
				switch(user_list.user_state)
				{
					case TOOL_STOP:
						user_list.flag.bits.gToolEn=0;
						if(user_list.flag.bits.gToolTrigger)
						{
						
								user_list.flag.bits.gToolEn=1;
								user_list.user_state=TOOL_RUN;  // 切换到运行状态
						}
					break;
						case TOOL_ERROR_STOP:
							user_list.flag.bits.gToolEn=0;
						if(user_list.flag.bits.gToolTrigger==0)
						{
								if(user_list.userErr.word==0)
								{
									bldc_list.MC_error.word=0;
									machine_error_hold.MC_error_hold.word=0;
									machine_error_hold.userErr_hold.word&=0x00;
								
								}
//							user_list.userErr.word=0;
							user_list.user_state=TOOL_STOP;  // 切换到停止状态
							
						}
							break;
					case TOOL_RUN:
						if(machine_error_hold.MC_error_hold.word!=0||machine_error_hold.userErr_hold.word!=0)
						{
							user_list.user_state=TOOL_ERROR_STOP;  // 切换到故障停止状态
						
						}
						if(user_list.flag.bits.gToolTrigger==0)
						{
						
							user_list.user_state=TOOL_STOP;
						}
					
						break;
					default:
						break;
				
				
				
				}


}

