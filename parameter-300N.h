#ifndef __PARAMETER_H_
#define __PARAMETER_H_

#include "HAL_device.h"


//----------------------------------------------------------------------//
/* System setting */
//----------------------------------------------------------------------//
/* 系统时钟频率60MHz */
#define SYSCLK_HSI			       	    ((u32)60000000)   									// 系统时钟频率
/* PWM频率16kHz */
#define PWMFREQ                	   	((u16)16000)    										// PWM频率
/* PWM周期装载值 = SYSCLK_HSI/PWMFREQ/2-1 */
#define PWM_PRIOD_LOAD 	       			(u16)(SYSCLK_HSI/PWMFREQ/2-1)  			// PWM周期装载值

/* 死区时间：1us */
#define DEADTIME 					       		(1)      														// 死区时间，单位us
/* 死区时间装载值 */
#define DEADTIME_LOAD 		       		((u16)(DEADTIME*SYSCLK_HSI/1000000))	 	// 死区时间装载值

/* TIM1慢环定时器装载值 */
#define SLOWLOOP_500us_CNT_LOAD     ((u16)(PWMFREQ/1000/2))							// TIM1定时500us
#define SLOWLOOP_1ms_CNT_LOAD      	((u16)(PWMFREQ/1000))								// TIM1定时1ms
#define SLOWLOOP_400ms_CNT_LOAD    	((u16)400*SLOWLOOP_1ms_CNT_LOAD)		// TIM1定时400ms

#define TIM3_TO_PWM_PERIOD 	       	(float)(1000000/PWMFREQ)

/* 慢环频率：1ms为1个单位 */
#define SLOWLOOP_FRE                (1000) // 循环频率1ms为1ms

/* 休眠时间 */
#define  SLEEP_TIME                 				10000ul//10000
//---------------------------------------------------------------------//
// PWM
//---------------------------------------------------------------------//

/*--------------------------------------
 螺丝刀模式（注释掉的配置）
#define Wrench_EN                   (0)
#define RESTAR_EN                  (0)
#define USER_ROTATE_ERROR_EN      (1)  // 反转检测使能---螺丝刀模式
#define Block_Protect_RUNING_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD) 堵转时间unit:ms

 电钻模式
#define Wrench_EN                   (1)
#define RESTAR_EN                  (1)
#define RESTAR_COUNT               (20)
#define RESTAR_MIN_PWM_AIM_DUTY    ((u16)(0.5f*PWM_PRIOD_LOAD))
#define RESTAR_MIN_PWM_LIMIT_DUTY    ((u16)(0.25f*PWM_PRIOD_LOAD))

#define Wrench_CW_MAX_DUTY             ((u16)(PWMFREQ*0.96f))
#define USER_ROTATE_ERROR_EN      (0)  // 反转检测使能---电钻模式

#define Block_Protect_RUNING_TIME                     (20*SLOWLOOP_1ms_CNT_LOAD) 堵转时间unit:ms
----------------------------------------*/

/* 电钻模式使能 */
#define Wrench_EN                   (1)
/* 重启功能使能 */
#define RESTAR_EN                  (1)
/* 重启次数限制 */
#define RESTAR_COUNT               (25)
/* 重启最小目标占空比 */
#define RESTAR_MIN_PWM_AIM_DUTY    ((u16)(0.5f*PWM_PRIOD_LOAD))
/* 重启最小限制占空比 */
#define RESTAR_MIN_PWM_LIMIT_DUTY    ((u16)(0.25f*PWM_PRIOD_LOAD))

/* 正转最大占空比：96% */
#define Wrench_CW_MAX_DUTY             ((u16)(PWM_PRIOD_LOAD*0.96f))

/* 反转检测使能（电钻模式关闭） */
#define USER_ROTATE_ERROR_EN      (0)  // 反转检测使能---电钻模式

/* 限速功能使能 */
#define LIMIT_SPEED_EN                 (0)  // 限速

/* LED扫描使能 */
#define LED_SCAN_EN       (1)

/* PWM占空比档位 */
#define PWM_DUTY_GEARS1    (u32)(32768*0.6f)   // 档位1：60%占空比
#define PWM_DUTY_GEARS2    (u32)(32768*0.80f)  // 档位2：80%占空比
#define PWM_DUTY_GEARS3    (u32)(32768*1.0f)    // 档位3：100%占空比
#define PWM_DUTY_GEARS4    (u32)(32768*1.0f)    // 档位4：100%占空比
//---------------------------------------------------------------------------------------------------------------------------------------------------//
//************************************************************************************************************************************************/


// VSP 占空比控制参数
/* 最大工具占空比：满量程 */
#define MAX_TOOL_DUTY 		                	(PWM_PRIOD_LOAD)// 高量程满占空比
/* 最小工具占空比：15% */
#define MIN_TOOL_DUTY 		   		 						(PWM_PRIOD_LOAD*0.15f) // 低量程最小占空比

/* 扳机触发ADC阈值 */
#define TRG_ON_ADC               						300// 扳机触发判断ADC
/* 扳机最大ADC值 */
#define MAX_TRG_ADC 		   		   						4000// 扳机ADC最大值
/* 扳机最小ADC值 */
#define MIN_TRG_ADC              						600// 扳机ADC最小值
/* ADC到占空比的比例系数 */
#define K_ADC_DUTY			   									(float)(MAX_TOOL_DUTY-MIN_TOOL_DUTY) / (float)(MAX_TRG_ADC - MIN_TRG_ADC)

// PWM占空比限制参数（VSP控制的PWM占空比范围）
/* 最小PWM占空比：15% */
#define LIMIT_MIN_PWN_DUTY   ((u16)(PWM_PRIOD_LOAD*0.15f))
/* 最大PWM占空比：满量程 */
#define LIMIT_MAX_PWN_DUTY   (PWM_PRIOD_LOAD)


//---------------------------------------------------------------------//
// ADC
//---------------------------------------------------------------------//
/* ADC参考电压：5V */
#define ADC_REFV                 		5													// unit:V, ADC参考电压
/* 12位ADC满量程值 */
#define ADC_REF_VALUE          	   	4095											// 12-bit ADC

/* ------------------------------ ADC校准相关参数 ---------------------------- */
/* 是否使能判断偏置异常 */
#define EN_OFFSET_CALIB       			(1)      									/* 是否使能判断偏置异常 */
/* 校准采样次数：2^10 = 1024 */
#define CALIB_SAMPLES_K            		(10)   										// 2^10 = 1024, 校准采样次数设定
#define CALIB_SAMPLES              	(1024)										// 校准采样次数设定
#define CALIB_SAMPLES_DOUBLE       	(CALIB_SAMPLES << 1)
/* 电流偏置电压：0V */
#define CURRENT_OFFSET_VOLTAGE     	(0.0)    									/* 电流偏置电压，根据实际电路设定, unit:V */
/* 偏置电压转换为ADC值 */
#define CURRENT_OFFSET_VALUE				(u16)(ADC_REF_VALUE * CURRENT_OFFSET_VOLTAGE / ADC_REFV)
/* 偏置上限阈值：偏置值+500 */
#define CURRENT_OFFSET_H_THD				(u16)(CURRENT_OFFSET_VALUE + 500)
/* 偏置下限阈值：偏置值-500 */
#define CURRENT_OFFSET_L_THD				(u16)(CURRENT_OFFSET_VALUE - 500)

/* ---------------------------- Current Protect  ---------------------------------- */

/* ------------------------------ 电流采样硬件参数 ---------------------------- */
/* 采样电阻：1mΩ */
#define RSHUNT 												 1.0f							/* 采样电阻，单位:mR */
/* 运放放大倍数：10.89 */
#define AMPLIFICATION_GAIN 						 (float)(10.89)		/* 运放放大倍数 */

/* 每安培对应的ADC值 = RSHUNT * AMPLIFICATION_GAIN * ADC_REF_VALUE / ADC_REFV / 1000 */
#define CURRENT_ADC_PER_A              (float)(RSHUNT * AMPLIFICATION_GAIN * ADC_REF_VALUE / ADC_REFV / 1000.0f)  			/* 每安培ADC值 */

/* MOS短路电流阈值：100A */
#define MOS_SHORT_CURRENT               (100.0f)
#define MOS_SHORT_CURRENT_AdcValue    (u16)(MOS_SHORT_CURRENT*CURRENT_ADC_PER_A)

/* 平均电流记录时间：200ms */
#define IBus_Avg_REC_MS               (200) //ms
#define IBus_Avg_REC_MS_CONUT         (u16)((float)IBus_Avg_REC_MS*SLOWLOOP_FRE/1000.0f)

/* 平均过流保护1：38A，持续1500ms触发 */
#define IBus_Avg_OCP1                (38.0f)
#define IBus_Avg_OCP1_AdcValue         (u16)(IBus_Avg_OCP1*CURRENT_ADC_PER_A)
#define IBus_Avg_OCP1_MS               (1500) //ms
#define IBus_Avg_OCP1_MS_CONUT         (u16)((float)IBus_Avg_OCP1_MS*SLOWLOOP_FRE/1000.0f)

/* 平均过流保护2：45A，持续200ms触发 */
#define IBus_Avg_OCP2                 (45.0f)
#define IBus_Avg_OCP2_AdcValue         (u16)(IBus_Avg_OCP2*CURRENT_ADC_PER_A)
#define IBus_Avg_OCP2_MS               (200) //ms
#define IBus_Avg_OCP2_MS_CONUT        (u16)((float)IBus_Avg_OCP2_MS*SLOWLOOP_FRE/1000.0f)

/* 平均过流保护3：55A，持续50ms触发 */
#define IBus_Avg_OCP3                  (55.0f)
#define IBus_Avg_OCP3_AdcValue         (u16)(IBus_Avg_OCP3*CURRENT_ADC_PER_A)
#define IBus_Avg_OCP3_MS               (50) //ms
#define IBus_Avg_OCP3_MS_CONUT        (u16)((float)IBus_Avg_OCP3_MS*SLOWLOOP_FRE/1000.0f)

/* 平均过流保护4：65A，持续5ms触发（最严重） */
#define IBus_Avg_OCP4                  (65.0f)
#define IBus_Avg_OCP4_AdcValue         (u16)(IBus_Avg_OCP4*CURRENT_ADC_PER_A)
#define IBus_Avg_OCP4_MS               (5) //ms
#define IBus_Avg_OCP4_MS_CONUT        (u16)((float)IBus_Avg_OCP4_MS*SLOWLOOP_FRE/1000.0f)


/* 堵转保护使能 */
#define Block_Protect_En                                        (1)/*堵转保护使能*/

/* 基于电流等级的堵转保护使能 */
#define Block_Protect_Current_En                                 (1)/*根据电流等级判断堵转时间*/
/* 堵转恢复时间：100ms */
#define Block_Protect_Recover_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)/*堵转恢复时间unit:ms*/

#if Block_Protect_Current_En

/* 最大堵转时间：100ms */
#define Block_Protect_MAX_TIME                     (100*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/
/* 运行中堵转时间：20ms */
#define Block_Protect_RUNING_TIME                     (20*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

/* 堵转电流等级1：45A，允许堵转80ms */
#define Block_Protect_Current1                      (45.00)/*unit:a*/
#define Block_Protect_Current1_AdcValue             (u16)(Block_Protect_Current1*CURRENT_ADC_PER_A)
#define Block_Protect_Current1_TIME                 (80*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

/* 堵转电流等级2：50A，允许堵转50ms */
#define Block_Protect_Current2                      (50.00)/*unit:a*/
#define Block_Protect_Current2_AdcValue             (u16)(Block_Protect_Current2*CURRENT_ADC_PER_A)
#define Block_Protect_Current2_TIME                 (50*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

/* 堵转电流等级3：55A，允许堵转30ms */
#define Block_Protect_Current3                      (55.00)/*unit:a*/
#define Block_Protect_Current3_AdcValue             (u16)(Block_Protect_Current3*CURRENT_ADC_PER_A)
#define Block_Protect_Current3_TIME                 (30*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

/* 堵转电流等级4：60A，允许堵转10ms（最严重） */
#define Block_Protect_Current4                      (60.00)/*unit:a*/
#define Block_Protect_Current4_AdcValue             (u16)(Block_Protect_Current4*CURRENT_ADC_PER_A)
#define Block_Protect_Current4_TIME                 (10*SLOWLOOP_1ms_CNT_LOAD)/*堵转时间unit:ms*/

#else
/* 基于PWM占空比的堵转保护 */
#define Block_Protect_LowDuty_DATA									            (u16)(0.70f*PWM_PRIOD_LOAD)/*堵转低占空比阈值*/
#define Block_Protect_LowDuty_COUNT                            (80*SLOWLOOP_1ms_CNT_LOAD)/*低占空比堵转时间unit:ms*/
#define Block_Protect_HighDuty_COUNT                           (30*SLOWLOOP_1ms_CNT_LOAD)/*高占空比堵转时间unit:ms*/

#endif


/* 多级积分峰值过流保护使能 */
#define Multistage_Integral_Peak_OCP_EN              (1) // 多级积分峰值过流保护使能标志位


#if Multistage_Integral_Peak_OCP_EN

/* 智能峰值过流保护1：50A，持续300ms触发 */
	#define IBus_Smart_Peak_OCP1                      (50.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP1_AdcValue             (u16)(IBus_Smart_Peak_OCP1*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP1_MS                   (300) //ms
	#define IBus_Smart_OCP1_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP1_MS*SLOWLOOP_FRE/1000.0f))

/* 智能峰值过流保护2：60A，持续100ms触发 */
	#define IBus_Smart_Peak_OCP2                      (60.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP2_AdcValue             (u16)(IBus_Smart_Peak_OCP2*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP2_MS                   (100) //ms
	#define IBus_Smart_OCP2_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP2_MS*SLOWLOOP_FRE/1000.0f))

/* 智能峰值过流保护3：70A，持续10ms触发 */
	#define IBus_Smart_Peak_OCP3                      (70.00)/*unit:a*/
	#define IBus_Smart_Peak_OCP3_AdcValue             (u16)(IBus_Smart_Peak_OCP3*CURRENT_ADC_PER_A)
	#define IBus_Smart_Peak_OCP3_MS                   (10) //ms
	#define IBus_Smart_OCP3_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f))

/* 智能峰值恢复时间：120ms */
	#define IBus_Smart_REC_MS                         120
	#define IBus_Smart_REC_MS_Operation_Value        (u16)(32768.0f/((float)IBus_Smart_REC_MS*SLOWLOOP_FRE/1000.0f))
#else

/* 峰值过流保护1：50A，持续800ms触发 */
	#define IBus_Peak_OCP1                  (50.00)/*unit:a*/
	#define IBus_Peak_OCP1_AdcValue         (u16)(IBus_Peak_OCP1*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP1_MS               (800) //ms
	#define IBus_Peak_OCP1_MS_CONUT        (u16)((float)IBus_Peak_OCP1_MS*SLOWLOOP_FRE/1000.0f)

/* 峰值过流保护2：60A，持续90ms触发 */
	#define IBus_Peak_OCP2                  (60.00)/*unit:a*/
	#define IBus_Peak_OCP2_AdcValue         (u16)(IBus_Peak_OCP2*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP2_MS               (90) //ms
	#define IBus_Peak_OCP2_MS_CONUT        (u16)((float)IBus_Peak_OCP2_MS*SLOWLOOP_FRE/1000.0f)

/* 峰值过流保护3：70A，持续10ms触发 */
	#define IBus_Peak_OCP3                  (70.00)/*unit:a*/
	#define IBus_Peak_OCP3_AdcValue         (u16)(IBus_Peak_OCP3*CURRENT_ADC_PER_A)
	#define IBus_Peak_OCP3_MS               (10) //ms
	#define IBus_Peak_OCP3_MS_CONUT        (u16)((float)IBus_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f)

/* 峰值恢复时间：150ms */
  #define IBus_Peak_REC_MS               (150) //ms
	#define IBus_Peak_REC_MS_CONUT        (u16)((float)IBus_Peak_OCP3_MS*SLOWLOOP_FRE/1000.0f)
#endif

/* 峰值瞬时过流保护：100A，逐周期检测触发 */
#define IBus_Peak_IT_OCP               (100.0f)  // 峰值瞬时过流检测触发
#define IBus_Peak_IT_OCP_AdcValue      (u16)(IBus_Peak_IT_OCP*CURRENT_ADC_PER_A)
#define IBus_Peak_IT_COUNT     				 (u16)(PWMFREQ/1000)// PWMFREQ/1000=1MS

/* 峰值单次过流保护使能 */
#define IBus_Peak_OCP_ONCE_EN              (1)
/* 峰值单次过流保护：100A */
#define IBus_Peak_OCP_ONCE               (100.0f)  // 峰值单次过流检测触发
#define IBus_Peak_OCP_ONCE_AdcValue      (u16)(IBus_Peak_OCP_ONCE*CURRENT_ADC_PER_A)
/* IPD过流保护：100A */
#define IBus_IPD_OCP               (100.0f)  // IPD过流检测触发
#define IBus_IPD_OCP_AdcValue      (u16)(IBus_IPD_OCP*CURRENT_ADC_PER_A)


/* 峰值电流限流使能 */
#define LIMIT_PEAK_CURRENT_EN               (0)
/* 峰值电流限流值：110A */
#define LIMIT_PEAK_CURRENT                  (110.0f)
#define LIMIT_PEAK_CURRENT_AdcValue         (u16)(LIMIT_PEAK_CURRENT*CURRENT_ADC_PER_A)


/* 平均电流限流使能 */
#define LIMIT_AVG_CURRENT_EN               (1)
/* 平均电流限流值（多档） */
#define LIMIT_AVG_CURRENT4                  (26.0f)
#define LIMIT_AVG_CURRENT4_AdcValue         (u16)(LIMIT_AVG_CURRENT4*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT3                 (26.0f)
#define LIMIT_AVG_CURRENT3_AdcValue         (u16)(LIMIT_AVG_CURRENT3*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT2                  (20.0f)
#define LIMIT_AVG_CURRENT2_AdcValue         (u16)(LIMIT_AVG_CURRENT2*CURRENT_ADC_PER_A)

#define LIMIT_AVG_CURRENT1                  (15.0f)
#define LIMIT_AVG_CURRENT1_AdcValue         (u16)(LIMIT_AVG_CURRENT1*CURRENT_ADC_PER_A)

/* 反转平均电流限流值 */
#define LIMIT_AVG_CURRENT_CCW                  (27.0f)
#define LIMIT_AVG_CURRENT_AdcValue_CCW         (u16)(LIMIT_AVG_CURRENT_CCW*CURRENT_ADC_PER_A)



/* ---------------------------- Voltage Protect Parameter -------------------------- */
/* 母线电压保护使能 */
#define EN_VOLATAGE_PROTECT    	       	(1)    														/* 母线电压保护使能 */
/* 母线电压分压比：(1+10)/1 = 11 */
#define VBUS_SHUNT_RATIO           			(float)((1.0 + 10.0)/1.0)			// 母线电压分压比 (下臂电阻/(上臂电阻+下臂电阻))

#define VBUS_BASE_X100       ((u32)(VBUS_SHUNT_RATIO*ADC_REFV*100))

/* 稳态欠压保护：14.8V，滤波100ms */
#define VBus_Stb_UVP_V                                          (14.80f)/*unit:v*/
#define VBus_Stb_UVP_ADC     		       	                        (u16)(VBus_Stb_UVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)  		/* 欠压保护ADC阈值*/
#define VBus_Stb_UVP_FiltMS                                     (100)/*unit:ms*/
#define VBus_Stb_UVP_FiltMS_COUNT                               (u16)((float)VBus_Stb_UVP_FiltMS*SLOWLOOP_FRE/1000.0f)

/* 运行欠压保护1：13.2V，滤波300ms */
#define VBus_Run_UVP1_V                                         (13.2f)/*unit:v*/
#define VBus_Run_UVP1_ADC     		       	                      (u16)(VBus_Run_UVP1_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_Run_UVP1_FiltMS                                    (300)/*unit:ms*/
#define VBus_Run_UVP1_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP1_FiltMS*SLOWLOOP_FRE/1000.0f)

/* 运行欠压保护2：8.0V，滤波10ms（严重欠压） */
#define VBus_Run_UVP2_V                                         (8.00f)/*unit:v*/
#define VBus_Run_UVP2_ADC     		       	                      (u16)(VBus_Run_UVP2_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_Run_UVP2_FiltMS                                    (10)/*unit:ms*/
#define VBus_Run_UVP2_FiltMS_COUNT                              (u16)((float)VBus_Run_UVP2_FiltMS*SLOWLOOP_FRE/1000.0f)

/* 过压保护：24.8V，滤波300ms */
#define VBus_OVP_V                                              (24.80f)/*unit:v*/
#define VBus_OVP_ADC     		       	                             (u16)(VBus_OVP_V * ADC_REF_VALUE / VBUS_SHUNT_RATIO / ADC_REFV)
#define VBus_OVP_FiltMS                                         (300)/*unit:ms*/
#define VBus_OVP_FiltMS_COUNT                                   (u16)((float)VBus_OVP_FiltMS*SLOWLOOP_FRE/1000.0f)

/* 电池SOC电压阈值（单位：100mV） */
#define SOC3_MV                     	                        (u32)((18.2)*100)  // SOC 3档：18.2V
#define SOC2_MV                     	                        (u32)((17.2)*100)  // SOC 2档：17.2V
#define SOC1_MV                     	                        (u32)((16.2)*100)  // SOC 1档：16.2V


/* MOS NTC类型选择 */
#define MOS_NTC_10K_3535K 0
#define MOS_NTC_100K_3950K 1
#if 	MOS_NTC_10K_3535K

/* 10K NTC（3435K B值）各温度点ADC值 */
#define RT_60_F 2.965f   // 3435K-10k
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

/* 100K NTC（3950K B值）各温度点ADC值 */
#define RT_60_F 24.701f  // 3950-100k
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
/* MOS温度保护使能 */
#define EN_MOS_TEMP_DETECT     	       	(1)    		/* MOS温度保护使能 */
/* MOS温度检测上拉电压：5V */
#define MOS_TEMP_UP_VOL                	5     		/* MOS温度检测上拉电压，单位:V */
/* MOS温度检测上拉电阻：4.7KΩ */
#define MOS_TEMP_UP_RES        	       	4.7f    		/* MOS温度检测上拉电阻，单位:KΩ */
/* MOS过温时间：50ms */
#define MOS_TEMP_OVER_TIME     					50  			/* 单位:ms */
/* MOS过温恢复时间：500ms */
#define RSM_MOS_TEMP_OVER_TIME 					500  			/* 单位:ms */
/* 转换为ADC阈值 */
#define MOS_TEMP_OVER_RES_RATIO					(float)(RT_105_F/(RT_105_F +MOS_TEMP_UP_RES)) 								// 分压比 (NTC电阻/(NTC电阻+上拉电阻))
#define RSM_MOS_TEMP_OVER_RES_RATIO			(float)(RT_95_F/(RT_95_F + MOS_TEMP_UP_RES)) 				// 分压比 (NTC电阻/(NTC电阻+上拉电阻))
#define MOS_TEMP_OVER_THD_ADC       		(u16)(MOS_TEMP_OVER_RES_RATIO * MOS_TEMP_UP_VOL * ADC_REF_VALUE / ADC_REFV)			// 过温保护ADC阈值
#define RSM_MOS_TEMP_OVER_THD_ADC   		(u16)(RSM_MOS_TEMP_OVER_RES_RATIO * MOS_TEMP_UP_VOL * ADC_REF_VALUE / ADC_REFV)	// 过温恢复ADC阈值




/* 转向定义 */
#define Tool_Dir_CW_Value                                       (1)/*顺时针方向值*/
#define Tool_Dir_CCW_Value                                      (0)/*逆时针方向值*/
#define Tool_Dir_Init                                         (0 )/*逆时针方向值*/


/* 反转自动停止电流阈值 */
#define CCW_AUTO_STOP_CURRENT      	   	                        (12.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT*CURRENT_ADC_PER_A)

#define CCW_AUTO_STOP_CURRENT_2      	   	                        (10.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_2_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT_2*CURRENT_ADC_PER_A)

#define CCW_AUTO_STOP_CURRENT_3      	   	                        (14.0f)     /*unit A*/
#define CCW_AUTO_STOP_CURRENT_3_AdcValue    (u16)(CCW_AUTO_STOP_CURRENT_3*CURRENT_ADC_PER_A)
/* 反转自动停止起始屏蔽计数 */
#define CCW_AUTO_STOP_START_MASK_COUNT                      (150)  // 启动时间，屏蔽检测
/* 反转自动停止扇区计数 */
#define AUTO_STOP_Sector_CNT       	                        (60ul)


#endif
