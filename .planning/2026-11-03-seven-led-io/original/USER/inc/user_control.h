#ifndef __USER_CONTROL_H
#define __USER_CONTROL_H
#include "stdint.h"
#include "mm32spin0230.h"
#include "mc_core.h"

//电机是否允许转状态变量
typedef enum 
{
    TOOL_STOP = 0,
    TOOL_RUN = 1,
    TOOL_ERROR_STOP = 2,
    TOOL_POWER_DOWN = 3,
    TOOL_STOPPING = 4,
    TOOL_PRESTOP = 5,
    TOOL_READY = 6,
}UC_StateMachine_e;

typedef union
{
    uint32_t word;
    struct
    {
        uint32_t    gToolEn:1;
        uint32_t    gToolDir:1;
		uint32_t    gToolTrigger:1;
		uint32_t    gAutoStopEn;
		
		uint32_t    gAutoStop;
    }bits;
}UC_Flag_u;

//异常报错结构体
typedef union
{
    uint32_t word;
    struct
    {
        uint32_t	gIBusAcmpOCP:1;
        uint32_t	gIBusPeakOCP1:1;
        uint32_t	gIBusAvgOCP1:1;	    
        uint32_t	gIBusAvgOCP2:1;	
        
        uint32_t	gIBusAvgOCP3:1;
		uint32_t	gIBusAvgOCP4:1;
        uint32_t	gVBusRunUVP1:1;	    
        uint32_t	gVBusRunUVP2:1;		
        uint32_t	gVBusStbUVP:1;
        
        uint32_t	gVBusOVP:1;	
        uint32_t	gLVD:1;
        uint32_t	gNtcMos:1;
        uint32_t	gNtcBat:1;
					
        uint32_t	gIBusOffset:1;
        uint32_t	gMosBridgeH:1;
        uint32_t	gMosBridgeL:1;
        uint32_t	gWDTOver:1;
        uint32_t	gHardFault:1;
					
		uint32_t	fly_machine:1;
		uint32_t	gMos_overload:1;
		uint32_t	gBatComFault:1;		
    }bits;
}UC_Error_u;    

//typedef struct 
//{
//    UC_Error_u	userErr;   
//}User_Error_t;





//用户状态结构体
typedef struct
{
	UC_StateMachine_e	user_state;	//工具是否允许转
	UC_Error_u			userErr; 	//异常报错结构体

	uint8_t		Key_Data;       // 按键数据
	uint8_t		Dir_Data;       // 方向数据
	uint16_t	Tmos_Adc;       // MOS温度ADC值
	uint16_t	Tbat_Adc;       // 电池温度ADC值
	uint16_t	Vbus_Adc;       // 母线电压ADC值
	uint16_t	IBusAvg1_Adc;   // 平均电流1 ADC值
	uint16_t	IBusAvg_Adc;    // 平均电流ADC值
	uint16_t	IBusPeak_Adc;   // 峰值电流ADC值
	uint8_t		u8_1ms_flag;    // 1ms时基标志
	uint32_t 	VBus_Vx100;     // 母线电压（单位0.01V）
	uint16_t	dir_adc;        // 方向ADC值
	uint16_t	key_adc;        // 按键ADC值
	uint8_t 	gears_level;    // 挡位等级
	uint8_t 	gears_ccw_level; // 反转挡位等级
	uint32_t 	sleep_cnt;     // 休眠计数
	uint16_t 	CCW_Auto_Mask_count; // 反转自动停机屏蔽计数
	UC_Flag_u	flag;          // 工具标志位   
	uint8_t		En_pin_dir;    // 使能引脚方向
	uint8_t		led_reuse_adc_flag; // LED复用ADC标志
	uint8_t 	led_reuse_io_flag;  // LED复用IO标志
	uint8_t 	led_scan_start_flag; // LED扫描启动标志
	uint16_t 	Load_exist;    // 负载存在标志（用于重启判断）
	uint32_t 	Vcc_cal_gain;  // VCC校准增益
	uint8_t 	gears_level_changed; // 挡位变化标志
	uint16_t 	dir_level_changed_count;
	uint8_t		direction_ready;
	uint8_t		dir_valid;        // 方向有效：恰好一个方向引脚生效（两同高/同低为无效）
	uint8_t		dir_retrig_need;  // 需重新扣动电位器才能启动：松开电位器后清除
} User_list_Struct;

//故障锁存
typedef struct
{
	UC_Error_u  userErr_hold;
	MC_Error_u  MC_error_hold;

}Machine_ERROR_HOLD;


extern User_list_Struct user_list;
extern Machine_ERROR_HOLD machine_error_hold;
void user_direction_handle(void);
void motor_on_off_control(void);
void user_state_control(void);
void user_oc_handle(void);

void error_hold_updata(void);
void User_Temperature_Handler_Fast(void);
void User_Temperature_Handler(void);
void Volt_Handler_fast(void);
void Volt_Handler(void);
void gears_deal(void);
void pwm_duty_aim_deal(void);
void user_WrenchKey_Handler(void);
void user_para_init(void);
void Vcc_5V_CAL(void);
void user_gears_handle(void);

//extern DiagFlag DiagnoseFAULT;

#endif

