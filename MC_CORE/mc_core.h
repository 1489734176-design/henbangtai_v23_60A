#ifndef __MC_CORE_H
#define __MC_CORE_H

#include "stdint.h"
#include "mm32spin0230.h"

/* ADC采样值结构体：存储所有ADC通道的实时数据 */
typedef struct
{
	__IO  uint8_t  	u8ADC_Timer_flag;    // ADC定时器标志

	/* 三相BEMF电压ADC值 */
	__IO  uint16_t  u16BemfU;            // U相BEMF
	__IO  uint16_t  u16BemfV;            // V相BEMF
	__IO  uint16_t  u16BemfW;            // W相BEMF
	__IO  uint16_t  u16BemfZero;         // BEMF零点（中性点电压）

	/* 电流相关ADC值 */
	__IO  uint16_t  u16I_Peak;	        // 峰值电流ADC值
	__IO  uint16_t  u16I_Peak_calculate;	// 计算用峰值电流
	__IO  uint16_t  u16I_SUM_Avg;		// 平均电流ADC值

	/* 电压和温度ADC值 */
	__IO  uint16_t  u16Vbus;             // 母线电压
	__IO  uint16_t  u16Ref2V5;           // 2.5V参考电压
	__IO  uint16_t  u16NTC_MOS;          // MOS温度NTC
	__IO  uint16_t  u16NTC_BAT;          // 电池温度NTC
	__IO  uint16_t  u16VR;               // VR（扳机电位器）电压

} ADC_Value_TypeDef;


/* 电机错误标志联合体 */
typedef union
{
    uint16_t word;
    struct
    {
		uint16_t	gIPDOCP:1;           // IPD过流保护
		uint16_t	gLackPhase:1;        // 缺相故障，IPD值与静态偏置差值过小
		uint16_t	gBlock:1;            // 堵转故障
		uint16_t	gLosStep:1;          // 失步故障
		uint16_t	gOpenLoopErr:1;      // 开环拖动故障
    }bits;

}MC_Error_u;

/* 电机核心控制结构体：ADC/BEMF实时数据、换相计时、当前PWM、电机使能和核心故障 */
typedef struct
{
	/* ADC采样数据 */
	ADC_Value_TypeDef Board_ADC_DATA;

	/* 电机错误标志 */
	MC_Error_u MC_error;

	/* 比较器输出 */
	uint8_t current_comp_out;

	/* 开环拖动定时器 */
	uint32_t lock_timer_time;

	/* 60度电周期计时 */
	uint32_t  time_count_60_degree;

	/* 当前换相步序 */
	uint8_t bldc_step_current;

	/* BEMF过零检测标志 */
	uint8_t bldc_bemf_detected;

	/* 换相定时器计数 */
	uint32_t swith_timer_count;

	/* BEMF过零时刻定时器计数 */
	uint32_t crossed_zero_timer_count;

	/* 实际运行PWM占空比 */
	uint32_t pwm_actual_runing_data;

	/* 电机转向 */
	uint8_t mc_dir;

	/* 电机运动标志 */
	uint8_t mc_moving;

	/* 对齐计数器 */
	uint32_t  aglin_count;

	/* IPD启动标志 */
	uint8_t ipd_star_flag;

	/* 电机使能标志 */
	uint8_t  motor_en;

	/* 堵转恢复计数器 */
	uint32_t block_recover_count;

	/* 堵转计数器（总） */
	uint32_t block_count;

	/* 多级堵转计数器（按电流等级） */
	uint32_t block_count1;
	uint32_t block_count2;
	uint32_t block_count3;
	uint32_t block_count4;

	/* 运行中堵转计数器 */
	uint32_t block_count_runing;

	/* 电流采样偏置ADC值 */
	uint16_t CurrentOffset_ad_data;

	/* 拖动计数器 */
	uint16_t drag_step_count;
	uint16_t drag_step_count_compare;

	/* BEMF闭环计数器 */
	uint8_t motor_bemf_closed_count_set;
	uint8_t motor_bemf_closed_count;

	/* BEMF运行标志 */
	uint8_t motor_bemf_runing;

	/* 扇区计数器 */
	uint32_t Run_SectorCnt;

	/* 运动检测使能 */
	uint8_t  motion_en;
	uint8_t motion_processed_result;

	/* IPD过流ADC值 */
	uint16_t Ibus_Peak_Ocp_Once_AdcValue;

} Mc_core_list;

/* 电机核心参数结构体 */
typedef struct
{
	/* BEMF换相角参数 */
	uint16_t shift_closedloop_angle;              // 闭环移位角
	uint16_t mask_closedloop_angle;               // 闭环屏蔽角
	uint16_t high_speed_shift_closedloop_angle;   // 高速闭环移位角
	uint16_t high_speed_mask_closedloop_angle;    // 高速闭环屏蔽角
	uint16_t low_speed_shift_closedloop_angle;    // 低速闭环移位角
	uint16_t low_speed_mask_closedloop_angle;     // 低速闭环屏蔽角

	/* 开环拖动时间参数 */
	uint32_t mask_openloop_timer_count;           // 开环屏蔽时间
	uint32_t shift_openloop_timer_count;          // 开环移位时间

	/* BEMF过零检测窗口参数 */
	uint32_t mask_min_timer_count;                // 最小屏蔽时间
	uint32_t mask_max_timer_count;                // 最大屏蔽时间
	uint32_t shift_min_timer_count;               // 最小移位时间
	uint32_t shift_max_timer_count;               // 最大移位时间

	/* IPD参数 */
	uint16_t ipd_idle_time;                       // IPD空闲时间
	uint16_t ipd_pulse_time;                      // IPD脉冲时间

	/* 失步保护参数 */
	uint8_t  lose_step_protect_en;                // 失步保护使能
	uint16_t lose_step_protect_count_set;         // 失步保护滤波次数

	/* 强制换相参数 */
	uint8_t  force_shihf_phase_en;                // 强制换相使能
	uint32_t force_shihf_phase_timer_count_set;   // 强制换相时间
	uint16_t force_change_phase_protect_count_set;// 强制换相保护次数

	/* 反转检测使能 */
	uint8_t  motor_rotate_error_detect_en;

	/* IPD过流ADC阈值 */
	uint16_t ibus_ipd_ocp_adc_value;

	/* 峰值过流单次检测使能 */
	uint8_t  ibus_peak_ocp_once_en;

	/* 运动检测参数 */
	uint16_t motion_check_time_count;             // 运动检测检查时间
	uint16_t motion_pass_count_set;               // 运动判断滤波次数

	/* 最大扇区计数和时间 */
	uint32_t max_sector_count_set;                // 最大扇区计数
	uint32_t max_sector_time_count_set;           // 最大扇区时间

	/* 缺相检测差值 */
	uint16_t lack_phase_delta;

	/* 峰值电流ADC通道 */
	uint8_t  i_peak_ad_channel;
} McCore_Param_t;

/* 全局电机核心控制列表 */
extern Mc_core_list mc_core_list;

/* 六步换相控制 */
void step_control(unsigned char step,unsigned char dir);

/* 换相执行 */
void change_phase(void);

/* 读取定时器计数 */
u32 read_timer_count(void);

/* 电机核心初始化 */
void mc_init(void);

/* 堵转检测 */
void motor_block_detect(void);

/* 运动检测处理 */
void motion_handle(void);

/* IPD初始位置检测 */
void ipd_detect(void);

/* 切换到BEMF闭环运行准备 */
void motor_to_bemf_runing_pre(void);

/* TIM14中断处理 */
void mc_core_timer14_isr_handle(void);

/* ADC中断处理 */
void mc_core_adc_isr_handle(void);

/* 最大扇区时间处理 */
void max_sector_handle(void);

/* 比较器中断处理 */
void mc_core_comp_isr_handle(void);

/* 失步保护初始化 */
void mc_lose_step_init(void);

/* 电机核心列表初始化 */
void mc_core_list_init(const McCore_Param_t *param);

/* 电机停止复位 */
void mc_core_stop_reset(void);
#endif
