#include "test_hw.h"

#include "parameter.h"

#include "motor_config.h"

#include "D:/mym/NXN/lanhui/dianzuan/core/NX32F002-V1_01-lib-release/USER/Bat_com.c"

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
			if(++user_list.sleep_cnt>SLEEP_TIME && light_ctrl.state==LIGHT_OFF)  // 休眠超时，进入低功耗
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
			else if(user_list.flag.bits.gToolTrigger && user_list.direction_ready && Bat_Com_CanRun())
			{
				user_list.flag.bits.gToolEn=1;
				user_list.user_state=TOOL_RUN;
				user_list.sleep_cnt=0;
			}
		break;
			
		case TOOL_ERROR_STOP:
			user_list.flag.bits.gToolEn=0;
//			user_list.Load_exist=0;
			if(++user_list.sleep_cnt>SLEEP_TIME && light_ctrl.state==LIGHT_OFF)
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
			/* 运行中方向变为无效（两引脚同时生效或同时不生效）立即停机 */
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

void user_direction_handle(void)
{
	
	static u8 dir_fwd_count=0;
	static u8 dir_rwd_count=0;
	static u8 dir_invalid_count=0;
	#if(1)
//	if(GPIO_ReadInputDataBit(DIR_PORT,DIR_PIN)) 

/*
	if(user_list.Dir_Data) 
	{
			dir_ccw_count=0;
			if(++dir_cw_count>10)
			{
				dir_cw_count=10;
				if(motor_control_list.bldc_state==MOTOR_STOP||motor_control_list.bldc_state==MOTOR_HBRAKE||motor_control_list.bldc_state==MOTOR_SBRAKE||motor_control_list.bldc_state==MOTOR_BRAKE2STOP)
				{
					user_list.flag.bits.gToolDir=Tool_Dir_CCW_Value;
					mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
				}
				
			}						

	}
	else 
	{
			dir_cw_count=0;
			if(++dir_ccw_count>10)
			{
				dir_ccw_count=10;
				if(motor_control_list.bldc_state==MOTOR_STOP||motor_control_list.bldc_state==MOTOR_HBRAKE||motor_control_list.bldc_state==MOTOR_SBRAKE||motor_control_list.bldc_state==MOTOR_BRAKE2STOP)
				{
					user_list.flag.bits.gToolDir=Tool_Dir_CW_Value;
					mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
				}
			
			}	
	}
*/
	u16 dir_inputs = GPIO_ReadInputData(DIR_PORT) & (DIR_PIN | RWD_PIN);
	/* 恰好一个方向引脚生效（一低一高）才算有效方向 */
	u8 single = (dir_inputs == RWD_PIN) || (dir_inputs == DIR_PIN);
	user_list.direction_ready=0;

	/* 两个方向引脚同时生效或同时不生效：方向无效。
	   去抖后清除 dir_valid，并置重扣锁定——退出无效状态后必须先松开电位器
	   再按下才能启动。此判断在运行早退出之前执行，故运行途中变无效也能被捕获。 */
	if(!single)
	{
		if(dir_invalid_count < 10)
		{
			dir_invalid_count++;
		}
		if(dir_invalid_count >= 10)
		{
			user_list.dir_valid=0;
		}
		user_list.dir_retrig_need=1;
		dir_fwd_count=0;
		dir_rwd_count=0;
		return;
	}
	dir_invalid_count=0;

	/* 单一方向有效时，电位器（扳机）松开即解除重扣锁定 */
	if(user_list.flag.bits.gToolTrigger==0)
	{
		user_list.dir_retrig_need=0;
	}

	/* 制动或启动请求尚未撤销时，不得修改换相方向。 */
	if(motor_control_list.bldc_state != MOTOR_STOP ||
	   mc_core_list.motor_en || user_list.flag.bits.gToolEn)
	{
		dir_fwd_count=0;
		dir_rwd_count=0;
		return;
	}

	if(dir_inputs == RWD_PIN)
	{
		dir_rwd_count=0;
		if(dir_fwd_count < 10)
		{
			dir_fwd_count++;
		}
		if(dir_fwd_count == 10)
		{
			user_list.flag.bits.gToolDir=Tool_Dir_CW_Value;
			mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
			user_list.dir_valid=1;
			/* 仅在不需要重扣时才置就绪，实现“松开电位器再按下”方可启动 */
			if(!user_list.dir_retrig_need)
			{
				user_list.direction_ready=1;
			}
		}
	}
	else if(dir_inputs == DIR_PIN)
	{
		dir_fwd_count=0;
		if(dir_rwd_count < 10)
		{
			dir_rwd_count++;
		}
		if(dir_rwd_count == 10)
		{
			user_list.flag.bits.gToolDir=Tool_Dir_CCW_Value;
			mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
			user_list.dir_valid=1;
			if(!user_list.dir_retrig_need)
			{
				user_list.direction_ready=1;
			}
		}
	}
	else
	{
		dir_fwd_count=0;
		dir_rwd_count=0;
	}
	#endif
	
	#if(0)
	user_list.dir_adc=(uint16_t)GET_ADC_VALUE(DIR_CHANNEL);
//	mc_core_list.mc_dir=1;
		if(user_list.dir_adc>3600) 
	{
			dir_ccw_count=0;
			if(++dir_cw_count>10)
			{
				dir_cw_count=10;
				if(motor_control_list.bldc_state==MOTOR_STOP||motor_control_list.bldc_state==MOTOR_HBRAKE||motor_control_list.bldc_state==MOTOR_SBRAKE||motor_control_list.bldc_state==MOTOR_BRAKE2STOP)
				{
					user_list.flag.bits.gToolDir=Tool_Dir_CCW_Value;
					mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
				}
			}
			
			

	}
	else if(user_list.dir_adc<3600&&user_list.dir_adc>2000)
	{
			dir_cw_count=0;
			if(++dir_ccw_count>10)
			{
				dir_ccw_count=10;
				if(motor_control_list.bldc_state==MOTOR_STOP||motor_control_list.bldc_state==MOTOR_HBRAKE||motor_control_list.bldc_state==MOTOR_SBRAKE||motor_control_list.bldc_state==MOTOR_BRAKE2STOP)
				{
					user_list.flag.bits.gToolDir=Tool_Dir_CW_Value;
					mc_core_list.mc_dir=user_list.flag.bits.gToolDir;
				
				}
				
			}
	
	}
	#endif
}

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

void run_main_enable_ipd(void)
{
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

#include "D:/mym/NXN/lanhui/dianzuan/core/NX32F002-V1_01-lib-release/tests/battery_com/test_battery_com.c"