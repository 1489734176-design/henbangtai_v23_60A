#include "user_control.h"
#include "parameter.h"
#include "board.h"
#include "motor_control.h"
#include "drv_adc.h"
#include "flash_data_save.h"
#include "Bat_com.h"
#include "led.h"


#include "systick.h"
#include "motor_config.h"
#include "drv_pwm.h"
User_list_Struct user_list;

Machine_ERROR_HOLD machine_error_hold;

	u32 VERF_adc_data_avg=0;
	uint16_t CalibrationData = 0;
/**
 * @brief  5V基准校准
 *        读取芯片内部VREFINT校准值，通过ADC测量VREFINT计算VCC校准增益
 *        用于将ADC原始值转换为实际电压值，确保电流/电压检测精度
 */
void Vcc_5V_CAL(void)
{
		u32 CalibrationData_temp;
	  ADC_InitTypeDef ADC_InitStruct;
	  /* 读取Flash中VREFINT校准值（0x1FFFF7E0地址） */
	  CalibrationData = *(uint16_t *)(0x1FFFF7E0);
	
		CalibrationData_temp=(((u32)CalibrationData)*21626)>>15;
		if(CalibrationData_temp<904||CalibrationData_temp>1061)
		{
			user_list.Vcc_cal_gain=32768;
		}
		else 
		{
				Stop_Motor();


				RCC_APB1PeriphClockCmd(RCC_APB1Periph_ADC1, ENABLE);
				ADC_ExternalTrigConvCmd(ADC1, DISABLE);			// �ر�ADC�ⲿ����ת��
				systick_delay_us(200);
				ADC_Cmd(ADC1, DISABLE);
				systick_delay_us(200);
				ADC_DeInit(ADC1);
				systick_delay_us(200);
				ADC_Cmd(ADC1, DISABLE);
				systick_delay_us(200);
						ADC_AnyChannelCmd(ADC1, DISABLE);
				ADC_StructInit(&ADC_InitStruct);
				ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
				ADC_InitStruct.ADC_Prescaler  = ADC_Prescaler_16;
				ADC_InitStruct.ADC_Mode       = ADC_Mode_Imm;
				ADC_InitStruct.ADC_DataAlign  = ADC_DataAlign_Right;
				ADC_Init(ADC1, &ADC_InitStruct);

				ADC_SampleTimeConfig(ADC1, ADC_Channel_VoltReference, ADC_SampleTime_240_5);

				ADC_ChannelCmd(ADC1, ADC_Channel_VoltReference, ENABLE);

				ADC_TempSensorCmd(ENABLE);

				ADC_Cmd(ADC1, ENABLE);
				systick_delay_us(200);
						ADC_SoftwareStartConvCmd(ADC1, ENABLE);
						while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
						{
						}

						ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
								systick_delay_us(200);
						ADC_SoftwareStartConvCmd(ADC1, ENABLE);
						while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
						{
						}

						ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

						VERF_adc_data_avg = ADC_GetConversionValue(ADC1);
						/* 计算VCC校准增益 = 预期值/测量值 * 32768 */
							user_list.Vcc_cal_gain=(u32)(32768.0f*((float)CalibrationData_temp)/((float)VERF_adc_data_avg));
						
				ADC_Cmd(ADC1, DISABLE);
				systick_delay_us(200);
				ADC_DeInit(ADC1);
				systick_delay_us(200);
				Board_ADC_Init();
		}
	
}

/**
 * @brief  用户参数初始化
 *        清零错误状态，初始化状态机为停止状态，设置初始方向
 */
void user_para_init(void)
{
	user_list.userErr.word=0;
	user_list.user_state=TOOL_STOP;
		user_list.flag.bits.gToolDir= Tool_Dir_Init;
	/* 扳机为上电来源：首次稳定方向确认后允许本次扣动启动。 */
	user_list.direction_initialized=0;
	user_list.dir_valid=0;
	user_list.dir_retrig_need=1;

}
#if 0
/**
 * @brief  平均电流过流保护（OCP1）
 *        当IBusAvg电流超过阈值IBus_Avg_OCP1_AdcValue持续1000ms时触发保护
 */
void user_oc_handle(void)
{
		static u16 IBusAvgOCP1_count=0;
	static u16 IBusAvgOCP2_count=0;
	static u16 IBusAvgOCP3_count=0;
	static u16 IBusPeakOCP1_count=0;
		static u16 IBusAvg_rec_count=0;
		#if (!Multistage_Integral_Peak_OCP_EN)
	static u16 IBusPeakOCP2_count=0;
	static u16 IBusPeakOCP3_count=0;
	static u16 IBusPeakREC_count=0;
	#endif
	u16  ibus_temp=0;
	   
		
		if(mc_core_list.Board_ADC_DATA.u16I_SUM_Avg>mc_core_list.CurrentOffset_ad_data)
		{
			ibus_temp=mc_core_list.Board_ADC_DATA.u16I_SUM_Avg-mc_core_list.CurrentOffset_ad_data;
		}
		else 
		{
			ibus_temp=0;
		
		}
			if(mc_core_list.Board_ADC_DATA.u16I_Peak>mc_core_list.CurrentOffset_ad_data)
		{
			user_list.IBusPeak_Adc=mc_core_list.Board_ADC_DATA.u16I_Peak-mc_core_list.CurrentOffset_ad_data;
		}
		else 
		{
			user_list.IBusPeak_Adc=0;
		
		}	
		
		
//		user_list.IBusAvg1_Adc=(u16)((float)mc_core_list.pwm_actual_runing_data *ibus_temp/PWM_PRIOD_LOAD);	
//		user_list.IBusAvg_Adc=(user_list.IBusAvg1_Adc>>3)+user_list.IBusAvg_Adc-(user_list.IBusAvg_Adc>>3);
		user_list.IBusAvg_Adc=ibus_temp;
		//------------------ocp1
		if(user_list.IBusAvg_Adc>IBus_Avg_OCP1_AdcValue)
		{
				
			IBusAvgOCP1_count++;
			if(IBusAvgOCP1_count>IBus_Avg_OCP1_MS_CONUT)
			{
				IBusAvgOCP1_count=IBus_Avg_OCP1_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP1=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP1_count>0)
			{
				IBusAvgOCP1_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP1=0;
			
			}
		}
			////------------------ocp2
			if(user_list.IBusAvg_Adc>IBus_Avg_OCP2_AdcValue)
		{
				
			IBusAvgOCP2_count++;
			if(IBusAvgOCP2_count>IBus_Avg_OCP2_MS_CONUT)
			{
				IBusAvgOCP2_count=IBus_Avg_OCP2_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP2=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP2_count>0)
			{
				IBusAvgOCP2_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP2=0;
			
			}
			
		}
		
			//------------------ocp3
			if(user_list.IBusAvg_Adc>IBus_Avg_OCP3_AdcValue)
		{
				
			IBusAvgOCP3_count++;
			if(IBusAvgOCP3_count>IBus_Avg_OCP3_MS_CONUT)
			{
				IBusAvgOCP3_count=IBus_Avg_OCP3_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP3=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP3_count>0)
			{
				IBusAvgOCP3_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP3=0;
			
			}
			
		}	
		
		if(user_list.IBusAvg_Adc<IBus_Avg_OCP1_AdcValue&&motor_control_list.bldc_state==MOTOR_STOP)
		{
			IBusAvg_rec_count++;
			if(IBusAvg_rec_count>IBus_Avg_REC_MS_CONUT)
			{
				IBusAvg_rec_count=0;
				user_list.userErr.bits.gIBusAvgOCP3=0;
				user_list.userErr.bits.gIBusAvgOCP2=0;
				user_list.userErr.bits.gIBusAvgOCP1=0;
				IBusAvgOCP3_count=0;
				IBusAvgOCP2_count=0;
				IBusAvgOCP1_count=0;
			}
			
		}


#if Multistage_Integral_Peak_OCP_EN
		
			  if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP3_AdcValue)
        {

						IBusPeakOCP1_count+=IBus_Smart_OCP3_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
		    else  if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP2_AdcValue)
        {

						IBusPeakOCP1_count+=IBus_Smart_OCP2_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
				else if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP1_AdcValue)
				{
				
						IBusPeakOCP1_count+=IBus_Smart_OCP1_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
				
				
				}
        else  
					
        {
					
						if(motor_control_list.bldc_state==MOTOR_STOP)
						{
								if(IBusPeakOCP1_count>IBus_Smart_REC_MS_Operation_Value)
							{
									IBusPeakOCP1_count-=IBus_Smart_REC_MS_Operation_Value;
							}
							else
							{
								IBusPeakOCP1_count=0;
									user_list.userErr.bits.gIBusPeakOCP1 = 0;
							}
						}
						else
						{
					
							if(IBusPeakOCP1_count>IBus_Smart_Peak_OCP1_AdcValue)
							{
									IBusPeakOCP1_count-=IBus_Smart_Peak_OCP1_AdcValue;
							}
							else
							{
								IBusPeakOCP1_count=0;
									user_list.userErr.bits.gIBusPeakOCP1 = 0;
							}
						}
        }
		
#else
        if(user_list.IBusPeak_Adc > IBus_Peak_OCP1_AdcValue)
        {
            if(IBusPeakOCP1_count++ >= IBus_Peak_OCP1_MS_CONUT )
            {
               IBusPeakOCP1_count= IBus_Peak_OCP1_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP1_count)
            {
                IBusPeakOCP1_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				
				
				  if(user_list.IBusPeak_Adc > IBus_Peak_OCP2_AdcValue)
        {
            if(IBusPeakOCP2_count++ >= IBus_Peak_OCP2_MS_CONUT )
            {
               IBusPeakOCP2_count= IBus_Peak_OCP2_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP2_count)
            {
                IBusPeakOCP2_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				
					if(user_list.IBusPeak_Adc > IBus_Peak_OCP3_AdcValue)
        {
            if(IBusPeakOCP3_count++ >= IBus_Peak_OCP3_MS_CONUT )
            {
               IBusPeakOCP3_count= IBus_Peak_OCP3_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP3_count)
            {
                IBusPeakOCP3_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				if(user_list.userErr.bits.gIBusPeakOCP1)
				{
					if(user_list.IBusPeak_Adc < IBus_Peak_OCP1_AdcValue)
					{
						IBusPeakREC_count++;
						if(IBusPeakREC_count>IBus_Peak_REC_MS_CONUT)
						{
							user_list.userErr.bits.gIBusPeakOCP1=0;
						}
										
					}
				}
				else 
				{
					IBusPeakREC_count=0;
				
				}
				
#endif		
}

/**
 * @brief  快速电压保护处理（无滤波延时）
 *        检测母线电压：静态欠压、运行欠压1/2、过压
 *        用于上电时快速判断电压状态
 */
void Volt_Handler_fast(void)
{
				#if(1)
        if((mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Stb_UVP_ADC)) // 0 ���Ը�Ϊ���ֹͣ
        {
      
								user_list.userErr.bits.gVBusStbUVP = 1; //��̬Ƿѹ����

        }
        else
        {

			      	user_list.userErr.bits.gVBusStbUVP = 0;
        }

				#endif
        if(mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Run_UVP1_ADC)
        {
  
     
                user_list.userErr.bits.gVBusRunUVP1 = 1;
                

        }
        else
        {

							user_list.userErr.bits.gVBusRunUVP1 = 0;

        }
        
       
        if(mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Run_UVP2_ADC)
        {
    
  
              user_list.userErr.bits.gVBusRunUVP2 = 1;
     
        }
        else
        {

					  user_list.userErr.bits.gVBusRunUVP2 = 0;
        }
        if(mc_core_list.Board_ADC_DATA.u16Vbus > VBus_OVP_ADC)
        {
              user_list.userErr.bits.gVBusOVP = 1;
        } 
        else
        {
							user_list.userErr.bits.gVBusOVP = 0;
        }
}
/**
 * @brief  电压保护处理（带滤波延时）
 *        检测母线电压：静态欠压(仅非运行时)、运行欠压1/2、过压
 *        各阈值有独立滤波时间，防止误触发
 */
void Volt_Handler(void)
{


	static u16 VBusStbUVP_Cnt=0;//�ϵ�ֻ����һ��
	static u16 VBusRunUVP1_Cnt=0;
	static u16 VBusRunUVP2_Cnt=0;
	static u16 	VBusOVP_Cnt=0;
    #if EN_VOLATAGE_PROTECT    
				/* 计算母线电压（单位0.01V）：ADC值 * 分压比 * 100 / 4096 */
				user_list.VBus_Vx100=(mc_core_list.Board_ADC_DATA.u16Vbus*VBUS_BASE_X100)>>12;
        if((mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Stb_UVP_ADC)&&(user_list.user_state != TOOL_RUN))  // 静态欠压（仅非运行时检测） // 0 ���Ը�Ϊ���ֹͣ
        {
            if(VBusStbUVP_Cnt++ >= VBus_Stb_UVP_FiltMS_COUNT)
            {
                VBusStbUVP_Cnt= VBus_Stb_UVP_FiltMS_COUNT;

								user_list.userErr.bits.gVBusStbUVP = 1; //��̬Ƿѹ����
            }
        }
        else
        {
            if(VBusStbUVP_Cnt)
            {
                VBusStbUVP_Cnt--;
            }
            else
            {
			      	user_list.userErr.bits.gVBusStbUVP = 0;
            }
        }


        if(mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Run_UVP1_ADC)
        {
            if(VBusRunUVP1_Cnt++ >= VBus_Run_UVP1_FiltMS_COUNT)
            {
                VBusRunUVP1_Cnt= VBus_Run_UVP1_FiltMS_COUNT;
     
                user_list.userErr.bits.gVBusRunUVP1 = 1;
                
            }
        }
        else
        {
            if(VBusRunUVP1_Cnt)
            {
                VBusRunUVP1_Cnt--;
            }
			else
            {
							user_list.userErr.bits.gVBusRunUVP1 = 0;
            }
        }
        
       
        if(mc_core_list.Board_ADC_DATA.u16Vbus < VBus_Run_UVP2_ADC)
        {
            if(VBusRunUVP2_Cnt++ >=VBus_Run_UVP2_FiltMS_COUNT)
            {
                VBusRunUVP2_Cnt= VBus_Run_UVP2_FiltMS_COUNT;
  
                user_list.userErr.bits.gVBusRunUVP2 = 1;
            }
        }
        else
        {
            if(VBusRunUVP2_Cnt)
            {
                VBusRunUVP2_Cnt--;
            }
						else
            {
					  	user_list.userErr.bits.gVBusRunUVP2 = 0;
            }
        }



        if(mc_core_list.Board_ADC_DATA.u16Vbus > VBus_OVP_ADC)
        {
            if(VBusOVP_Cnt++ >= VBus_OVP_FiltMS_COUNT)
            {
                VBusOVP_Cnt= VBus_OVP_FiltMS_COUNT;

                user_list.userErr.bits.gVBusOVP = 1;
            }
        } 
        else
        {
            if(VBusOVP_Cnt)
            {
                VBusOVP_Cnt--;
            }
						else
            {
							user_list.userErr.bits.gVBusOVP = 0;
            }
        }


		#endif
}



/**
 * @brief  快速MOS温度保护处理（无滤波延时）
 *        直接读取NTC ADC值判断是否超温或恢复正常
 */
void User_Temperature_Handler_Fast(void)
{
	 #if EN_MOS_TEMP_DETECT
				 if(mc_core_list.Board_ADC_DATA.u16NTC_MOS> RSM_MOS_TEMP_OVER_THD_ADC)//        
				{

						user_list.userErr.bits.gNtcMos=0;
				}
				 if(
				((mc_core_list.Board_ADC_DATA.u16NTC_MOS < MOS_TEMP_OVER_THD_ADC))
				 )//��������
				 {
				 
					user_list.userErr.bits.gNtcMos=1;
				 
				 }
		#endif

}

/**
 * @brief  MOS温度保护处理（带滤波延时）
 *        超温时需持续MOS_TEMP_OVER_TIME才触发报警
 *        恢复时需持续RSM_MOS_TEMP_OVER_TIME才清除报警
 */
void User_Temperature_Handler(void)
{

		static u16 NtcMosErrClr_Cnt=0;
		static u16 NtcMosErr_Cnt=0;
    #if EN_MOS_TEMP_DETECT
  
        if(user_list.userErr.bits.gNtcMos == 1)
        {
            if(mc_core_list.Board_ADC_DATA.u16NTC_MOS> RSM_MOS_TEMP_OVER_THD_ADC)//        
            {
                if(NtcMosErrClr_Cnt++ >= RSM_MOS_TEMP_OVER_TIME)
                {
                    NtcMosErrClr_Cnt= RSM_MOS_TEMP_OVER_TIME;
                    user_list.userErr.bits.gNtcMos=0;
                }
            }
            else
            {
                if(NtcMosErrClr_Cnt)
                {
                    NtcMosErrClr_Cnt--;
                }

            }
            NtcMosErr_Cnt= 0;
        }
        else
        {
           NtcMosErrClr_Cnt= 0;
            if(
            (mc_core_list.Board_ADC_DATA.u16NTC_MOS <MOS_TEMP_OVER_THD_ADC )//
						//(strInput.NtcMos_Adc < strInput.NtcMosOTP_Adc)
            //||(strInput.NtcMos_Adc > strInput.NtcMosUTP_Adc)
            )
            {
                if(NtcMosErr_Cnt++ >= MOS_TEMP_OVER_TIME)
                {
                  NtcMosErr_Cnt= MOS_TEMP_OVER_TIME;
                    
									user_list.userErr.bits.gNtcMos=1;
                }
            }
            else
            {
                if(NtcMosErr_Cnt)
                {
                   NtcMosErr_Cnt--;
                }
            }
                    
            
        }
#endif


}

#else // 使用user_function.c中的简化版过流保护
void user_oc_handle(void)
{
		static u16 IBusAvgOCP1_count=0;
	static u16 IBusAvgOCP2_count=0;
	static u16 IBusAvgOCP3_count=0;
		static u16 IBusAvgOCP4_count=0;
	static u16 IBusPeakOCP1_count=0;
		static u16 IBusAvg_rec_count=0;
		#if (!Multistage_Integral_Peak_OCP_EN)
	static u16 IBusPeakOCP2_count=0;
	static u16 IBusPeakOCP3_count=0;
	static u16 IBusPeakREC_count=0;
	#endif
	u16  ibus_temp=0;
	u16  ipeak_temp=0;
	   
		
		if(mc_core_list.Board_ADC_DATA.u16I_SUM_Avg>mc_core_list.CurrentOffset_ad_data)
		{
			ibus_temp=mc_core_list.Board_ADC_DATA.u16I_SUM_Avg-mc_core_list.CurrentOffset_ad_data;
			ibus_temp=((((u32)ibus_temp)*user_list.Vcc_cal_gain)>>15);
		}
		else 
		{
			ibus_temp=0;
		
		}
			if(mc_core_list.Board_ADC_DATA.u16I_Peak>mc_core_list.CurrentOffset_ad_data)
		{
			ipeak_temp=mc_core_list.Board_ADC_DATA.u16I_Peak-mc_core_list.CurrentOffset_ad_data;
			user_list.IBusPeak_Adc=((((u32)ipeak_temp)*user_list.Vcc_cal_gain)>>15);
		}
		else 
		{
			user_list.IBusPeak_Adc=0;
		
		}	
		
		
//		user_list.IBusAvg1_Adc=(u16)((float)mc_core_list.pwm_actual_runing_data *ibus_temp/PWM_PRIOD_LOAD);	
//		user_list.IBusAvg_Adc=(user_list.IBusAvg1_Adc>>3)+user_list.IBusAvg_Adc-(user_list.IBusAvg_Adc>>3);
		user_list.IBusAvg_Adc=ibus_temp;
		//------------------ocp1
		if(user_list.IBusAvg_Adc>IBus_Avg_OCP1_AdcValue)
		{
				
			IBusAvgOCP1_count++;
			if(IBusAvgOCP1_count>IBus_Avg_OCP1_MS_CONUT)
			{
				IBusAvgOCP1_count=IBus_Avg_OCP1_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP1=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP1_count>0)
			{
				IBusAvgOCP1_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP1=0;
			
			}
		}
			////------------------ocp2
			if(user_list.IBusAvg_Adc>IBus_Avg_OCP2_AdcValue)
		{
				
			IBusAvgOCP2_count++;
			if(IBusAvgOCP2_count>IBus_Avg_OCP2_MS_CONUT)
			{
				IBusAvgOCP2_count=IBus_Avg_OCP2_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP2=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP2_count>0)
			{
				IBusAvgOCP2_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP2=0;
			
			}
			
		}
		
			//------------------ocp3
			if(user_list.IBusAvg_Adc>IBus_Avg_OCP3_AdcValue)
		{
				
			IBusAvgOCP3_count++;
			if(IBusAvgOCP3_count>IBus_Avg_OCP3_MS_CONUT)
			{
				IBusAvgOCP3_count=IBus_Avg_OCP3_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP3=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP3_count>0)
			{
				IBusAvgOCP3_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP3=0;
			
			}
			
		}	
				//------------------ocp4
		if(user_list.IBusAvg_Adc>IBus_Avg_OCP4_AdcValue)
		{
				
			IBusAvgOCP4_count++;
			if(IBusAvgOCP4_count>IBus_Avg_OCP4_MS_CONUT)
			{
				IBusAvgOCP4_count=IBus_Avg_OCP4_MS_CONUT;
				user_list.userErr.bits.gIBusAvgOCP4=1;
				IBusAvg_rec_count=0;
			}
		}
		else 
		{
			if(IBusAvgOCP4_count>0)
			{
				IBusAvgOCP4_count--;
			}
			else
			{
				user_list.userErr.bits.gIBusAvgOCP4=0;
			
			}
			
		}
		
		
		if(user_list.IBusAvg_Adc<IBus_Avg_OCP1_AdcValue&&motor_control_list.bldc_state==MOTOR_STOP)
		{
			IBusAvg_rec_count++;
			if(IBusAvg_rec_count>IBus_Avg_REC_MS_CONUT)
			{
				IBusAvg_rec_count=0;
				user_list.userErr.bits.gIBusAvgOCP4=0;
				user_list.userErr.bits.gIBusAvgOCP3=0;
				user_list.userErr.bits.gIBusAvgOCP2=0;
				user_list.userErr.bits.gIBusAvgOCP1=0;
				IBusAvgOCP4_count=0;
				IBusAvgOCP3_count=0;
				IBusAvgOCP2_count=0;
				IBusAvgOCP1_count=0;
			}
			
		}


#if Multistage_Integral_Peak_OCP_EN
		
			  if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP3_AdcValue)
        {

						IBusPeakOCP1_count+=IBus_Smart_OCP3_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
		    else  if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP2_AdcValue)
        {

						IBusPeakOCP1_count+=IBus_Smart_OCP2_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
				else if(user_list.IBusPeak_Adc > IBus_Smart_Peak_OCP1_AdcValue)
				{
				
						IBusPeakOCP1_count+=IBus_Smart_OCP1_MS_Operation_Value;
            if(IBusPeakOCP1_count >= 32768 )
            {
               IBusPeakOCP1_count= 32768;
							//mos���ش���
							if(user_list.userErr.bits.gIBusPeakOCP1==0)
							{
//								user_list.Mos_overload_count++;
							
							}
							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
				
				
				}
        else  
					
        {
					
						if(motor_control_list.bldc_state==MOTOR_STOP)
						{
								if(IBusPeakOCP1_count>IBus_Smart_REC_MS_Operation_Value)
							{
									IBusPeakOCP1_count-=IBus_Smart_REC_MS_Operation_Value;
							}
							else
							{
								IBusPeakOCP1_count=0;
									user_list.userErr.bits.gIBusPeakOCP1 = 0;
							}
						}
						else
						{
					
							if(IBusPeakOCP1_count>IBus_Smart_Peak_OCP1_AdcValue)
							{
									IBusPeakOCP1_count-=IBus_Smart_Peak_OCP1_AdcValue;
							}
							else
							{
								IBusPeakOCP1_count=0;
									user_list.userErr.bits.gIBusPeakOCP1 = 0;
							}
						}
        }
		
#else
        if(user_list.IBusPeak_Adc > IBus_Peak_OCP1_AdcValue)
        {
            if(IBusPeakOCP1_count++ >= IBus_Peak_OCP1_MS_CONUT )
            {
               IBusPeakOCP1_count= IBus_Peak_OCP1_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP1_count)
            {
                IBusPeakOCP1_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				
				
				  if(user_list.IBusPeak_Adc > IBus_Peak_OCP2_AdcValue)
        {
            if(IBusPeakOCP2_count++ >= IBus_Peak_OCP2_MS_CONUT )
            {
               IBusPeakOCP2_count= IBus_Peak_OCP2_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP2_count)
            {
                IBusPeakOCP2_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				
					if(user_list.IBusPeak_Adc > IBus_Peak_OCP3_AdcValue)
        {
            if(IBusPeakOCP3_count++ >= IBus_Peak_OCP3_MS_CONUT )
            {
               IBusPeakOCP3_count= IBus_Peak_OCP3_MS_CONUT;

							user_list.userErr.bits.gIBusPeakOCP1 = 1; 
            }
        }
        else
        {
            if(IBusPeakOCP3_count)
            {
                IBusPeakOCP3_count--;
            }
            else
            {
//								user_list.userErr.bits.gIBusPeakOCP1 = 0;
            }
        }
				
				if(user_list.userErr.bits.gIBusPeakOCP1)
				{
					if(user_list.IBusPeak_Adc < IBus_Peak_OCP1_AdcValue)
					{
						IBusPeakREC_count++;
						if(IBusPeakREC_count>IBus_Peak_REC_MS_CONUT)
						{
							user_list.userErr.bits.gIBusPeakOCP1=0;
						}
										
					}
				}
				else 
				{
					IBusPeakREC_count=0;
				
				}
				
#endif		
}

void Volt_Handler_fast(void)
{
	
					u32 Vbus_temp=0;
				Vbus_temp=((user_list.Vcc_cal_gain*mc_core_list.Board_ADC_DATA.u16Vbus))>>15;
				#if(1)
	
	
        if((Vbus_temp < VBus_Stb_UVP_ADC) || (BatData.BatLowVolage_Flag)) // 0 ���Ը�Ϊ���ֹͣ
        {
      
								user_list.userErr.bits.gVBusStbUVP = 1; //��̬Ƿѹ����

        }
        else 
        {
			      	user_list.userErr.bits.gVBusStbUVP = 0;
        }

				#endif
        if(Vbus_temp < VBus_Run_UVP1_ADC)
        {
  
     
                user_list.userErr.bits.gVBusRunUVP1 = 1;
                

        }
        else
        {			
				user_list.userErr.bits.gVBusRunUVP1 = 0;
        }
        
       
        if(Vbus_temp < VBus_Run_UVP2_ADC)
        {
    
  
              user_list.userErr.bits.gVBusRunUVP2 = 1;
     
        }
        else
        {

					  user_list.userErr.bits.gVBusRunUVP2 = 0;
        }
        if(Vbus_temp > VBus_OVP_ADC)
        {
              user_list.userErr.bits.gVBusOVP = 1;
        } 
        else
        {
							user_list.userErr.bits.gVBusOVP = 0;
        }
}
void Volt_Handler(void)
{


	static u16 VBusStbUVP_Cnt=0;//上电只需检测一次
	static u16 VBusRunUVP1_Cnt=0;
	static u16 VBusRunUVP2_Cnt=0;
	static u16 	VBusOVP_Cnt=0;
		u32 Vbus_temp=0;
		Vbus_temp=((user_list.Vcc_cal_gain*mc_core_list.Board_ADC_DATA.u16Vbus))>>15;
	
    #if EN_VOLATAGE_PROTECT    
				user_list.VBus_Vx100=(Vbus_temp*VBUS_BASE_X100)>>12;
        if(((Vbus_temp < VBus_Stb_UVP_ADC)&&(user_list.user_state != TOOL_RUN)) || (BatData.BatLowVolage_Flag)) // 0 可以改为电机停止
        {
            if(VBusStbUVP_Cnt++ >= VBus_Stb_UVP_FiltMS_COUNT)
            {
                VBusStbUVP_Cnt= VBus_Stb_UVP_FiltMS_COUNT;

								user_list.userErr.bits.gVBusStbUVP = 1; //静态欠压保护
            }
        }
        else 
        {
			if(VBusStbUVP_Cnt)
			{
				VBusStbUVP_Cnt--;
			}
			else
			{
					user_list.userErr.bits.gVBusStbUVP = 0;
			}
        }


        if(Vbus_temp < VBus_Run_UVP1_ADC)
        {
            if(VBusRunUVP1_Cnt++ >= VBus_Run_UVP1_FiltMS_COUNT)
            {
                VBusRunUVP1_Cnt= VBus_Run_UVP1_FiltMS_COUNT;
     
                user_list.userErr.bits.gVBusRunUVP1 = 1;
                
            }
        }
        else
        {
			
            if(VBusRunUVP1_Cnt)
            {
                VBusRunUVP1_Cnt--;
            }
			else
            {
							user_list.userErr.bits.gVBusRunUVP1 = 0;
            }
        }
        
       
        if(Vbus_temp < VBus_Run_UVP2_ADC)
        {
            if(VBusRunUVP2_Cnt++ >=VBus_Run_UVP2_FiltMS_COUNT)
            {
                VBusRunUVP2_Cnt= VBus_Run_UVP2_FiltMS_COUNT;
  
                user_list.userErr.bits.gVBusRunUVP2 = 1;
            }
        }
        else
        {
            if(VBusRunUVP2_Cnt)
            {
                VBusRunUVP2_Cnt--;
            }
						else
            {
					  	user_list.userErr.bits.gVBusRunUVP2 = 0;
            }
        }



        if(Vbus_temp > VBus_OVP_ADC)
        {
            if(VBusOVP_Cnt++ >= VBus_OVP_FiltMS_COUNT)
            {
                VBusOVP_Cnt= VBus_OVP_FiltMS_COUNT;

                user_list.userErr.bits.gVBusOVP = 1;
            }
        } 
        else
        {
            if(VBusOVP_Cnt)
            {
                VBusOVP_Cnt--;
            }
						else
            {
							user_list.userErr.bits.gVBusOVP = 0;
            }
        }


		#endif
}



void User_Temperature_Handler_Fast(void)
{
	 #if EN_MOS_TEMP_DETECT
				u32 T_mos_temp=0;
				T_mos_temp=((user_list.Vcc_cal_gain*mc_core_list.Board_ADC_DATA.u16NTC_MOS))>>15;
	
				 if(T_mos_temp> RSM_MOS_TEMP_OVER_THD_ADC)//        
				{

						user_list.userErr.bits.gNtcMos=0;
				}
				 if(
				((mc_core_list.Board_ADC_DATA.u16NTC_MOS < MOS_TEMP_OVER_THD_ADC))
				 )//按键复用
				 {
				 
					user_list.userErr.bits.gNtcMos=1;
				 
				 }
		#endif

	#if EN_BAT_TEMP_DETECT
		if(BatData.TempAnomaly_Flag)
		{
			user_list.userErr.bits.gNtcBat=1;
		}
		else
		{
			user_list.userErr.bits.gNtcBat=0;
		}
	#endif
}

void User_Temperature_Handler(void)
{

		static u16 NtcMosErrClr_Cnt=0;
		static u16 NtcMosErr_Cnt=0;
	#if EN_BAT_TEMP_DETECT
		static u16 NtcBatErrClr_Cnt=0;
		static u16 NtcBatErr_Cnt=0;
	#endif
    #if EN_MOS_TEMP_DETECT
  			u32 T_mos_temp=0;
				T_mos_temp=((user_list.Vcc_cal_gain*mc_core_list.Board_ADC_DATA.u16NTC_MOS))>>15;
        if(user_list.userErr.bits.gNtcMos == 1)
        {
            if(T_mos_temp> RSM_MOS_TEMP_OVER_THD_ADC)//        
            {
                if(NtcMosErrClr_Cnt++ >= RSM_MOS_TEMP_OVER_TIME)
                {
                    NtcMosErrClr_Cnt= RSM_MOS_TEMP_OVER_TIME;
                    user_list.userErr.bits.gNtcMos=0;
                }
            }
            else
            {
                if(NtcMosErrClr_Cnt)
                {
                    NtcMosErrClr_Cnt--;
                }

            }
            NtcMosErr_Cnt= 0;
        }
        else
        {
           NtcMosErrClr_Cnt= 0;
            if(
            (T_mos_temp <MOS_TEMP_OVER_THD_ADC )//
						//(strInput.NtcMos_Adc < strInput.NtcMosOTP_Adc)
            //||(strInput.NtcMos_Adc > strInput.NtcMosUTP_Adc)
            )
            {
                if(NtcMosErr_Cnt++ >= MOS_TEMP_OVER_TIME)
                {
                  NtcMosErr_Cnt= MOS_TEMP_OVER_TIME;
                    
									user_list.userErr.bits.gNtcMos=1;
                }
            }
            else
            {
                if(NtcMosErr_Cnt)
                {
                   NtcMosErr_Cnt--;
                }
            }
                    
            
        }
#endif

	#if EN_BAT_TEMP_DETECT
		if(user_list.userErr.bits.gNtcBat == 1)
		{
			if(BatData.TempAnomaly_Flag == 0)
			{
				if(NtcBatErrClr_Cnt++ >= RSM_MOS_TEMP_OVER_TIME)
				{
					NtcBatErrClr_Cnt= RSM_MOS_TEMP_OVER_TIME;
					user_list.userErr.bits.gNtcBat=0;
				}
			}
			else
			{
				if(NtcBatErrClr_Cnt)
				{
					NtcBatErrClr_Cnt--;
				}
			}
			NtcBatErr_Cnt= 0;
		}
		else
		{
			NtcBatErrClr_Cnt= 0;
			if(BatData.TempAnomaly_Flag)
			{
				if(NtcBatErr_Cnt++ >= MOS_TEMP_OVER_TIME)
				{
					NtcBatErr_Cnt= MOS_TEMP_OVER_TIME;
					user_list.userErr.bits.gNtcBat=1;
				}
			}
			else
			{
				if(NtcBatErr_Cnt)
				{
					NtcBatErr_Cnt--;
				}
			}
		}
	#endif


}

#endif
/**
 * @brief  上电飞钻检测
 *        上电时采集扳机电压ADC，若低于阈值判定为扳机处于按下状态（飞钻）
 *        触发fly_machine错误标志，防止上电时电机意外启动
 */
/*
void power_on_fly_machine_handle(void)
{
		u8 i=0;
		u32 fly_machine_adc_sum=0;
		u32 fly_machine_adc=0;
		ADC_Cmd(ADC1, DISABLE);
		ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);		// �ر�ADCת���ж�
		TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);	// �ر�TIM1�����ж�
		
    ADC_AnyChannelNumCfg(ADC1, 0);
    ADC_AnyChannelSelect(ADC1, 0, FLY_CHANNEL);	
    ADC_AnyChannelCmd(ADC1, ENABLE);
	
	
	
		ADC_ExternalTrigConvCmd(ADC1, DISABLE);			// �ر�ADC�ⲿ����ת��
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);					// ���ADCת����־λ
	  ADC_SampleTimeConfig(ADC1,0, ADC_SampleTime_240_5);
		ADC_Cmd(ADC1, ENABLE);
		systick_delay_us(100);
	
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);								// ��������ADCת��, ��ȡ��ֵ��������ֵ
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);		// �ȴ�ADCת�����
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);										// ���ADCת����ɱ�־λ
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);								// ��������ADCת��, ��ȡ��ֵ��������ֵ
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);		// �ȴ�ADCת�����
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);										// ���ADCת����ɱ�־λ
		fly_machine_adc_sum=0;
		for(i=0;i<8;i++)
		{
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);								// ��������ADCת��, ��ȡ��ֵ��������ֵ
			while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);		// �ȴ�ADCת�����
			ADC_ClearFlag(ADC1, ADC_FLAG_EOC);										// ���ADCת����ɱ�־λ
			fly_machine_adc_sum += (uint16_t)GET_ADC_VALUE(FLY_CHANNEL);
		}
		fly_machine_adc=fly_machine_adc_sum>>3;
		if(fly_machine_adc<983)  // 飞钻判定阈值（扳机电压低于此值视为扳机已按下）
		{
			machine_error_hold.userErr_hold.bits.fly_machine=1;
		
		}
		ADC_Cmd(ADC1, DISABLE);
		systick_delay_us(100);
		Board_ADC_Init();

}
*/

/**
 * @brief  方向检测处理
 *        读取机械换向开关状态，滤波10次确认后设置电机旋转方向
 *        仅在电机停止且运行使能撤销后允许换向
 */
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
/**
 * @brief  按键扫描
 *        检测按键输入，短按返回res=1，长按(1000ms以上)返回res=2
 * @retval 0:无按键 1:短按 2:长按
 */
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

//void gears_deal(void)
//{
//	if(key_scan())
//	{
//		user_list.gears_level++;
//		if(user_list.gears_level>1)
//		{
//		
//			user_list.gears_level=0;
//		}
//	}

//}
u32 PI_sum_i=0;
u32 PI_p=0;
u32	pwm_duty_aim_PI_OUT=0;
/**
 * @brief  目标PWM占空比计算
 *        TRG_DIGITAL_EN=1：直接赋值MAX_TOOL_DUTY，不使用扳机ADC值
 *        TRG_DIGITAL_EN=0：扳机ADC线性映射，并限幅至MIN_TOOL_DUTY～MAX_TOOL_DUTY
 */
void pwm_duty_aim_deal(void)
{
	
	#if(TRG_DIGITAL_EN)
		motor_control_list.pwm_duty_aim_per = MAX_TOOL_DUTY;
	#else
		/* 电位器连续调速：扳机电压线性映射为目标占空比 */
		if(mc_core_list.Board_ADC_DATA.u16VR < MIN_TRG_ADC)
		{
			motor_control_list.pwm_duty_aim_per = MIN_TOOL_DUTY;
		}
		else if(mc_core_list.Board_ADC_DATA.u16VR >= MAX_TRG_ADC)
		{
			motor_control_list.pwm_duty_aim_per = MAX_TOOL_DUTY;
		}
		else
		{
			motor_control_list.pwm_duty_aim_per = (uint16_t)((float)(mc_core_list.Board_ADC_DATA.u16VR - MIN_TRG_ADC)*K_ADC_DUTY + MIN_TOOL_DUTY);
		}
	#endif
	#if(0)
	
		
		#if 0
	if(motor_run_count<50)
	
	{
		pwm_duty_aim_PI_OUT=0;
	
	}
	else 
	{
		if(mc_core_list.time_count_60_degree>((u32)(1500*MOTOR_TIMR_FRQ_MHZ)))
		{
			PI_p=mc_core_list.time_count_60_degree-((u32)(1500*MOTOR_TIMR_FRQ_MHZ));
			PI_p=(PI_p>>2);
			if(PI_sum_i<PWM_PRIOD_LOAD)
			{
				PI_sum_i++;
				
			}
			pwm_duty_aim_PI_OUT=PI_sum_i+PI_p;
		}
		
		else 
		{
			PI_p=((u32)(1500*MOTOR_TIMR_FRQ_MHZ))-mc_core_list.time_count_60_degree;
			PI_p=(PI_p>>2);
			if(PI_sum_i>LIMIT_MIN_PWN_DUTY)
			{
				PI_sum_i--;
			}
			if(PI_sum_i>PI_p)
			{
				pwm_duty_aim_PI_OUT=PI_sum_i-PI_p;
			}
			else 
			{
				pwm_duty_aim_PI_OUT=LIMIT_MIN_PWN_DUTY;
			
			}
		}
		

	}
	
		if(pwm_duty_aim_PI_OUT>LIMIT_MAX_PWN_DUTY)
		{
			pwm_duty_aim_PI_OUT=LIMIT_MAX_PWN_DUTY;
		
		}
			if(pwm_duty_aim_PI_OUT>motor_control_list.pwm_duty_aim_per)
		{
			motor_control_list.pwm_duty_aim=pwm_duty_aim_PI_OUT;
		}
		else 
		{
		
			motor_control_list.pwm_duty_aim=motor_control_list.pwm_duty_aim_per;
		}
		#else
//		motor_control_list.pwm_duty_aim_per=(u16)((PWM_PRIOD_LOAD)*1.0f);
		motor_control_list.pwm_duty_aim=motor_control_list.pwm_duty_aim_per;
		#endif
	#endif

}	
uint8_t pulse_mode=0;
uint16_t pulse_max_duty=0;
/**
 * @brief  脉冲模式判断
 *        当目标占空比低于80%时进入脉冲模式（用于低速扭矩控制）
 */
void pulse_th_handle(void)
{
	if(motor_control_list.pwm_duty_aim_per<((u32)(PWM_PRIOD_LOAD*0.8f)))
	{
		
		pulse_mode=1;
	}
	else 
	{
		pulse_mode=0;
	}
	if(user_list.flag.bits.gToolTrigger==0)
	{
		pulse_max_duty=((u32)(PWM_PRIOD_LOAD*0.14f));
	
	}


}
/**
 * @brief  齿轮/挡位处理
 *        根据当前挡位对目标占空比进行比例缩放
 *        挡位0: PWM_DUTY_GEARS1(50%) 挡位1: PWM_DUTY_GEARS2(80%) 挡位2+: PWM_DUTY_GEARS3(100%)
 */
void user_gears_handle(void)
{
	
	#if 0
		u8 res=0;
		static uint8_t pulse_dir=0;
		static uint16_t pulse_cout=0;
		pulse_th_handle();
		res=key_scan();
		if(user_list.user_state==TOOL_STOP)
		{
				if(res==1)
				{
					if(user_list.gears_level<1)
					{
						user_list.gears_level++;
						
					}
					else
					{
						user_list.gears_level=0;
					}
					save_user_data();	
			//		user_list.gears_level_changed=1;
				}
		}

	if(user_list.gears_level==0)
	{
		if(pulse_mode)
		{
			motor_control_list.pwm_duty_aim_per=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS1)>>15;
		
			if(pulse_dir==0)
			{
				
				if(motor_control_list.pwm_duty_aim>LIMIT_MIN_PWN_DUTY)
				{
					motor_control_list.pwm_duty_aim-=4;
				}
				pulse_cout++;
				if(pulse_cout>=250)
				{
					pulse_dir=1;
				}
			}
			else 
			{
				if(motor_control_list.pwm_duty_aim<pulse_max_duty||motor_control_list.pwm_duty_aim<((u32)(PWM_PRIOD_LOAD*0.20)))
				{
					motor_control_list.pwm_duty_aim+=4;

					
				}
				
				if(pulse_cout) 
				{
					pulse_cout--;
					
				}
				else 
				{
					pulse_max_duty+=((u32)(PWM_PRIOD_LOAD*0.02f));
					if(pulse_max_duty>((u32)(PWM_PRIOD_LOAD*0.45f)))
					{
						
						pulse_max_duty=((u32)(PWM_PRIOD_LOAD*0.45f));
					}
					pulse_dir=0;
				}
			
			}
		}
		else 
		{
			motor_control_list.pwm_duty_aim=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS1)>>15;
		
		}
		
	}
	else 
	{
	
		motor_control_list.pwm_duty_aim=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS3)>>15;
	}
	#else 
	motor_control_list.pwm_duty_aim=motor_control_list.pwm_duty_aim_per;
	#endif
}
/**
 * @brief  电动扳手按键处理
 *        正转时切换挡位(0/1/2)，反转时切换自动停机功能
 *        根据挡位计算目标占空比，并处理自动停机逻辑（反转电流低于阈值时停机）
 */
void user_WrenchKey_Handler(void)
{
//	gears_deal();//ɨ�赵λ
	uint16_t pwm_duty_aim_temp=0;
	static uint16_t auto_current_ad=0;
	static uint16_t auto_stop_time=0;
	u8 res=0;
	res=key_scan();
	if(motor_control_list.bldc_state!=MOTOR_RUN&&motor_control_list.bldc_state!=MOTOR_MOTION&&motor_control_list.bldc_state!=MOTOR_POSITION)
	{
			if(user_list.flag.bits.gToolDir==Tool_Dir_CW_Value)
			{
				if(res)
				{
					user_list.gears_level++;
					if(user_list.gears_level>2)
					{
					
						user_list.gears_level=0;
					}
					save_user_data();
				}
			}
			else 
			{
//				if(res)
//				{
//					user_list.gears_ccw_level++;
//					if(user_list.gears_ccw_level>3)
//					{
//					
//						user_list.gears_ccw_level=0;
//					}
//					save_user_data();
//				}
				if(res)
				{
						user_list.flag.bits.gAutoStopEn=!user_list.flag.bits.gAutoStopEn;
					save_user_data();
				}
			
			}
	}
	
	if(user_list.flag.bits.gToolDir==Tool_Dir_CCW_Value)
	{
//		if(user_list.gears_ccw_level==0)
//		{
//			user_list.flag.bits.gAutoStopEn=1;
//		}
//		else 
//		{
//			user_list.flag.bits.gAutoStopEn=0;
//		}
			if(user_list.flag.bits.gAutoStopEn)
			{
				pwm_duty_aim_temp=PWM_PRIOD_LOAD;
			}
			else 
			{
				
//				if(motor_control_list.pwm_duty_aim_per<((u16)(0.3f*PWM_PRIOD_LOAD)))
//				{
//					motor_control_list.pwm_duty_aim_per=((u16)(0.3f*PWM_PRIOD_LOAD));
//				}
			//	motor_control_list.pwm_duty_aim=motor_control_list.pwm_duty_aim_per;

//					if(user_list.gears_ccw_level==0)
//					{
//						pwm_duty_aim_temp=PWM_PRIOD_LOAD;
//					}
//					else if(user_list.gears_ccw_level==1)
//					{
//						pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS2)>>15;
//					}
//					else if(user_list.gears_ccw_level==2)
//					{
//						pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS3)>>15;
//					}
//					else 
//					{
//					
//						pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS4)>>15;
//					}
//					if(pwm_duty_aim_temp<((u16)(PWM_PRIOD_LOAD*0.50f)))
//					{
//						pwm_duty_aim_temp=((u16)(PWM_PRIOD_LOAD*0.50f));
//					}
				
				pwm_duty_aim_temp=motor_control_list.pwm_duty_aim_per;
				
			}
			
	}
	else 
	{
	
		
		if(user_list.gears_level==0)
		{
			pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS1)>>15;
		}
		else if(user_list.gears_level==1)
		{
			pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS2)>>15;
		}
		else 
		{
		
			pwm_duty_aim_temp=(motor_control_list.pwm_duty_aim_per*PWM_DUTY_GEARS3)>>15;
		}
		
		
		if(pwm_duty_aim_temp>Wrench_CW_MAX_DUTY)
		{
			pwm_duty_aim_temp=(Wrench_CW_MAX_DUTY);
		}

	}
	
		#if(1)
		if(user_list.Load_exist)
		{
			#if 0
				if(motor_control_list.motor_run_count<(180*SLOWLOOP_1ms_CNT_LOAD))
				
				{
					pwm_duty_aim_PI_OUT=(0.65f*PWM_PRIOD_LOAD);
				
				}
				else 
				{
					if(mc_core_list.time_count_60_degree>((u32)(1000*MOTOR_TIMR_FRQ_MHZ)))
					{
						PI_p=mc_core_list.time_count_60_degree-((u32)(1000*MOTOR_TIMR_FRQ_MHZ));
						PI_p=(PI_p>>2);
						if(PI_sum_i<PWM_PRIOD_LOAD)
						{
							PI_sum_i++;
							
						}
						pwm_duty_aim_PI_OUT=PI_sum_i+PI_p;
					}
					
					else 
					{
						PI_p=((u32)(1000*MOTOR_TIMR_FRQ_MHZ))-mc_core_list.time_count_60_degree;
						PI_p=(PI_p>>2);
						if(PI_sum_i>LIMIT_MIN_PWN_DUTY)
						{
							PI_sum_i--;
						}
						if(PI_sum_i>PI_p)
						{
							pwm_duty_aim_PI_OUT=PI_sum_i-PI_p;
						}
						else 
						{
							pwm_duty_aim_PI_OUT=LIMIT_MIN_PWN_DUTY;
						
						}
					}
					

				}
				
					if(pwm_duty_aim_PI_OUT>LIMIT_MAX_PWN_DUTY)
					{
						pwm_duty_aim_PI_OUT=LIMIT_MAX_PWN_DUTY;
					
					}
						if(pwm_duty_aim_PI_OUT>pwm_duty_aim_temp)
					{
						motor_control_list.pwm_duty_aim=pwm_duty_aim_PI_OUT;
					}
					else 
					{
					
						motor_control_list.pwm_duty_aim=pwm_duty_aim_temp;
					}
					#endif
					if(pwm_duty_aim_temp<RESTAR_MIN_PWM_AIM_DUTY)
					{
						motor_control_list.pwm_duty_aim=RESTAR_MIN_PWM_AIM_DUTY;
					}
					else 
					{
					
					motor_control_list.pwm_duty_aim=pwm_duty_aim_temp;
					}
					
		}
		else 
		{
			motor_control_list.pwm_duty_aim=pwm_duty_aim_temp;
		}
	#else
	
	motor_control_list.pwm_duty_aim=pwm_duty_aim_temp;
	#endif
	
	if(user_list.flag.bits.gAutoStopEn&&user_list.flag.bits.gToolDir==Tool_Dir_CCW_Value&&user_list.user_state==TOOL_RUN)
	{
				if(user_list.CCW_Auto_Mask_count<63334)
				{
					user_list.CCW_Auto_Mask_count++;
				}
				if(user_list.CCW_Auto_Mask_count==CCW_AUTO_STOP_START_MASK_COUNT)
				{
					auto_current_ad=user_list.IBusAvg_Adc;
					auto_current_ad=((((u32)auto_current_ad)*((u32)21299))>>15);  // 计算自动停机电流阈值（约乘以0.65）
//					if(auto_current_ad>CCW_AUTO_STOP_CURRENT_2_AdcValue)
//					{
//						auto_current_ad=(auto_current_ad-CCW_AUTO_STOP_CURRENT_2_AdcValue);
//					}
//					else 
//					{
//						auto_current_ad=0;
//					}
					if(auto_current_ad<CCW_AUTO_STOP_CURRENT_AdcValue)
					{
						auto_current_ad=CCW_AUTO_STOP_CURRENT_AdcValue;
					}
					
					if(auto_current_ad>CCW_AUTO_STOP_CURRENT_3_AdcValue)
					{
						auto_current_ad=CCW_AUTO_STOP_CURRENT_3_AdcValue;
					}
				
				}
				#if 0
				if(user_list.IBusAvg_Adc < auto_current_ad&&user_list.CCW_Auto_Mask_count>CCW_AUTO_STOP_START_MASK_COUNT)
				{						
						if(mc_core_list.Run_SectorCnt > AUTO_STOP_Sector_CNT) //
						{
								user_list.flag.bits.gAutoStop = 1;  // 触发自动停机

								mc_core_list.Run_SectorCnt = 0;	
						}		
				}
				else
				{
								
								mc_core_list.Run_SectorCnt = 0;
				}
				#else 
				if(user_list.CCW_Auto_Mask_count>CCW_AUTO_STOP_START_MASK_COUNT)
				{
				
					if(user_list.IBusAvg_Adc < auto_current_ad)
					{
						auto_stop_time++;
						if(auto_stop_time > AUTO_STOP_Sector_CNT) //
						{
								user_list.flag.bits.gAutoStop = 1;
								auto_stop_time=0;
								mc_core_list.Run_SectorCnt = 0;	
						}	
					}
					else 
					{
						if(auto_stop_time)
						{
							auto_stop_time--;
						}
					}
					
				}
				else 
				{
				
					auto_stop_time=0;
				}
				
				#endif
				
	}
	else 
	{
			auto_stop_time=0;
			mc_core_list.Run_SectorCnt = 0;	
			user_list.CCW_Auto_Mask_count=0;
	}
	

}




/**
 * @brief  电机开关控制
 *        数字模式：PA1低电平松开、高电平按下；ADC模式：低于TRG_ON_ADC-100松开，达到TRG_ON_ADC按下
 *        对应计数超过20次后更新gToolTrigger标志（1ms任务调用）
 */
void motor_on_off_control(void)
{
	static 	u8 gToolTrigger_off_count=0;
	static 	u8 gToolTrigger_on_count=0;
//		user_list.flag.bits.gToolTrigger=1;
	#if(TRG_DIGITAL_EN)
		/* SPEED 外部下拉，按键闭合接入高电平。 */
		if(GPIO_ReadInputDataBit(VR_PORT, VR_PIN) == Bit_RESET)
		{
			gToolTrigger_off_count++;
			gToolTrigger_on_count=0;
			if(gToolTrigger_off_count>20)
			{
				user_list.flag.bits.gToolTrigger=0;
			}
		}
		else
		{
			gToolTrigger_off_count=0;
			gToolTrigger_on_count++;
			if(gToolTrigger_on_count>20)
			{
				user_list.flag.bits.gToolTrigger=1;
			}
		}
	#else
		/* ADC 模式：VR 电压与阈值比较 */
		if(mc_core_list.Board_ADC_DATA.u16VR < (TRG_ON_ADC - 100))
		{
			gToolTrigger_off_count++;
			gToolTrigger_on_count=0;
			if(gToolTrigger_off_count>20)
			{
				user_list.flag.bits.gToolTrigger=0;
			}
		}
		if(mc_core_list.Board_ADC_DATA.u16VR >= TRG_ON_ADC)
		{
			gToolTrigger_off_count=0;
			gToolTrigger_on_count++;
			if(gToolTrigger_on_count>20)
			{
				user_list.flag.bits.gToolTrigger=1;
			}
		}
	#endif
	
	
	#if 0
		if(GPIO_ReadInputDataBit(DIR_PORT,DIR_PIN)==1) 
	{
		gToolTrigger_off_count++;
		gToolTrigger_on_count=0;
		if(gToolTrigger_off_count>10)
		{
			user_list.flag.bits.gToolTrigger=0;
		}
	
	}
	if(GPIO_ReadInputDataBit(DIR_PORT,DIR_PIN)==0) 
	{
				gToolTrigger_off_count=0;
	   	gToolTrigger_on_count++;
			if(gToolTrigger_on_count>10)
			{
			user_list.flag.bits.gToolTrigger=1;
			}
	}
	
	#endif

}

/**
 * @brief  故障锁存更新
 *        将当前MC错误和用户错误锁存到machine_error_hold中
 *        锁存后需手动清除才能恢复
 */
void error_hold_updata(void)
{
	machine_error_hold.MC_error_hold.word|=mc_core_list.MC_error.word;
	machine_error_hold.userErr_hold.word|=user_list.userErr.word;

}

/**
 * @brief  工具状态机控制
 *        状态: TOOL_STOP -> TOOL_RUN -> TOOL_ERROR_STOP
 *              TOOL_STOP -> TOOL_POWER_DOWN(休眠)
 *        根据扳机、故障、自动停机标志进行状态切换
 */
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
