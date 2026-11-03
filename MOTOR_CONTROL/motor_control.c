#include "motor_control.h"
#include "mc_core.h"
#include "drv_pwm.h"
#include "parameter.h"
#include "board.h"
#include "drv_adc.h"
#include "user_control.h"
#include "systick.h"
#include "motor_config.h"

/* 全局电机控制列表实例 */
Motor_Control_list motor_control_list;

#if LIMIT_SPEED_EN
/* 速度PI调节器实例 */
PidPI_t Speed_Pid;
#endif

extern u32 PI_sum_i;
extern u32 PI_p;
extern u32 pwm_duty_aim_PI_OUT;

/******************************************************************************
* 函数名: mc_init
* 描  述: 电机控制系统初始化，配置核心参数并初始化所有状态变量
*        将motor_config.h中的参数加载到mc_core参数结构体中
******************************************************************************/
void mc_init(void)
{
	McCore_Param_t p;
	/* 加载BEMF换相角参数（掩码角/移位角）闭环 */
	p.shift_closedloop_angle = ShiftTime_ClosedLoop_Angle;
	p.mask_closedloop_angle = MaskTime_ClosedLoop_Angle;
	/* 高速BEMF换相角参数 */
	p.high_speed_shift_closedloop_angle = High_Speed_ShiftTime_ClosedLoop_Angle;
	p.high_speed_mask_closedloop_angle = High_Speed_MaskTime_ClosedLoop_Angle;
	/* 低速BEMF换相角参数 */
	p.low_speed_shift_closedloop_angle = LOW_Speed_ShiftTime_ClosedLoop_Angle;
	p.low_speed_mask_closedloop_angle = LOW_Speed_MaskTime_ClosedLoop_Angle;
	/* 开环拖动阶段的掩码/移位时间 */
	p.mask_openloop_timer_count = MaskTime_OpenLoop_TIMER_COUNT;
	p.shift_openloop_timer_count = ShiftTime_OpenLoop_TIMER_COUNT;
	/* 最小/最大掩码时间和移位时间，用于BEMF过零检测窗口 */
	p.mask_min_timer_count = MaskTime_Min_TIMER_COUNT;
	p.mask_max_timer_count = MaskTime_Max_TIMER_COUNT;
	p.shift_min_timer_count = ShiftTime_Min_TIMER_COUNT;
	p.shift_max_timer_count = ShiftTime_Max_TIMER_COUNT;
	/* IPD初始位置检测参数 */
	p.ipd_idle_time = MotorStart_IPD_IdleTime;
	p.ipd_pulse_time = MotorStart_IPD_PulseTime;
	/* 失步保护配置 */
	p.lose_step_protect_en = LosStep_Protect_En;
	p.lose_step_protect_count_set = LosStep_Protect_CntSet;
	/* 强制换相配置 */
	p.force_shihf_phase_en = ForceChangePhase_En;
	p.force_shihf_phase_timer_count_set = ForceChangePhase_CntSet;
	p.force_change_phase_protect_count_set = ForceChangePhaseProtect_TimesSet;
	/* 电机反转检测使能 */
	p.motor_rotate_error_detect_en = MOTOR_ROTATE_ERROR_DETECT_En;
	/* IPD过流保护ADC阈值 */
	p.ibus_ipd_ocp_adc_value = IBus_IPD_OCP_AdcValue;
	/* 峰值过流单次检测使能 */
	p.ibus_peak_ocp_once_en = IBus_Peak_OCP_ONCE_EN;
	/* 运动检测（转动判断）参数 */
	p.motion_check_time_count = Motion_CheckTime_Count;
	p.motion_pass_count_set = Motion_Pass_CntSet;
	/* 最大扇区计数和时间 */
	p.max_sector_count_set = SectorTIme_Max_COUNT;
	p.max_sector_time_count_set = SectorTIme_Max_TIMER_COUNT;
	/* 缺相检测差值（1倍电流ADC值） */
	p.lack_phase_delta = (u16)(CURRENT_ADC_PER_A);
	/* 运放输出通道号 */
	p.i_peak_ad_channel = OP_O_CHANNEL;

	/* 初始化电机状态机为停止状态 */
	motor_control_list.bldc_state = MOTOR_STOP;
	/* 加载软/硬制动使能配置 */
	motor_control_list.hbrake_en = HBrake_En;
	motor_control_list.sbrake_en = SBrake_En;
	motor_control_list.motor_run_count = 0;

	/* 清除错误标志 */
	mc_core_list.MC_error.word = 0;
	/* 初始化开环拖动定时器为开环掩码时间 */
	mc_core_list.lock_timer_time = MaskTime_OpenLoop_TIMER_COUNT;
	mc_core_list.time_count_60_degree = 0;
	mc_core_list.bldc_step_current = 0;
	mc_core_list.bldc_bemf_detected = 0;
	mc_core_list.crossed_zero_timer_count = 0;
	mc_core_list.pwm_actual_runing_data = 0;
	mc_core_list.mc_moving = 0;
	mc_core_list.aglin_count = 0;
	mc_core_list.mc_dir = Tool_Dir_Init;
	mc_core_list.ipd_star_flag = 0;
	mc_core_list.motor_en = 0;
	/* 堵转保护计数器清零 */
	mc_core_list.block_count = 0;
	mc_core_list.block_count1 = 0;
	mc_core_list.block_count2 = 0;
	mc_core_list.block_count3 = 0;
	mc_core_list.block_count4 = 0;
	mc_core_list.block_count_runing = 0;
	/* 拖动计数器清零 */
	mc_core_list.drag_step_count = 0;
	mc_core_list.drag_step_count_compare = 0;
	mc_core_list.motor_bemf_runing = 0;
	mc_core_list.Run_SectorCnt = 0;
	/* 运动检测使能 */
	mc_core_list.motion_en = Motion_En;
	mc_core_list.motion_processed_result = 0;
	/* 单次峰值过流ADC阈值 */
	mc_core_list.Ibus_Peak_Ocp_Once_AdcValue = IBus_Peak_OCP_ONCE_AdcValue;

	mc_core_list_init(&p);
}

/******************************************************************************
* 函数名: motor_block_detect
* 描  述: 堵转保护检测函数
*        支持两种模式:
*        1. Block_Protect_Current_En=1: 基于电流等级的多级堵转保护，
*           电流越大允许的堵转时间越短（反时限特性）
*        2. Block_Protect_Current_En=0: 基于PWM占空比判断的堵转时间保护，
*           低占空比允许更长的堵转时间
*        堵转恢复时间由Block_Protect_Recover_TIME控制
******************************************************************************/
void motor_block_detect(void)
{
	u32 block_ipeak_data=0;

	/* 计算当前峰值电流（减去电流采样偏置值） */
	if(mc_core_list.Board_ADC_DATA.u16I_Peak>mc_core_list.CurrentOffset_ad_data)
	{
		block_ipeak_data=mc_core_list.Board_ADC_DATA.u16I_Peak-mc_core_list.CurrentOffset_ad_data;
	}
	else
	{
		block_ipeak_data=0;
	}

	#if Block_Protect_Current_En

	/* 基于电流等级的多级堵转保护（仅在运行或开环拖动状态生效） */
	if(motor_control_list.bldc_state==MOTOR_RUN||motor_control_list.bldc_state==MotorStart_Drag_OPENLOOP)
	{
		/* 总堵转计数器：超过最大时间直接报堵转 */
		mc_core_list.block_count++;
		if(mc_core_list.block_count>Block_Protect_MAX_TIME)
		{
			mc_core_list.MC_error.bits.gBlock=1;
			mc_core_list.block_recover_count=0;
		}

		/* 运行中堵转时间：电机运行超过250ms后，检测连续运行时间是否超限 */
		if(motor_control_list.motor_run_count>(250*SLOWLOOP_1ms_CNT_LOAD))
		{
			mc_core_list.block_count_runing++;
			if(mc_core_list.block_count_runing>Block_Protect_RUNING_TIME)
			{
				mc_core_list.MC_error.bits.gBlock=1;
				mc_core_list.block_recover_count=0;
			}
		}
		else
		{
			mc_core_list.block_count_runing=0;
		}

		/* 电流等级1: >45A，允许堵转80ms */
		if(block_ipeak_data>Block_Protect_Current1_AdcValue)
		{
			mc_core_list.block_count1++;
			if(mc_core_list.block_count1>Block_Protect_Current1_TIME)
			{
				mc_core_list.MC_error.bits.gBlock=1;
				mc_core_list.block_recover_count=0;
			}
		}

		/* 电流等级2: >50A，允许堵转50ms */
		if(block_ipeak_data>Block_Protect_Current2_AdcValue)
		{
			mc_core_list.block_count2++;
			if(mc_core_list.block_count2>Block_Protect_Current2_TIME)
			{
				mc_core_list.MC_error.bits.gBlock=1;
				mc_core_list.block_recover_count=0;
			}
		}

		/* 电流等级3: >55A，允许堵转30ms */
		if(block_ipeak_data>Block_Protect_Current3_AdcValue)
		{
			mc_core_list.block_count3++;
			if(mc_core_list.block_count3>Block_Protect_Current3_TIME)
			{
				mc_core_list.MC_error.bits.gBlock=1;
				mc_core_list.block_recover_count=0;
			}
		}

		/* 电流等级4: >60A，允许堵转10ms（最严重，最短延时） */
		if(block_ipeak_data>Block_Protect_Current4_AdcValue)
		{
			mc_core_list.block_count4++;
			if(mc_core_list.block_count4>Block_Protect_Current4_TIME)
			{
				mc_core_list.MC_error.bits.gBlock=1;
				mc_core_list.block_recover_count=0;
			}
		}
	}
	else
	{
		/* 非运行状态：清零堵转计数器，开始恢复计时 */
		mc_core_list.block_count=0;
		mc_core_list.block_count1=0;
		mc_core_list.block_count2=0;
		mc_core_list.block_count3=0;
		mc_core_list.block_count4=0;
		mc_core_list.block_count_runing=0;
		mc_core_list.block_recover_count++;
		/* 恢复时间到达后清除堵转错误 */
		if(mc_core_list.block_recover_count>Block_Protect_Recover_TIME)
		{
			mc_core_list.MC_error.bits.gBlock=0;
			mc_core_list.block_recover_count=0;
		}
	}

	#else

	/* 基于PWM占空比判断的堵转保护 */
	if(motor_control_list.bldc_state==MOTOR_RUN||motor_control_list.bldc_state==MotorStart_Drag_OPENLOOP)
	{
		mc_core_list.block_count++;

		if(mc_core_list.pwm_actual_runing_data<Block_Protect_LowDuty_DATA)
		{
			/* 低占空比：允许较长的堵转时间 */
			if(mc_core_list.block_count>Block_Protect_LowDuty_COUNT)
			{
				mc_core_list.block_recover_count=0;
				mc_core_list.MC_error.bits.gBlock=1;
			}
		}
		else
		{
			/* 高占空比：允许较短的堵转时间 */
			if(mc_core_list.block_count>Block_Protect_HighDuty_COUNT)
			{
				mc_core_list.MC_error.bits.gBlock=1;
			}
		}
	}
	else
	{
		mc_core_list.block_count=0;
		mc_core_list.block_recover_count++;
		if(mc_core_list.block_recover_count>Block_Protect_Recover_TIME)
		{
			mc_core_list.MC_error.bits.gBlock=0;
			mc_core_list.block_recover_count=0;
		}
	}
	#endif
}

/******************************************************************************
* 函数名: pwm_set
* 描  述: PWM占空比设置函数
*        实现PWM占空比的软启动斜坡控制：
*        - pwm_duty_virtual: 虚拟目标占空比（软斜坡输出）
*        - pwm_actual_runing_data: 实际输出占空比（快速跟踪）
*        支持电流限流功能：当电流超限时强制降低占空比
*        支持重启功能：正反转切换时限制最小占空比
******************************************************************************/
void pwm_set(void)
{
	static u8 pwm_add_count=0;
	static u8 pwm_sub_count=0;
	static u8 pwm_add_fast_count=0;
	static u8 pwm_sub_fast_count=0;
	u8 limit_current_on=0;

	#if(LIMIT_PEAK_CURRENT_EN)
	uint16_t IBusPeak_Adc_temp=0;
	#endif
	#if(LIMIT_AVG_CURRENT_EN)
	u32 limit_current_temp=0;
	uint16_t IBusAvg_Adc_temp=0;
	#endif

	/* 计算峰值电流ADC值（减去偏置，并校准VCC增益） */
	#if(LIMIT_PEAK_CURRENT_EN)
		if(mc_core_list.Board_ADC_DATA.u16I_Peak>mc_core_list.CurrentOffset_ad_data)
	{
		IBusPeak_Adc_temp=mc_core_list.Board_ADC_DATA.u16I_Peak-mc_core_list.CurrentOffset_ad_data;
		IBusPeak_Adc_temp=((((u32)IBusPeak_Adc_temp)*user_list.Vcc_cal_gain)>>15);
	}
	else
	{
		IBusPeak_Adc_temp=0;
	}
	#endif

	/* 计算平均电流ADC值（减去偏置，并校准VCC增益） */
	#if(LIMIT_AVG_CURRENT_EN)
	if( mc_core_list.Board_ADC_DATA.u16I_SUM_Avg> mc_core_list.CurrentOffset_ad_data)
	{
		IBusAvg_Adc_temp=mc_core_list.Board_ADC_DATA.u16I_SUM_Avg-mc_core_list.CurrentOffset_ad_data;
		IBusAvg_Adc_temp=((((u32)IBusAvg_Adc_temp)*user_list.Vcc_cal_gain)>>15);
	}
	else
	{
		IBusAvg_Adc_temp=0;
	}
	#endif

	/* 峰值电流限流判断 */
	#if(LIMIT_PEAK_CURRENT_EN)
	if(IBusPeak_Adc_temp>LIMIT_PEAK_CURRENT_AdcValue)
	{
		limit_current_on=1;
	}
	#endif

	/* 平均电流限流判断：根据转向和档位选择不同的限流阈值 */
	#if(LIMIT_AVG_CURRENT_EN)
	if(user_list.flag.bits.gToolDir==Tool_Dir_CCW_Value)
	{
		/* 反转：使用反转限流值 */
		limit_current_temp=LIMIT_AVG_CURRENT_AdcValue_CCW;
	}
	else
	{
		/* 正转：根据档位选择限流值，档位越高限流越大 */
		if(user_list.gears_level==0)
		{
			limit_current_temp=LIMIT_AVG_CURRENT1_AdcValue;
		}
		else if(user_list.gears_level==1)
		{
			limit_current_temp=LIMIT_AVG_CURRENT2_AdcValue;
		}
		else
		{
			limit_current_temp=LIMIT_AVG_CURRENT3_AdcValue;
		}
	}
	if(IBusAvg_Adc_temp>limit_current_temp)
	{
		limit_current_on=1;
	}
	#endif

	/* 虚拟占空比斜坡上升：每PWM_ADD_GAP次中断增加PWM_ADD_VALUE */
	if((motor_control_list.pwm_duty_virtual<motor_control_list.pwm_duty_aim))
	{
		pwm_add_count++;
		if(pwm_add_count>=PWM_ADD_GAP)
		{
			pwm_add_count=0;
			motor_control_list.pwm_duty_virtual+=PWM_ADD_VALUE;
			/* 不超过目标值 */
			if(motor_control_list.pwm_duty_virtual>motor_control_list.pwm_duty_aim)
			{
				motor_control_list.pwm_duty_virtual=motor_control_list.pwm_duty_aim;
			}
		}
	}
	/* 虚拟占空比斜坡下降：每PWM_SUB_GAP次中断减少PWM_SUB_VALUE */
	else if(motor_control_list.pwm_duty_virtual>motor_control_list.pwm_duty_aim)
	{
		pwm_sub_count++;
		if(pwm_sub_count>=PWM_SUB_GAP)
		{
			pwm_sub_count=0;
			if(motor_control_list.pwm_duty_virtual>PWM_SUB_VALUE)
			{
				motor_control_list.pwm_duty_virtual-=PWM_SUB_VALUE;
			}
			/* 不低于目标值 */
			if(motor_control_list.pwm_duty_virtual<motor_control_list.pwm_duty_aim)
			{
				motor_control_list.pwm_duty_virtual=motor_control_list.pwm_duty_aim;
			}
		}
	}

	/* 虚拟占空比限幅 */
	if(motor_control_list.pwm_duty_virtual<LIMIT_MIN_PWN_DUTY)
	{
		motor_control_list.pwm_duty_virtual=LIMIT_MIN_PWN_DUTY;
	}
	if(motor_control_list.pwm_duty_virtual>LIMIT_MAX_PWN_DUTY)
	{
		motor_control_list.pwm_duty_virtual=LIMIT_MAX_PWN_DUTY;
	}

	#if LIMIT_SPEED_EN
	/* 速度闭环模式：实际占空比跟踪pwm_duty_aim2（PI输出） */
		if((mc_core_list.pwm_actual_runing_data<motor_control_list.pwm_duty_aim2)&&(limit_current_on==0))
	{
		pwm_add_fast_count++;
		if(pwm_add_fast_count>=PWM_ADD_FAST_GAP)
		{
			pwm_add_fast_count=0;
			mc_core_list.pwm_actual_runing_data+=PWM_ADD_FAST_VALUE;
			if(mc_core_list.pwm_actual_runing_data>motor_control_list.pwm_duty_aim2)
			{
				mc_core_list.pwm_actual_runing_data=motor_control_list.pwm_duty_aim2;
			}
		}
	}
	if(mc_core_list.pwm_actual_runing_data>motor_control_list.pwm_duty_aim2||limit_current_on)
	{
		pwm_sub_fast_count++;
		if(pwm_sub_fast_count>=PWM_SUB_FAST_GAP)
		{
			pwm_sub_fast_count=0;
			if(mc_core_list.pwm_actual_runing_data>PWM_SUB_FAST_VALUE)
			{
				mc_core_list.pwm_actual_runing_data-=PWM_SUB_FAST_VALUE;
			}
			if((mc_core_list.pwm_actual_runing_data<motor_control_list.pwm_duty_aim2)&&(limit_current_on==0))
			{
				mc_core_list.pwm_actual_runing_data=motor_control_list.pwm_duty_aim2;
			}
		}
	}
	#else
	/* 非速度闭环模式：实际占空比快速跟踪虚拟占空比 */
	if((mc_core_list.pwm_actual_runing_data<motor_control_list.pwm_duty_virtual)&&(limit_current_on==0))
	{
		pwm_add_fast_count++;
		if(pwm_add_fast_count>=PWM_ADD_FAST_GAP)
		{
			pwm_add_fast_count=0;
			mc_core_list.pwm_actual_runing_data+=PWM_ADD_FAST_VALUE;
			if(mc_core_list.pwm_actual_runing_data>motor_control_list.pwm_duty_virtual)
			{
				mc_core_list.pwm_actual_runing_data=motor_control_list.pwm_duty_virtual;
			}
		}
	}
	if(mc_core_list.pwm_actual_runing_data>motor_control_list.pwm_duty_virtual||limit_current_on)
	{
		pwm_sub_fast_count++;
		if(pwm_sub_fast_count>=PWM_SUB_FAST_GAP)
		{
			pwm_sub_fast_count=0;
			if(mc_core_list.pwm_actual_runing_data>PWM_SUB_FAST_VALUE)
			{
				mc_core_list.pwm_actual_runing_data-=PWM_SUB_FAST_VALUE;
			}
			if((mc_core_list.pwm_actual_runing_data<motor_control_list.pwm_duty_virtual)&&(limit_current_on==0))
			{
				mc_core_list.pwm_actual_runing_data=motor_control_list.pwm_duty_virtual;
			}
		}
	}
	#endif

	/* 重启功能：有负载且方向改变时限制最小占空比，防止重启冲击 */
	#if(RESTAR_EN&&Wrench_EN)
	if(user_list.Load_exist||(user_list.flag.bits.gToolDir==Tool_Dir_CCW_Value&&user_list.flag.bits.gAutoStopEn))
	{
		if(mc_core_list.pwm_actual_runing_data<RESTAR_MIN_PWM_LIMIT_DUTY)
		{
			mc_core_list.pwm_actual_runing_data=RESTAR_MIN_PWM_LIMIT_DUTY;
		}
	}
	#endif

	/* 实际输出占空比限幅 */
	if(mc_core_list.pwm_actual_runing_data<LIMIT_MIN_PWN_DUTY)
	{
		mc_core_list.pwm_actual_runing_data=LIMIT_MIN_PWN_DUTY;
	}
	if(mc_core_list.pwm_actual_runing_data>LIMIT_MAX_PWN_DUTY)
	{
		mc_core_list.pwm_actual_runing_data=LIMIT_MAX_PWN_DUTY;
	}

	/* 更新PWM硬件寄存器 */
	PwmDutyUpdate(mc_core_list.pwm_actual_runing_data);
}

/******************************************************************************
* 函数名: CurrentOffsetCalibration
* 描  述: 电流采样偏置校准函数
*        在电机停止状态下，切换ADC到运放输出通道(OP_O_CHANNEL)，
*        采集16次ADC值取平均，作为电流零偏值。
*        校准期间关闭TIM1中断和ADC外部触发，校准完成后恢复ADC正常配置。
******************************************************************************/
void CurrentOffsetCalibration(void)
{
	u16 t_ccr4_temp;
	u8 i=0;
	u32 ibus_adc_sum=0;

	/* 停止电机输出 */
	Stop_Motor();
	/* 关闭TIM1更新中断 */
	TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
	systick_delay_us(500);

	/* 保存并修改TIM1 CCR4为最大值，确保ADC触发关闭 */
	t_ccr4_temp=TIM1->CCR4;
	TIM1->CCR4 = 65535;
	systick_delay_us(2000);

	/* 等待ADC转换完成 */
	while(ADC1->ADSTA&0x00000004)
	{;}

	/* 关闭ADC外部触发转换 */
	ADC_ExternalTrigConvCmd(ADC1, DISABLE);
	systick_delay_us(500);

	/* 禁用ADC和中断，切换通道进行校准采样 */
	ADC_Cmd(ADC1, DISABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
	TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);

	/* 配置ADC单通道模式，选择运放输出通道 */
	ADC_AnyChannelNumCfg(ADC1, 0);
	ADC_AnyChannelSelect(ADC1, 0, OP_O_CHANNEL);
	ADC_AnyChannelCmd(ADC1, ENABLE);

	/* 确认外部触发关闭 */
	ADC_ExternalTrigConvCmd(ADC1, DISABLE);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SampleTimeConfig(ADC1,0, ADC_SampleTime_3_5);
	ADC_Cmd(ADC1, ENABLE);
	systick_delay_us(500);

	/* 丢弃前两次采样，稳定ADC */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

	/* 采集16次取平均作为零偏值 */
	ibus_adc_sum=0;
	for(i=0;i<16;i++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		ibus_adc_sum += (uint16_t)GET_ADC_VALUE(OP_O_CHANNEL);
	}
	/* 右移4位等价于除以16，得到16次采样的平均值 */
	mc_core_list.CurrentOffset_ad_data=ibus_adc_sum>>4;

	/* 禁用ADC，恢复系统ADC配置 */
	ADC_Cmd(ADC1, DISABLE);
	systick_delay_us(100);
	Board_ADC_Init();

	/* 恢复TIM1 CCR4值 */
	TIM1->CCR4=t_ccr4_temp;

	/* 标记校准完成 */
	motor_control_list.u8Calibration_OK_flag=1;
}

/******************************************************************************
* 函数名: mos_check_current
* 描  述: MOS管短路检测（逐相检测）
*        对指定相的MOS管施加导通/关断信号，检测电流是否超过短路阈值。
*        如果电流超过MOS_SHORT_CURRENT_AdcValue或比较器触发(OCP)，
*        则判定为MOS短路，停止电机并记录错误（高侧或低侧桥臂）。
* 参  数: mose_check_inedx - 检测相序号
*        0: U相低侧导通  1: V相低侧导通  2: W相低侧导通
*        3: U相高侧导通  4: V相高侧导通  5: W相高侧导通
******************************************************************************/
void mos_check_current(u8 mose_check_inedx)
{
	u8 i=0;
	u16 mos_check_ad=0;

	/* 根据序号控制对应相的MOS管 */
	switch(mose_check_inedx)
	{
		case 0:	UL_HOffLOn();	break;  /* U相低侧导通，高侧关断 */
		case 1:	VL_HOffLOn();	break;  /* V相低侧导通，高侧关断 */
		case 2:	WL_HOffLOn();	break;  /* W相低侧导通，高侧关断 */
		case 3:	UH_HOnLOff();	break;  /* U相高侧导通，低侧关断 */
		case 4:	VH_HOnLOff();	break;  /* V相高侧导通，低侧关断 */
		case 5:	WH_HOnLOff();	break;  /* W相高侧导通，低侧关断 */
		default :	break;
	}

	/* 多次采样检测是否过流 */
	for(i=0;i<20;i++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		mos_check_ad= (uint16_t)GET_ADC_VALUE(OP_O_CHANNEL);

		/* 减去偏置 */
		if(mos_check_ad>mc_core_list.CurrentOffset_ad_data)
		{
			mos_check_ad=mos_check_ad-mc_core_list.CurrentOffset_ad_data;
		}
		else
		{
			mos_check_ad=0;
		}

		/* 过流或比较器触发：判定MOS短路 */
		if(mos_check_ad>MOS_SHORT_CURRENT_AdcValue||	machine_error_hold.userErr_hold.bits.gIBusAcmpOCP==1)
		{
			Stop_Motor();
			machine_error_hold.userErr_hold.bits.gIBusAcmpOCP=0;
			/* 序号0-2为低侧MOS，3-5为高侧MOS */
			if(mose_check_inedx<3)
			{
				user_list.userErr.bits.gMosBridgeH=1;  /* 低侧短路 */
			}
			else
			{
				user_list.userErr.bits.gMosBridgeL=1;  /* 高侧短路 */
			}
			break;
		}
	}

	Stop_Motor();
}

/******************************************************************************
* 函数名: mose_check
* 描  述: MOS管全面短路检测
*        依次对6个MOS管（3个低侧+3个高侧）进行短路检测。
*        检测前需停止电机，关闭TIM1中断，将ADC切换到运放输出通道。
*        检测完成后恢复ADC正常配置。
*        注意：检测期间TIM1 CCR4设为65535以关闭ADC触发。
******************************************************************************/
void mose_check(void)
{
	u8 i=0;
	u16 t_ccr4_temp;

	/* 停止电机 */
	Stop_Motor();
	/* 关闭TIM1更新中断 */
	TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
	systick_delay_us(500);

	/* 保存并修改TIM1 CCR4 */
	t_ccr4_temp=TIM1->CCR4;
	TIM1->CCR4 = 65535;
	systick_delay_us(2000);

	/* 等待ADC转换完成 */
	while(ADC1->ADSTA&0x00000004)
	{;}

	/* 关闭外部触发和ADC中断 */
	ADC_ExternalTrigConvCmd(ADC1, DISABLE);
	systick_delay_us(500);
	ADC_Cmd(ADC1, DISABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
	TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
	ADC_ExternalTrigConvCmd(ADC1, DISABLE);

	/* 配置ADC单通道模式，选择运放输出通道 */
	ADC_AnyChannelNumCfg(ADC1, 0);
	ADC_AnyChannelSelect(ADC1, 0, OP_O_CHANNEL);
	ADC_AnyChannelCmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	systick_delay_us(500);

	/* 丢弃前两次采样 */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

	/* 依次检测6个MOS管，每次间隔800us */
	for(i=0;i<6;i++)
	{
		mos_check_current(i);
		systick_delay_us(800);
	}

	/* 恢复ADC和TIM1配置 */
	ADC_Cmd(ADC1, DISABLE);
	systick_delay_us(500);
	Board_ADC_Init();
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	TIM1->CCR4=t_ccr4_temp;
}

/******************************************************************************
* 函数名: start_mc_init
* 描  述: 启动前初始化，清零对齐计数器
******************************************************************************/
void start_mc_init(void)
{
	mc_core_list.aglin_count=0;
}

/******************************************************************************
* 函数名: align_control
* 描  述: 对齐控制，强制输出第0相，使转子对齐到已知位置
******************************************************************************/
void align_control(void)
{
	mc_core_list.bldc_step_current=0;
	step_control(mc_core_list.bldc_step_current,mc_core_list.mc_dir);
}

/******************************************************************************
* 函数名: drag_control_init
* 描  述: 开环拖动初始化，设置拖动计数器和比较值
*        drag_step_count: 拖动计数器（决定换相频率）
*        drag_step_count_compare: 换相比较阈值（越小换相越快）
******************************************************************************/
void drag_control_init(void)
{
	mc_core_list.drag_step_count=600;
	mc_core_list.drag_step_count_compare=500;
}

/******************************************************************************
* 函数名: drag_control
* 描  述: 开环拖动控制，按固定频率强制换相
*        拖动过程中逐步减小换相周期（加快换相速度），
*        当换相周期减小到阈值后返回1，切换到闭环运行。
* 返回值: 0-拖动未完成  1-拖动完成，准备切入闭环
******************************************************************************/
u8 drag_control(void)
{
	u8 res=0;

	mc_core_list.drag_step_count++;
	if(mc_core_list.drag_step_count>(mc_core_list.drag_step_count_compare))
	{
		if(mc_core_list.drag_step_count_compare>80)
		{
			/* 逐步减小换相周期，加速拖动 */
			res=0;
			mc_core_list.drag_step_count_compare=mc_core_list.drag_step_count_compare-10;
		}
		else
		{
			/* 换相周期已减小到阈值，拖动完成 */
			res=1;
		}
		/* 执行换相 */
		change_phase();
		/* 记录换相时刻的定时器值 */
		mc_core_list.crossed_zero_timer_count=read_timer_count();
		mc_core_list.drag_step_count=0;
	}

	return res;
}

/******************************************************************************
* 函数名: MC_Machine_State
* 描  述: 电机状态机主函数，周期调用（通常1ms）
*        状态转换流程:
*        MOTOR_STOP → MOTOR_MOTION(运动检测) → MOTOR_POSITION(IPD) → MOTOR_RUN(闭环运行)
*                    ↘ MOTOR_READY(准备/制动等待) → MOTOR_SBRAKE/MOTOR_HBRAKE → MOTOR_BRAKE2STOP → MOTOR_STOP
*        MOTOR_RUN → (扳机释放) → MOTOR_HBRAKE/MOTOR_SBRAKE → MOTOR_BRAKE2STOP → MOTOR_STOP
*        MOTOR_RUN → (堵转恢复) → MOTOR_STOP → MOTOR_MOTION
*        各状态说明:
*        MOTOR_STOP: 停止状态，等待启动条件
*        MOTOR_MOTION: 检测电机是否已在转动（BEMF判断）
*        MOTOR_READY: 准备状态，制动等待后进入位置检测
*        MOTOR_POSITION: IPD初始位置检测
*        Motor_Align: 转子对齐
*        MotorStart_Drag_OPENLOOP: 开环拖动加速
*        MOTOR_RUN: 闭环运行
*        MOTOR_HBRAKE: 硬制动（短接电机绕组）
*        MOTOR_SBRAKE: 软制动（PWM控制制动电流）
*        MOTOR_BRAKE2STOP: 制动到停止的过渡状态
*        MOTOR_BRAKE_WAIT: 制动等待状态（用于软启动重启判断）
******************************************************************************/
void MC_Machine_State(void)
{
	static u16 wait_break_count=0;
	static u16 hbreak_count=0;
	static u16 sbreak_count=0;
	static u16 break2stop_count=0;
	static u8 motionless_count=0;

	switch (motor_control_list.bldc_state)
	{
		/*==================== 停止状态 ====================*/
		case MOTOR_STOP :
			Stop_Motor();
			/* 清零PI积分项 */
			PI_sum_i=0;
			PI_p=0;
			pwm_duty_aim_PI_OUT=0;

			mc_core_stop_reset();
			mc_core_list.pwm_actual_runing_data=0;
			motor_control_list.pwm_duty_virtual=0;
			motor_control_list.motor_run_count=0;

			/* 电机使能且无故障时进入运动检测 */
			if(mc_core_list.motor_en==1&&machine_error_hold.userErr_hold.word==0&&machine_error_hold.MC_error_hold.word==0)
			{
				motor_control_list.bldc_state=MOTOR_MOTION;
				mc_core_list.mc_moving=0;
				mc_core_list.motion_processed_result=0;
				motionless_count=0;
			}
			break;

		/*==================== 运动检测状态 ====================*/
		case MOTOR_MOTION :
			/* 电机禁用或有故障时返回停止 */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				motor_control_list.bldc_state=MOTOR_STOP;
				mc_core_list.mc_moving=0;
				break;  /* 早退出，避免后续 motion_handle 覆盖 STOP */
			}

			/* BEMF超过阈值，判断电机是否已在转动 */
			if(mc_core_list.Board_ADC_DATA.u16BemfU>Motion_ADC_THRESHOLD||mc_core_list.Board_ADC_DATA.u16BemfV>Motion_ADC_THRESHOLD||mc_core_list.Board_ADC_DATA.u16BemfW>Motion_ADC_THRESHOLD)
			{
				motion_handle();
				if(mc_core_list.motion_processed_result==1)
				{
					/* 运动检测失败 */
					motor_control_list.bldc_state=MOTOR_READY;
				}
				else if(mc_core_list.motion_processed_result==2)
				{
					/* 检测到电机在转动，进入闭环 */
					if(mc_core_list.motion_en==0)
					{
						motor_control_list.bldc_state=MOTOR_READY;
					}
					else
					{
						mc_core_list.mc_moving=0;
						mc_core_list.motor_bemf_runing=1;
						motor_control_list.bldc_state=MOTOR_RUN;
					}
				}
				motionless_count=0;
			}
			else
			{
				/* BEMF无信号，超时后进入READY或POSITION */
				motionless_count++;
				if(motionless_count>10)
				{
					motor_control_list.bldc_state=MOTOR_READY;
					hbreak_count=0;
				}
			}
			break;

		/*==================== 准备状态 ====================*/
		case MOTOR_READY:
			if(mc_core_list.mc_moving)
			{
				hbreak_count=0;
				mc_core_list.mc_moving=0;
			}
			/* 电机禁用或有故障时返回停止 */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				motor_control_list.bldc_state=MOTOR_STOP;
				break;  /* 早退出，避免继续 Brake_Motor 或误置 IPD 请求 */
			}
			/* 执行制动 */
			Brake_Motor();
			hbreak_count++;
			/* 制动时间到达后进入位置检测 */
			if(hbreak_count>Motion_Fail_BrakeTime_Count)
			{
				mc_core_list.bldc_bemf_detected=0;
				motor_control_list.bldc_state=MOTOR_POSITION;
				mc_core_list.ipd_star_flag=1;
			}
			break;

		/*==================== 对齐状态 ====================*/
		case Motor_Align :
			/* 故障判断：有错误进入停止，无错误进入制动 */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				else if(motor_control_list.sbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_SBRAKE;
					sbreak_count=0;
				}
				else if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				break;  /* 早退出，避免继续对齐/拖动 */
			}

			/* 对齐计数：对齐200ms后开始拖动 */
			if(mc_core_list.aglin_count<200)
			{
				mc_core_list.aglin_count++;
			}
			else
			{
				drag_control_init();
				/* 换相到下一相，准备开环拖动 */
				mc_core_list.bldc_step_current++;
				if(mc_core_list.bldc_step_current>=6)
				{
					mc_core_list.bldc_step_current=0;
				}
				/* 关闭TIM14，准备进入闭环 */
				TIM_Cmd(TIM14, DISABLE);
				TIM_ClearFlag(TIM14, TIM_FLAG_Update);
				mc_core_list.bldc_bemf_detected=0;
				mc_core_list.motor_bemf_runing=1;
				motor_control_list.bldc_state=MotorStart_Drag_OPENLOOP;
				mc_core_list.lock_timer_time=12000;  /* 开环拖动超时时间 */
			}
			align_control();
			break;

		/*==================== 位置检测（IPD）状态 ====================*/
		case MOTOR_POSITION:
			/* 电机禁用或有故障时处理 */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				mc_core_list.motor_bemf_runing=0;
				if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				else if(motor_control_list.sbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_SBRAKE;
					sbreak_count=0;
				}
				else if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				break;  /* 早退出，避免 ipd_star_flag==0 时又切回 RUN */
			}

			/* IPD检测完成后进入运行 */
			if(mc_core_list.ipd_star_flag==0)
			{
				motor_to_bemf_runing_pre();
				motor_control_list.bldc_state=MOTOR_RUN;
			}
			break;

		/*==================== 开环拖动状态 ====================*/
		case MotorStart_Drag_OPENLOOP :
			/* 电机禁用或有故障时处理（pwm_set 已移到判停之后，禁用时不再输出 PWM） */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				else if(motor_control_list.sbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_SBRAKE;
					sbreak_count=0;
				}
				else if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
				break;  /* 早退出，避免禁用后继续拖动 */
			}
			pwm_set();  /* 通过许可后才输出 PWM */

			/* 拖动完成（换相加速到阈值），切入闭环 */
			if(drag_control())
			{
				mc_core_list.motor_bemf_runing=1;
				motor_control_list.bldc_state=MOTOR_RUN;
			}
			break;

		/*==================== 闭环运行状态 ====================*/
		case MOTOR_RUN :
			/* 记录当前转向 */
			motor_control_list.motor_run_last_dir=mc_core_list.mc_dir;
			pwm_set();

			/* 运行计时 */
			if(motor_control_list.motor_run_count<0xffffffff)
			{
				motor_control_list.motor_run_count++;
			}

			/* 电机禁用或有故障时处理 */
			if(mc_core_list.motor_en==0||machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				mc_core_list.motor_bemf_runing=0;
				if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
				{
					#if(RESTAR_EN)
					/* 重启功能：非堵转故障直接停止，堵转故障允许重启 */
					if(machine_error_hold.MC_error_hold.word&0xfffb)
					{
						motor_control_list.bldc_state=MOTOR_STOP;
					}
					else
					{
						/* 堵转计数，超过重启次数后锁定 */
						if(user_list.Load_exist<30000)
						{
							user_list.Load_exist++;
						}
						if(user_list.Load_exist<RESTAR_COUNT)
						{
							/* 清除堵转标志，允许重启 */
							mc_core_list.MC_error.bits.gBlock=0;
							machine_error_hold.MC_error_hold.bits.gBlock=0;
							mc_core_list.block_count=0;
							mc_core_list.block_count1=0;
							mc_core_list.block_count2=0;
							mc_core_list.block_count3=0;
							mc_core_list.block_count4=0;
							mc_core_list.block_count_runing=0;
							motor_control_list.bldc_state=MOTOR_STOP;
						}
						else
						{
							/* 超过重启次数，锁定 */
							motor_control_list.bldc_state=MOTOR_STOP;
						}
					}
					#else
					motor_control_list.bldc_state=MOTOR_STOP;
					#endif
				}
				else if(motor_control_list.sbrake_en)
				{
					#if BREAKE_WAIT_EN
					motor_control_list.bldc_state=MOTOR_BRAKE_WAIT;
					wait_break_count=0;
					#else
					motor_control_list.bldc_state=MOTOR_SBRAKE;
					#endif
					sbreak_count=0;
				}
				else if(motor_control_list.hbrake_en)
				{
					#if BREAKE_WAIT_EN
					motor_control_list.bldc_state=MOTOR_BRAKE_WAIT;
					wait_break_count=0;
					#else
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					#endif
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
			}
			break;

		/*==================== 制动到停止过渡状态 ====================*/
		case MOTOR_BRAKE2STOP:
			Stop_Motor();
			break2stop_count++;
			/* 10ms后进入停止状态 */
			if(break2stop_count>160)
			{
				motor_control_list.bldc_state=MOTOR_STOP;
			}
			break;

		/*==================== 制动等待状态 ====================*/
		case MOTOR_BRAKE_WAIT:
			mc_core_list.pwm_actual_runing_data=0;
			motor_control_list.pwm_duty_virtual=0;
			Stop_Motor();
			wait_break_count++;

			/* 有故障直接转入制动到停止 */
			if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 扳机再触发且方向不变，直接制动到停止（重启） */
			else if(mc_core_list.motion_en&&mc_core_list.motor_en&&(motor_control_list.motor_run_last_dir==mc_core_list.mc_dir))
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 等待时间到达，转入制动 */
			else if(wait_break_count>=WAIT_Brake_Time_MS_CNT)
			{
				if(motor_control_list.sbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_SBRAKE;
					sbreak_count=0;
				}
				else if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_STOP;
				}
			}
			break;

		/*==================== 硬制动状态（短接绕组） ====================*/
		case MOTOR_HBRAKE:
			mc_core_list.pwm_actual_runing_data=0;
			motor_control_list.pwm_duty_virtual=0;
			Brake_Motor();
			hbreak_count++;

			/* 有故障直接转入制动到停止 */
			if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 扳机再触发且方向不变，直接重启 */
			else if(mc_core_list.motion_en&&mc_core_list.motor_en&&(motor_control_list.motor_run_last_dir==mc_core_list.mc_dir))
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 最小制动时间到达且扳机再触发，允许重启 */
			else if(hbreak_count>HBrake_MIN_Time_MS_CNT&&mc_core_list.motor_en&&mc_core_list.motion_en)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 最大制动时间到达或OCP触发，转入制动到停止 */
			else if(hbreak_count>HBrake_Time_MS_CNT||machine_error_hold.userErr_hold.bits.gIBusAcmpOCP)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			break;

		/*==================== 软制动状态（PWM制动） ====================*/
		case MOTOR_SBRAKE:
			if(sbreak_count==0)
			{
				/* 初始化制动占空比为最小值 */
				motor_control_list.sbreak_pwm_duty=Sbrake_Duty_Min;
			}
			sbreak_count++;
			PwmDutyUpdate(motor_control_list.sbreak_pwm_duty);

			/* 逐步增加制动占软到最大值 */
			motor_control_list.sbreak_pwm_duty+=Sbrake_Duty_Add;
			if(motor_control_list.sbreak_pwm_duty>Sbrake_Duty_Max)
			{
				motor_control_list.sbreak_pwm_duty=Sbrake_Duty_Max;
			}

			/* 下桥臂PWM制动 */
			UVWL_HOffLPwm();

			/* 有故障直接转入制动到停止 */
			if(machine_error_hold.userErr_hold.word!=0||machine_error_hold.MC_error_hold.word!=0)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 扳机再触发且方向不变，直接重启 */
			else if(mc_core_list.motion_en&&mc_core_list.motor_en&&(motor_control_list.motor_run_last_dir==mc_core_list.mc_dir))
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 扳机再触发且超过最短制动时间，转入硬制动或停止 */
			else if(mc_core_list.motion_en&&mc_core_list.motor_en&&sbreak_count>SBrake_MIN_Time_MS_CNT)
			{
				if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
					break2stop_count=0;
					Stop_Motor();
				}
			}
			/* OCP触发，转入制动到停止 */
			else if(machine_error_hold.userErr_hold.bits.gIBusAcmpOCP)
			{
				motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
				break2stop_count=0;
				Stop_Motor();
			}
			/* 软制动时间到达，转入硬制动或停止 */
			else if(sbreak_count>Sbrake_Time_MS)
			{
				if(motor_control_list.hbrake_en)
				{
					motor_control_list.bldc_state=MOTOR_HBRAKE;
					hbreak_count=0;
				}
				else
				{
					motor_control_list.bldc_state=MOTOR_BRAKE2STOP;
					break2stop_count=0;
					Stop_Motor();
				}
			}
			break;

		default:
			break;
	}
}

/******************************************************************************
* 函数名: phase_time_filter
* 描  述: 60度电周期时间滤波
*        使用一阶低通滤波算法：y(n) = 0.75*x(n) + 0.25*y(n-1)
*        通过定点数实现：9830/32768≈0.3, 22938/32768≈0.7
*        用于平滑速度测量，减少换相时间波动
******************************************************************************/
void phase_time_filter(void)
{
	motor_control_list.time_count_60_degree_filter=((mc_core_list.time_count_60_degree*9830)>>15)+((motor_control_list.time_count_60_degree_filter*22938)>>15);
}

/******************************************************************************
* 函数名: get_speed
* 描  述: 计算电机转速（RPM）
*        转速 = 定时器频率 / 60度电周期时间
*        即 75000000 / time_count_60_degree_filter
*        仅在闭环运行且检测到BEMF过零时计算
******************************************************************************/
void get_speed(void)
{
	if(motor_control_list.bldc_state==MOTOR_RUN&&mc_core_list.bldc_bemf_detected==1)
	{
		phase_time_filter();
		/* 75MHz定时器频率除以60度电周期时间得到转速 */
		motor_control_list.motor_speed=(u32)(75000000.0f/((float)motor_control_list.time_count_60_degree_filter));
	}
	else
	{
		motor_control_list.time_count_60_degree_filter=0;
		motor_control_list.motor_speed=0;
	}
}

#if LIMIT_SPEED_EN
/******************************************************************************
* 函数名: Speed_PI_init
* 描  述: 速度PI调节器初始化
*        设置比例增益ki=kp，参考转速，输出限幅范围
******************************************************************************/
void  Speed_PI_init(void)
{
	Speed_Pid.ki=(u32)(32768*0.002f);
	Speed_Pid.kp2=(u32)(32768*2.0f);
	Speed_Pid.kp=(u32)(32768*0.002f);
	Speed_Pid.refValue=48000;  /* 参考转速 */
	Speed_Pid.Limit_max_data=PWM_PRIOD_LOAD;  /* 最大占空比 */
	Speed_Pid.Limit_min_data=(PWM_PRIOD_LOAD*0.15f);  /* 最小占空比 */
}

/******************************************************************************
* 函数名: Speed_PI_reset
* 描  述: 速度PI调节器复位，清零所有状态变量
******************************************************************************/
void  Speed_PI_reset(void)
{
	Speed_Pid.error = 0;
	Speed_Pid.error1 = 0;
	Speed_Pid.error2 = 0;
	Speed_Pid.output = 0;
	Speed_Pid.pTemp = 0;
	Speed_Pid.iTemp = 0;
	Speed_Pid.deltaUk = 0;
	Speed_Pid.deltaOut = 0;
	Speed_Pid.rpmSetPwmDuty = 0;
	Speed_Pid.errorIntegral= 0;
	Speed_Pid.KpDuty = 0;
	Speed_Pid.KiDuty = 0;
	Speed_Pid.OutDuty= 0;
}

/******************************************************************************
* 函数名: MC_PI_Handler
* 描  述: 增量式PI调节器
*        使用增量式算法：ΔU(k) = Kp*(e(k)-e(k-1)) + Ki*e(k)
*        带积分抗饱和：输出饱和时仅在误差方向有利于退出饱和时积分
* 参  数: pi - PI调节器指针  errorBack - 反馈值（实际转速）
* 返回值: PI输出占空比值
******************************************************************************/
int32_t MC_PI_Handler( PidPI_t *pi,int32_t errorBack)
{
	/* 计算误差：目标值 - 反馈值 */
	pi->error = (pi->refValue - errorBack)>>0;

	/* 误差限幅 */
	if(pi->error > 32768)
	{
		pi->error = 32768;
	}
	else if(pi->error <(- 32768))
	{
		pi->error = -32768;
	}

	/* 比例项：Kp*(e(k) - e(k-1)) */
	pi->pTemp = pi->kp*(pi->error-pi->error1);

	/* 积分项：带抗饱和处理 */
	pi->iTemp = 0;

	if(pi->OutDuty >=pi->Limit_max_data)
	{
		/* 输出饱和于上限：仅当误差为负（有利于退出饱和）时积分 */
		if( pi->error < 0)
		pi->iTemp = pi->ki*(pi->error);
	}
	else if(pi->OutDuty <= pi->Limit_min_data)
	{
		/* 输出饱和于下限：仅当误差为正（有利于退出饱和）时积分 */
		if( pi->error > 0)
		pi->iTemp = pi->ki*(pi->error);
	}
	else
	{
		pi->iTemp = pi->ki*(pi->error);
	}

	/* 增量式输出：U(k) = U(k-1) + ΔU(k) */
	pi->output += (pi->pTemp + pi->iTemp);

	/* 输出积分限幅 */
	if(pi->output > (pi->Limit_max_data<<15))
	{
		pi->output = (pi->Limit_max_data<<15);
	}
	else if(pi->output < (pi->Limit_min_data<<15))
	{
		pi->output = ((pi->Limit_min_data<<15));
	}

	/* 右移15位得到实际占空比 */
	pi->OutDuty = pi->output>>15;

	/* 占空比限幅 */
	if(pi->OutDuty>pi->Limit_max_data)
	{
		pi->OutDuty = pi->Limit_max_data;
	}
	else if(pi->OutDuty < pi->Limit_min_data)
	{
		pi->OutDuty = pi->Limit_min_data;
	}

	pi->rpmSetPwmDuty = pi->OutDuty;

	/* 更新历史误差 */
	pi->error2 = pi->error1;
	pi->error1 = pi->error;

	return pi->rpmSetPwmDuty;
}

#endif
