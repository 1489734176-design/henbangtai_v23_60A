#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H


#include "stdint.h"
#include "mm32spin0230.h"


/* 电机状态机枚举 */
typedef enum
{
    MOTOR_STOP = 0,           // 停止状态
    MOTOR_MOTION = 1,		      // 运动检测状态（检测电机是否已在转动）
    MOTOR_POSITION = 2,	      // 位置检测状态（IPD初始位置检测）
    MOTOR_RUN = 3, 		       	// 闭环运行状态
    MOTOR_SBRAKE = 4,         // 软制动状态（PWM控制制动电流）
    MOTOR_HBRAKE = 5,         // 硬制动状态（短接电机绕组）
    MOTOR_BRAKE2STOP = 6,     // 制动到停止的过渡状态
    MotorStart_Drag_OPENLOOP = 7,  // 开环拖动状态
	Motor_Align=8,            // 转子对齐状态
    MOTOR_READY = 9,          // 准备状态（制动等待后进入位置检测）
    MOTOR_STOPWAIT = 10,      // 停止等待状态
    MOTOR_MOTION_FAIL = 11,		      // 运动检测失败状态
	MOTOR_BRAKE_WAIT = 12,		      // 制动等待状态
}MC_StateMachine_e;

/* PI调节器结构体（增量式PI） */
typedef struct PidPI
{
		int32_t 	kp2;            // 比例增益2（用于误差缩放）
    int32_t 	kp;             // 比例增益（Q15格式）
    int32_t 	ki;             // 积分增益（Q15格式）
    int32_t 	error;          // 当前误差
    int32_t 	error1;         // 上一次误差
    int32_t 	error2;         // 上上次误差
    int32_t 	refValue;       // 参考值（目标转速）
    int32_t 	output;         // PI输出（Q15格式）
    int32_t 	proportion;     // 比例项中间值
    int64_t 	errorIntegral;  // 误差积分累加器
    int32_t 	lowerLimitOutput;   // 输出下限
    int32_t 	upperLimitOutput;   // 输出上限
    int32_t 	maxIntegralRatio;   // 最大积分比率
    int32_t 	lowerLimitIntegral; // 积分项下限
    int32_t 	upperLimitIntegral; // 积分项上限

    int32_t 	pTemp;          // 比例项临时变量
    int32_t 	iTemp;          // 积分项临时变量

    int32_t 	deltaUk;        // 增量式PI输出增量：deltaUk = Uk - Uk_1
    int32_t 	deltaOut;       // 输出增量
    int32_t 	rpmSetPwmDuty;  // 转速对应的PWM占空比设定值
    int32_t 	KpDuty;         // 比例增益对应的占空比
    int32_t 	KiDuty;         // 积分增益对应的占空比
    int32_t 	OutDuty;        // 占空比输出
	int32_t  Limit_max_data;  // 占空比上限
	int32_t  Limit_min_data;  // 占空比下限
} PidPI_t;

/* 电机控制列表结构体 */
typedef struct
{
	/* 电机状态机 */
	MC_StateMachine_e bldc_state;

	/* 电流采样偏置校准标志 */
	uint8_t u8Calibration_OK_flag;     // 校准完成标志
	uint8_t u8CurrentOffset_error_flag; // 偏置异常标志
	uint16_t CurrentOffset_ad_data;    // 电流偏置ADC值

	/* PWM占空比相关 */
	uint16_t pwm_duty_aim;       // 目标占空比
	uint16_t pwm_duty_aim2;      // 目标占空比2（PI输出）
	uint16_t pwm_duty_aim_per;   // 目标占空比预设值
	uint16_t pwm_duty_virtual;   // 虚拟占空比（软斜坡输出）

	/* 制动使能 */
	uint8_t  hbrake_en;          // 硬制动使能
	uint8_t  sbrake_en;          // 软制动使能
	uint16_t sbreak_pwm_duty;    // 软制动占空比

	/* 运行状态 */
	uint8_t motor_run_last_dir;  // 上次运行方向
	uint32_t motor_run_count;    // 运行计时器
	uint32_t motor_speed;        // 电机转速（RPM）
	uint32_t	time_count_60_degree_filter;  // 60度电周期滤波时间

}Motor_Control_list;

/* 全局电机控制列表 */
extern  Motor_Control_list motor_control_list;

#if LIMIT_SPEED_EN
/* 速度PI调节器 */
extern PidPI_t Speed_Pid;
#endif

/* 电机状态机主函数 */
void MC_Machine_State(void);

/* 电流采样偏置校准 */
void CurrentOffsetCalibration(void);

/* MOS管短路检测 */
void mose_check(void);

/* 电机控制系统初始化 */
void start_mc_init(void);

/* 转子对齐控制 */
void align_control(void);

/* 开环拖动控制 */
u8 drag_control(void);

/* 开环拖动初始化 */
void drag_control_init(void);

/* 速度PI调节器初始化 */
void  Speed_PI_init(void);

/* PI调节器计算 */
int32_t MC_PI_Handler( PidPI_t *pi,int32_t errorBack);

/* 获取电机转速 */
void get_speed(void);

/* 速度PI调节器复位 */
void  Speed_PI_reset(void);
#endif
