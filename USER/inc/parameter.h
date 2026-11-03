#ifndef __PARAMETER_H_
#define __PARAMETER_H_

#include "HAL_device.h"


//----------------------------------------------------------------------//
/* System setting */
//----------------------------------------------------------------------//
#define SYSCLK_HSI			       	    ((u32)60000000)   									// 系统时钟频率
#define PWMFREQ                	   	((u16)16000)    										// PWM频率
#define PWM_PRIOD_LOAD 	       			(u16)(SYSCLK_HSI/PWMFREQ/2-1)  			// PWM周期装载值

#define DEADTIME 					       		(1)      														// us, 死区时间
#define DEADTIME_LOAD 		       		((u16)(DEADTIME*SYSCLK_HSI/1000000))	 	// 死区装载值

#define SLOWLOOP_500us_CNT_LOAD     ((u16)(PWMFREQ/1000/2))							// TIM1 基准 500us
#define SLOWLOOP_1ms_CNT_LOAD      	((u16)(PWMFREQ/1000))								// TIM1 基准 1ms
#define SLOWLOOP_400ms_CNT_LOAD    	((u16)400*SLOWLOOP_1ms_CNT_LOAD)		// TIM1 基准 200ms

#define TIM3_TO_PWM_PERIOD 	       	(float)(1000000/PWMFREQ)  		

#define SLOWLOOP_FRE                (1000) //慢循环频率，单位1ms

#define  SLEEP_TIME                 				5000ul//10000

#define COM_50US_CNT_LOAD      		25

//---------------------------------------------------------------------//
// PWM 
//---------------------------------------------------------------------//

/*--------------------------------------
电机1
#define Wrench_EN                   (0)
#define RESTAR_EN                  (0)
#define USER_ROTATE_ERROR_EN      (1)  //堵转反转保护---电机1默认打开
#define Block_Protect_RUNING_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)堵转时间unit:ms

电机2
#define Wrench_EN                   (1)
#define RESTAR_EN                  (1)
#define RESTAR_COUNT               (20)
#define RESTAR_MIN_PWM_AIM_DUTY    ((u16)(0.5f*PWM_PRIOD_LOAD))
#define RESTAR_MIN_PWM_LIMIT_DUTY    ((u16)(0.25f*PWM_PRIOD_LOAD))

#define Wrench_CW_MAX_DUTY             ((u16)(PWMFREQ*0.96f))
#define USER_ROTATE_ERROR_EN      (0)  //堵转反转保护---电机1默认打开

#define Block_Protect_RUNING_TIME                     (20*SLOWLOOP_1ms_CNT_LOAD)堵转时间unit:ms
----------------------------------------*/



#define Wrench_EN                   (0)
#define RESTAR_EN                  (0)
#define RESTAR_COUNT               (20)
#define RESTAR_MIN_PWM_AIM_DUTY    ((u16)(0.5f*PWM_PRIOD_LOAD))
#define RESTAR_MIN_PWM_LIMIT_DUTY    ((u16)(0.25f*PWM_PRIOD_LOAD))

#define Wrench_CW_MAX_DUTY             ((u16)(PWM_PRIOD_LOAD*0.96f))

#define USER_ROTATE_ERROR_EN      (1)  //堵转反转保护---电机1默认打开

#define LIMIT_SPEED_EN                 (0)  //限速



#define LED_SCAN_EN       (0)
#define PWM_DUTY_GEARS1    (u32)(32768 * 0.5f)  
#define PWM_DUTY_GEARS2    (u32)(32768 * 0.80f)    
#define PWM_DUTY_GEARS3    (u32)(32768 * 1.0f) 
#define PWM_DUTY_GEARS4    (u32)(32768 * 1.0f) 
//---------------------------------------------------------------------------------------------------------------------------------------------------//
//************************************************************************************************************************************************/


//----------------------------------------------------------------------//
// 扳机检测模式
//  1 = 数字 IO 模式（PA1 高电平按下、低电平松开，目标占空比直接赋 MAX_TOOL_DUTY）
//  0 = ADC 连续调速模式（PA1 模拟电压映射占空比）
//----------------------------------------------------------------------//
#define TRG_DIGITAL_EN                         (0)

//vsp 占空比控制参数
#define MAX_TOOL_DUTY 		                	(PWM_PRIOD_LOAD)//高电平最大占空比
#define MIN_TOOL_DUTY 		   		 						(PWM_PRIOD_LOAD*0.15f) //电机最小占空比

#define TRG_ON_ADC               						300//电机启动触发ADC
#define MAX_TRG_ADC 		   		   						4000//电机触发最大值
#define MIN_TRG_ADC              						600//电机触发最小值
#define K_ADC_DUTY			   									(float)(MAX_TOOL_DUTY-MIN_TOOL_DUTY) / (float)(MAX_TRG_ADC - MIN_TRG_ADC)
	
//---PMW_duty---限速参数，此处限速VSP调节后的PWM_DUTY范围，即PWM范围 由VSP和LIMIT_MAX_DUTY和VSP和LIMIT_MIN_DUTY同时决定

#define LIMIT_MIN_PWN_DUTY   ((u16)(PWM_PRIOD_LOAD*0.15f)) 

#define LIMIT_MAX_PWN_DUTY   (PWM_PRIOD_LOAD) 


//---------------------------------------------------------------------//
// ADC 
//---------------------------------------------------------------------//
#define ADC_REFV                 		5													// unit:V, ADC参考电压
#define ADC_REF_VALUE          	   	4095											// 12-bit ADC

/* ------------------------------ ADC校准相关参数 ---------------------------- */
#define EN_OFFSET_CALIB       			(1)      									/* 失调校准使能开关，通常打开 */
#define CALIB_SAMPLES_K            	(10)   										// 2^10 = 1024, 校准采样点数
#define CALIB_SAMPLES              	(1024)										// 校准采样点数
#define CALIB_SAMPLES_DOUBLE       	(CALIB_SAMPLES << 1)  
#define CURRENT_OFFSET_VOLTAGE     	(0.0)    									/* 电流偏置电压, 根据电路实测调整, unit:V */
/* 将用户设定值转换为ADC采样值 */
#define CURRENT_OFFSET_VALUE				(u16)(ADC_REF_VALUE * CURRENT_OFFSET_VOLTAGE / ADC_REFV)
#define CURRENT_OFFSET_H_THD				(u16)(CURRENT_OFFSET_VALUE + 500)
#define CURRENT_OFFSET_L_THD				(u16)(CURRENT_OFFSET_VALUE - 500)

/* ---------------------------- Current Protect  ---------------------------------- */

/* ------------------------------ 电流采样硬件参数 ---------------------------- */
#define RSHUNT 												 1.0f							/* 采样电阻，单位：mR */
#define AMPLIFICATION_GAIN 						 (float)(10.89)		/* 运放放大倍数 */
	
//#define MAX_BUS_CURRENT_SETTINT        (float)(18.0)                                   						/* 母线电流，单位：A */
#define CURRENT_ADC_PER_A              (float)(RSHUNT * AMPLIFICATION_GAIN * ADC_REF_VALUE / ADC_REFV / 1000.0f)  			/* 每安培电流ADC值 */
//#define CURRENT_LIM_VALUE              (u16)(MAX_BUS_CURRENT_SETTINT * CURRENT_ADC_PER_A) 				/* 电流ADC值 */

//短路保护
#define MOS_SHORT_CURRENT               (105.0f)
#define MOS_SHORT_CURRENT_AdcValue    (u16)(MOS_SHORT_CURRENT*CURRENT_ADC_PER_A)


#define IBus_Avg_REC_MS               (200) //ms
#define IBus_Avg_REC_MS_CONUT         (u16)((float)IBus_Avg_REC_MS*SLOWLOOP_FRE/1000.0f) 

#define IBus_Avg_OCP1                (30.0f)
#define IBus_Avg_OCP1_AdcValue         (u16)(IBus_Avg_OCP1*CURRENT_ADC_PER_A)

#define IBus_Avg_OCP1_MS               (3000) //ms
#define IBus_Avg_OCP1_MS_CONUT         (u16)((float)IBus_Avg_OCP1_MS*SLOWLOOP_FRE/1000.0f) 
	
#define IBus_Avg_OCP2                 (55.0f)
#define IBus_Avg_OCP2_AdcValue         (u16)(IBus_Avg_OCP2*CURRENT_ADC_PER_A)

#define IBus_Avg_OCP2_MS               (500) //ms
#define IBus_Avg_OCP2_MS_CONUT        (u16)((float)IBus_Avg_OCP2_MS*SLOWLOOP_FRE/1000.0f) 

#define IBus_Avg_OCP3                  (68.0f)
#define IBus_Avg_OCP3_AdcValue         (u16)(IBus_Avg_OCP3*CURRENT_ADC_PER_A)

#define IBus_Avg_OCP3_MS               (80) //ms
#define IBus_Avg_OCP3_MS_CONUT        (u16)((float)IBus_Avg_OCP3_MS*SLOWLOOP_FRE/1000.0f)

#define IBus_Avg_OCP4                  (80.0f)
#define IBus_Avg_OCP4_AdcValue         (u16)(IBus_Avg_OCP4*CURRENT_ADC_PER_A)

#define IBus_Avg_OCP4_MS               (5) //ms
#define IBus_Avg_OCP4_MS_CONUT        (u16)((float)IBus_Avg_OCP4_MS*SLOWLOOP_FRE/1000.0f)


#define Block_Protect_En                                        (1)/*堵转保护*/

#define Block_Protect_Current_En                                 (1)/*根据电流采样堵转保护时间*/
#define Block_Protect_Recover_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)/*堵转恢复时间unit:ms*/

#if Block_Protect_Current_En
	
//此处堵转时间注意根据实际情况调整
#define Block_Protect_MAX_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/
//此处堵转时间注意根据实际情况调整
#define Block_Protect_RUNING_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

#define Block_Protect_Current1                      (40.00)/*unit:a*/
#define Block_Protect_Current1_AdcValue             (u16)(Block_Protect_Current1*CURRENT_ADC_PER_A)
#define Block_Protect_Current1_TIME                 (80*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/


#define Block_Protect_Current2                      (55.00)/*unit:a*/
#define Block_Protect_Current2_AdcValue             (u16)(Block_Protect_Current2*CURRENT_ADC_PER_A)
#define Block_Protect_Current2_TIME                 (50*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/


#define Block_Protect_Current3                      (65.00)/*unit:a*/
#define Block_Protect_Current3_AdcValue             (u16)(Block_Protect_Current3*CURRENT_ADC_PER_A)
#define Block_Protect_Current3_TIME                 (30*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

#define Block_Protect_Current4                      (80.00)/*unit:a*/
#define Block_Protect_Current4_AdcValue             (u16)(Block_Protect_Current4*CURRENT_ADC_PER_A)
#define Block_Protect_Current4_TIME                 (10*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/



#else
#define Block_Protect_LowDuty_DATA									            (u16)(0.70f*PWM_PRIOD_LOAD)/*堵转保护低占空比*/
#define Block_Protect_LowDuty_COUNT                            (80*SLOWLOOP_1ms_CNT_LOAD)/*低占空比堵转时间unit:ms*/
#define Block_Protect_HighDuty_COUNT                           (30*SLOWLOOP_1ms_CNT_LOAD)/*高占空比堵转时间unit:ms*/

#endif


#define Multistage_Integral_Peak_OCP_EN              (1) //多级积分峰值保护，共用一个保护标志位，兼容MM32F5系列IPD




#if Multistage_Integral_Peak_OCP_EN



	#define IBus_Smart_Peak_OCP1                      (60.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP1_AdcValue             (u16)(IBus_Smart_Peak_OCP1*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP1_MS                   (800) //ms
	#define IBus_Smart_OCP1_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP1_MS*SLOWLOOP_FRE/1000.0f))

	#define IBus_Smart_Peak_OCP2                      (70.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP2_AdcValue             (u16)(IBus_Smart_Peak_OCP2*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP2_MS                   (100) //ms
	#define IBus_Smart_OCP2_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP2_MS*SLOWLOOP_FRE/1000.0f))
		
	#define IBus_Smart_Peak_OCP3                      (80.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP3_AdcValue             (u16)(IBus_Smart_Peak_OCP3*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP3_MS                   (10) //ms
	#define IBus_Smart_OCP3_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f))
	
	#define IBus_Smart_REC_MS                         80
	#define IBus_Smart_REC_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_REC_MS*SLOWLOOP_FRE/1000.0f))
#else

	#define IBus_Peak_OCP1                  (50.00)/*unit:a*/
	#define IBus_Peak_OCP1_AdcValue         (u16)(IBus_Peak_OCP1*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP1_MS               (800) //ms
	#define IBus_Peak_OCP1_MS_CONUT        (u16)((float)IBus_Peak_OCP1_MS*SLOWLOOP_FRE/1000.0f)
	
	
	#define IBus_Peak_OCP2                  (60.00)/*unit:a*/
	#define IBus_Peak_OCP2_AdcValue         (u16)(IBus_Peak_OCP2*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP2_MS               (90) //ms
	#define IBus_Peak_OCP2_MS_CONUT        (u16)((float)IBus_Peak_OCP2_MS*SLOWLOOP_FRE/1000.0f)
	
	#define IBus_Peak_OCP3                  (70.00)/*unit:a*/
	#define IBus_Peak_OCP3_AdcValue         (u16)(IBus_Peak_OCP3*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP3_MS               (10) //ms
	#define IBus_Peak_OCP3_MS_CONUT        (u16)((float)IBus_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f)

	
  #define IBus_Peak_REC_MS               (150) //ms
	#define IBus_Peak_REC_MS_CONUT        (u16)((float)IBus_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f)
#endif


#define IBus_Peak_IT_OCP               (100.0f)  //峰值保护，单次超过触发
#define IBus_Peak_IT_OCP_AdcValue      (u16)(IBus_Peak_IT_OCP*CURRENT_ADC_PER_A)
#define IBus_Peak_IT_COUNT     				 (u16)(PWMFREQ/1000)//PWMFREQ/1000=1MS

#define IBus_Peak_OCP_ONCE_EN              (1)
#define IBus_Peak_OCP_ONCE               (105.0f)  //峰值保护，单次超过触发
#define IBus_Peak_OCP_ONCE_AdcValue      (u16)(IBus_Peak_OCP_ONCE*CURRENT_ADC_PER_A)
#define IBus_IPD_OCP               (105.0f)  //峰值保护，单次超过触发
#define IBus_IPD_OCP_AdcValue      (u16)(IBus_IPD_OCP*CURRENT_ADC_PER_A)



#define LIMIT_PEAK_CURRENT_EN               (0)
#define LIMIT_PEAK_CURRENT                  (110.0f)
#define LIMIT_PEAK_CURRENT_AdcValue         (u16)(LIMIT_PEAK_CURRENT*CURRENT_ADC_PER_A)


#define LIMIT_AVG_CURRENT_EN               (0)
#define LIMIT_AVG_CURRENT4                  (35.0f)
#define LIMIT_AVG_CURRENT4_AdcValue         (u16)(LIMIT_AVG_CURRENT4*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT3                 (35.0f)
#define LIMIT_AVG_CURRENT3_AdcValue         (u16)(LIMIT_AVG_CURRENT3*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT2                  (25.0f)
#define LIMIT_AVG_CURRENT2_AdcValue         (u16)(LIMIT_AVG_CURRENT2*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT1                  (15.0f)
#define LIMIT_AVG_CURRENT1_AdcValue         (u16)(LIMIT_AVG_CURRENT1*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT_CCW                  (36.0f)
#define LIMIT_AVG_CURRENT_AdcValue_CCW         (u16)(LIMIT_AVG_CURRENT_CCW*CURRENT_ADC_PER_A)



/* ---------------------------- Voltage Protect Parameter -------------------------- */
#define EN_VOLATAGE_PROTECT    	       	(1)    														/* 母线电压保护功能使能 */
#define VBUS_SHUNT_RATIO           			(float)((1.0 + 10.0)/1.0)			// 母线电压分压比(下拉电阻/(上拉电阻+下拉电阻)) 10K/1K

#define VBUS_BASE_X100       ((u32)(VBUS_SHUNT_RATIO*ADC_REFV*100))

//40
#define VBus_Stb_UVP_V                                          (12.80f)/*unit:v*/
#define VBus_Stb_UVP_ADC     		       	                    (u16)(VBus_Stb_UVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)  		/* 稳态欠压保护阈值*/
#define VBus_Stb_UVP_FiltMS                                     (100)/*unit:ms*/
#define VBus_Stb_UVP_FiltMS_COUNT                               (u16)((float)VBus_Stb_UVP_FiltMS*SLOWLOOP_FRE/1000.0f)
                    
#define VBus_Run_UVP1_V                                         (12.80f)/*unit:v*/
#define VBus_Run_UVP1_ADC     		       	                      (u16)(VBus_Run_UVP1_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_Run_UVP1_FiltMS                                    (500)/*unit:ms*/
#define VBus_Run_UVP1_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP1_FiltMS*SLOWLOOP_FRE/1000.0f)
	
#define VBus_Res_UVP_V                                         (13.80f)/*unit:v*/
#define VBus_Res_UVP_ADC     		       	                      (u16)(VBus_Res_UVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_Res_UVP_FiltMS                                    (500)/*unit:ms*/
#define VBus_Res_UVP_FiltMS_COUNT                              (u16)((float)VBus_Res_UVP_FiltMS*SLOWLOOP_FRE/1000.0f)


#define VBus_Run_UVP2_V                                         (8.00f)/*unit:v*/
#define VBus_Run_UVP2_ADC     		       	                    (u16)(VBus_Run_UVP2_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_Run_UVP2_FiltMS                                    (10)/*unit:ms*/
#define VBus_Run_UVP2_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP2_FiltMS*SLOWLOOP_FRE/1000.0f)

    
#define VBus_OVP_V                                              (22.30f)/*unit:v*/
#define VBus_OVP_ADC     		       	                             (u16)(VBus_OVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_OVP_FiltMS                                         (300)/*unit:ms*/
#define VBus_OVP_FiltMS_COUNT                                   (u16)((float)VBus_OVP_FiltMS*SLOWLOOP_FRE/1000.0f)

#define SOC3_MV                     	                        (u32)((18.2)*100)
#define SOC2_MV                     	                        (u32)((17.2)*100)
#define SOC1_MV                     	                        (u32)((16.2)*100)

//20
//#define VBus_Stb_UVP_V                                          (14.80f)/*unit:v*/
//#define VBus_Stb_UVP_ADC     		       	                        (u16)(VBus_Stb_UVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)  		/* 稳态欠压保护阈值*/
//#define VBus_Stb_UVP_FiltMS                                     (100)/*unit:ms*/
//#define VBus_Stb_UVP_FiltMS_COUNT                               (u16)((float)VBus_Stb_UVP_FiltMS*SLOWLOOP_FRE/1000.0f)
//                    
//#define VBus_Run_UVP1_V                                         (13.2f)/*unit:v*/
//#define VBus_Run_UVP1_ADC     		       	                      (u16)(VBus_Run_UVP1_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
//#define VBus_Run_UVP1_FiltMS                                    (500)/*unit:ms*/
//#define VBus_Run_UVP1_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP1_FiltMS*SLOWLOOP_FRE/1000.0f)
//	

//#define VBus_Run_UVP2_V                                         (8.00f)/*unit:v*/
//#define VBus_Run_UVP2_ADC     		       	                    (u16)(VBus_Run_UVP2_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
//#define VBus_Run_UVP2_FiltMS                                    (10)/*unit:ms*/
//#define VBus_Run_UVP2_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP2_FiltMS*SLOWLOOP_FRE/1000.0f)

//    
//#define VBus_OVP_V                                              (24.00f)/*unit:v*/
//#define VBus_OVP_ADC     		       	                             (u16)(VBus_OVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
//#define VBus_OVP_FiltMS                                         (300)/*unit:ms*/
//#define VBus_OVP_FiltMS_COUNT                                   (u16)((float)VBus_OVP_FiltMS*SLOWLOOP_FRE/1000.0f)
//	
//#define SOC3_MV                     	                        (u32)((18.2)*100)
//#define SOC2_MV                     	                        (u32)((17.2)*100)
//#define SOC1_MV                     	                        (u32)((16.2)*100)

	
#define MOS_NTC_10K_3435K 1
#define MOS_NTC_100K_3950K 0
#if 	MOS_NTC_10K_3435K

#define RT_60_F 2.965f   //3435K-10k
#define RT_65_F 2.537f
#define RT_70_F 2.180f
#define RT_75_F 1.880f
#define RT_80_F 1.628f
#define RT_85_F 1.414f
#define RT_90_F 1.233f
#define RT_95_F 1.079f
#define RT_100_F 0.947f
#define RT_105_F 0.833f
#define RT_110_F 0.736f
#define RT_115_F 0.652f

#elif MOS_NTC_100K_3950K

#define RT_60_F 24.701f  //3950-100k
#define RT_65_F 20.641f
#define RT_70_F 17.325f
#define RT_75_F 14.605f
#define RT_80_F 12.364f
#define RT_85_F 10.509f
#define RT_90_F 8.967f
#define RT_95_F 7.680f
#define RT_100_F 6.601f
#define RT_105_F 5.694f
#define RT_110_F 4.929f
#define RT_115_F 4.280f
#define RT_120_F 3.728f
#define RT_124_F 3.346f
#define RT_125_F 3.257f
#endif 



/* ---------------------------- MOS Temperature Protect Parameter -------------------------- */
#define EN_MOS_TEMP_DETECT     	       	(1)    		/* MOS温度保护功能使能 */
#define MOS_TEMP_UP_VOL                	5     		/* MOS温度检测上拉电压，单位：V */
#define MOS_TEMP_UP_RES        	       	10.0f    		/* MOS温度检测上拉电阻，单位：K欧 */
#define MOS_TEMP_OVER_TIME     					50  			/* 单位：ms */
#define RSM_MOS_TEMP_OVER_TIME 					500  			/* 单位：ms */
/* 将用户设定值转换为ADC采样值 */
#define MOS_TEMP_OVER_RES_RATIO					(float)(RT_95_F/(RT_95_F +MOS_TEMP_UP_RES)) 								// 分压比(下拉电阻/(上拉电阻+下拉电阻)) 需求表: 95±5℃
#define RSM_MOS_TEMP_OVER_RES_RATIO			(float)(RT_85_F/(RT_85_F + MOS_TEMP_UP_RES)) 				// 分压比(下拉电阻/(上拉电阻+下拉电阻)) 需求表: 恢复 85±5℃
#define MOS_TEMP_OVER_THD_ADC       		(u16)(MOS_TEMP_OVER_RES_RATIO * MOS_TEMP_UP_VOL * ADC_REF_VALUE / ADC_REFV)			// 稳态过温保护阈值
#define RSM_MOS_TEMP_OVER_THD_ADC   		(u16)(RSM_MOS_TEMP_OVER_RES_RATIO * MOS_TEMP_UP_VOL * ADC_REF_VALUE / ADC_REFV)	// 恢复过温保护阈值

/* ---------------------------- BAT Temperature Protect Parameter -------------------------- */
#define EN_BAT_TEMP_DETECT               (1)             /* BAT温度保护功能使能 */








#define Tool_Dir_CW_Value                                       (0)/*顺时针方向值*/
#define Tool_Dir_CCW_Value                                      (1)/*逆时针方向值*/
#define Tool_Dir_Init                                         (0 )/*逆时针方向值*/





//#define MOS_SHORT_CURRENT               (8.0f)
//#define MOS_SHORT_CURRENT_AdcValue    (u16)(MOS_SHORT_CURRENT*CURRENT_ADC_PER_A)



/*反转停机参数*/
#define CCW_AUTO_STOP_CURRENT      	   	                        (18.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT*CURRENT_ADC_PER_A)

#define CCW_AUTO_STOP_CURRENT_2      	   	                        (10.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_2_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT_2*CURRENT_ADC_PER_A)


#define CCW_AUTO_STOP_CURRENT_3      	   	                        (24.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_3_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT_3*CURRENT_ADC_PER_A)
#define CCW_AUTO_STOP_START_MASK_COUNT                      (150)  //启动阶段滤波，启动过程过滤
//#define TIME_AUTO_STOP                 	                        (500ul)     /*unit 1ms*/
//#define TIME_AUTO_STOP_REDUCE          	                        (1ul)     /*unit 1ms*/
#define AUTO_STOP_Sector_CNT       	                        (60ul)     


#endif
