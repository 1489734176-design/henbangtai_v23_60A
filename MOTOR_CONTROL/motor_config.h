#ifndef __MOTOR_CONFIG_H
#define __MOTOR_CONFIG_H
#include "parameter.h"

/* PWM频率（kHz） */
#define PWMFREQ_KHZ  (u16)(PWMFREQ/1000)

/* 电机定时器频率（7.5MHz） */
#define MOTOR_TIMR_FRQ_HZ 7500000
#define MOTOR_TIMR_FRQ_MHZ 7.5f


/*****************************************************************************/

/****************************************************************************
*
* BEMF过零检测时间参数（掩码时间/移位时间）
* 掩码时间：BEMF过零检测的屏蔽窗口，防止换相噪声误触发
* 移位时间：BEMF过零检测的相位补偿角对应的时间
*
*****************************************************************************/
/* 最大掩码时间：1500us，防止过长的屏蔽窗口导致换相延迟 */
#define MaskTime_Max_US                                         (1500)/*最大屏蔽时间，单位:us*/
#define MaskTime_Max_TIMER_COUNT                                (u32)(MaskTime_Max_US*MOTOR_TIMR_FRQ_MHZ)

/* 最小掩码时间：20us，防止过短的屏蔽窗口导致噪声误触发 */
#define MaskTime_Min_US                                         (20)/*最小屏蔽时间，单位:us*/
#define MaskTime_Min_TIMER_COUNT 																(u32)(MaskTime_Min_US*MOTOR_TIMR_FRQ_MHZ)

/* 最大移位时间：1500us */
#define ShiftTime_Max_US                                        (1500)/*最大移位时间，单位:us*/
#define ShiftTime_Max_TIMER_COUNT                               (u32)(ShiftTime_Max_US*MOTOR_TIMR_FRQ_MHZ)

/* 最小移位时间：10us */
#define ShiftTime_Min_US                                        (10)/*最小移位时间，单位:us*/
#define ShiftTime_Min_TIMER_COUNT                           		(u32)(ShiftTime_Min_US*MOTOR_TIMR_FRQ_MHZ)

/* 最小扇区时间：50us */
#define SectorTIme_Min_US                                       (50)/*最小扇区时间，单位:us*/
#define SectorTIme_Min_TIMER_COUNT                              (u32)(SectorTIme_Min_US*MOTOR_TIMR_FRQ_MHZ)

/* 最大扇区时间：500ms，超过此时间判定为堵转（不超过1.5s） */
#define SectorTIme_Max_MS                                       (500)/*最大扇区时间，单位:ms,不可超过1.5s*/
#define SectorTIme_Max_COUNT                                		(u32)(SectorTIme_Max_MS*PWMFREQ_KHZ) // PWM周期计数
#define SectorTIme_Max_TIMER_COUNT                           		(u32)(SectorTIme_Max_MS*MOTOR_TIMR_FRQ_MHZ*1000)
/****************************************************************************/

/****************************************************************************
*
* 开环拖动时间参数
*
*****************************************************************************/
/* 开环拖动阶段的掩码时间：800us */
#define MaskTime_OpenLoop_US                                    (800)/*开环屏蔽时间，单位:us*/
#define MaskTime_OpenLoop_TIMER_COUNT														(u32)(MaskTime_OpenLoop_US*MOTOR_TIMR_FRQ_MHZ)

/* 开环拖动阶段的移位时间：20us */
#define ShiftTime_OpenLoop_US                                   (20)/*开环移位时间，单位:us*/
#define ShiftTime_OpenLoop_TIMER_COUNT	                         (u32)(ShiftTime_OpenLoop_US*MOTOR_TIMR_FRQ_MHZ)

/* 低速闭环BEMF换相角参数 */
#define LOW_Speed_MaskTime_ClosedLoop_Angle                               (40)/*闭环屏蔽角:1-50*/
#define LOW_Speed_ShiftTime_ClosedLoop_Angle                              (5)/*闭环移位角:1-50*/

/* 闭环BEMF换相角参数 */
#define MaskTime_ClosedLoop_Angle                               (20)/*闭环屏蔽角:1-50*/
#define ShiftTime_ClosedLoop_Angle                              (5)/*闭环移位角:1-50*/

/* 高速闭环BEMF换相角参数 */
#define High_Speed_MaskTime_ClosedLoop_Angle                               (20)/*闭环屏蔽角:1-50*/
#define High_Speed_ShiftTime_ClosedLoop_Angle                              (5)/*闭环移位角:1-50*/

/****************************************************************************
* 失步保护
*
*****************************************************************************/
#define LosStep_Protect_En                                      (1)/*失步保护使能*/
#define LosStep_Protect_CntSet                                  (30)/*失步保护滤波次数*/
#define MOTOR_ROTATE_ERROR_DETECT_En                           (USER_ROTATE_ERROR_EN)  // 电机反转检测使能
/****************************************************************************/

/****************************************************************************
*
* 启动参数（IPD初始位置检测）
*
*****************************************************************************/
/* IPD脉冲时间：60us */
#define MotorStart_IPD_PulseTime                                (60)/*IPD脉冲时间，单位:us*/
/* IPD空闲时间：2000us */
#define MotorStart_IPD_IdleTime                                 (2000)/*IPD空闲时间，单位:us*/
/****************************************************************************/

/* 强制换相配置 */
#define ForceChangePhase_En                               		(0)/*强制换相使能*/
#define ForceChangePhase_CntSet                            		(300)/*强制换相时间*/
#define ForceChangePhaseProtect_TimesSet                            		(30)/*强制换相保护次数，65535为无限次*/

/****************************************************************************
*
* 运动检测（转动判断）
*
*****************************************************************************/
#define Motion_En                                               (1)/*运动检测使能*/
#define Motion_Pass_CntSet                                      (6)/*运动判断滤波次数*/

/* 运动检测最大扇区时间：10000us */
#define Motion_SectorTime_Lim_US                                (10000)/*运动检测最大扇区时间，单位:us*/

/* 运动检测检查时间 */
#define Motion_CheckTime_Count                                 (40*SLOWLOOP_1ms_CNT_LOAD)/*运动检测检查时间，单位:ms*/

/* 运动检测失败制动时间 */
#define Motion_Fail_BrakeTime_Count                                 (30*SLOWLOOP_1ms_CNT_LOAD)/*运动检测失败制动时间，单位:ms*/

/* 运动检测ADC阈值：BEMF超过此值判定为有转动 */
#define  Motion_ADC_THRESHOLD                                       (130) //运动检测ADC阈值

/****************************************************************************
*
* 制动参数
*
*****************************************************************************/
#define BREAKE_WAIT_EN                           (0)/*制动等待使能*/

/* 软制动参数 */
#define SBrake_En                                               (0)/*软制动使能*/
#define Sbrake_Duty_Add                                     (1)/*软制动占空比步进增量，0-32768*/
#define Sbrake_Duty_Max                                     (u16)(PWM_PRIOD_LOAD*0.7f)/*软制动最大占空比，0-32768*/
#define Sbrake_Duty_Min                                     (u16)(PWM_PRIOD_LOAD*0.1f)/*软制动最小占空比，0-32768*/
#define Sbrake_Time_MS                                          (200*SLOWLOOP_1ms_CNT_LOAD)/*软制动时间，单位:ms*/
#define SBrake_MIN_Time_MS_CNT                                   (100*SLOWLOOP_1ms_CNT_LOAD)/*软制动最短时间，单位:ms*/

/* 硬制动参数 */
#define HBrake_En                                               (1)/*硬制动使能*/
#define HBrake_MIN_Time_MS_CNT                                      (260*SLOWLOOP_1ms_CNT_LOAD)/*硬制动最短时间，单位:ms*/
#define HBrake_Time_MS_CNT                                          (800*SLOWLOOP_1ms_CNT_LOAD)/*硬制动时间，单位:ms*/
#define WAIT_Brake_Time_MS_CNT                                      (100*SLOWLOOP_1ms_CNT_LOAD)/*制动等待时间，单位:ms*/

/****************************************************************************
*
* PWM占空比加减控制参数
* 用于软启动斜坡控制：占空比逐步增减，防止电流冲击
*
*****************************************************************************/
/* 慢速PWM占空比增减参数（虚拟占空比斜坡） */
#define PWM_ADD_GAP                2  // 占空比增加间隔（中断次数）
#define PWM_ADD_VALUE              2  // 占空比增加步进值

#define PWM_SUB_GAP                2  // 占空比减少间隔（中断次数）
#define PWM_SUB_VALUE              2  // 占空比减少步进值

/* 快速PWM占空比增减参数（实际占空比跟踪） */
#define PWM_ADD_FAST_GAP                1  // 快速增加间隔
#define PWM_ADD_FAST_VALUE              2  // 快速增加步进值

#define PWM_SUB_FAST_GAP                1  // 快速减少间隔
#define PWM_SUB_FAST_VALUE              2  // 快速减少步进值
#endif
