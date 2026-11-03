#include "main.h"
#include "board.h"
#include "user_control.h"
#include "parameter.h"
#include "motor_control.h"
#include "led.h"
User_list_Struct user_list;
Machine_ERROR_HOLD machine_error_hold;
Mc_core_list mc_core_list;
Motor_Control_list motor_control_list;
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
#include "D:/mym/NXN/9.henbangtai/dianzuan/core/NX32F002-V1_01-lib-release/tests/seven_led/test_seven_led.c"