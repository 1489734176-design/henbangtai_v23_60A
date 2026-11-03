#ifndef __USER_SUB_H
#define __USER_SUB_H
#include "main.h"
#include "my_mc.h"
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
		struct
		{
			unsigned OverVBUSFlag 		:1;			//Over voltage			
			unsigned UnderVBUSFlag 		:1;	 		//Under voltage			
			unsigned OverTempFlag 		:1;	 		//Over temperature	
			unsigned OverIBUSFlag 		:1;			//Over current			
			unsigned LackPhaseFlag 		:1;			
			unsigned LockedFlag 			:1;			//Rotor Locked
			unsigned BrakeFlag 				:1;			//Hardware fault
			unsigned OffsetFlag 			:1;			//Hardware fault
		}bit;
		uint8_t Byte;
} DiagFlag;

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

typedef union
{
    uint32_t word;
    struct
    {
        uint32_t 		gIBusAcmpOCP:1;
        uint32_t 		gIBusPeakOCP1:1;
        uint32_t 		gIBusAvgOCP1:1;	    
        uint32_t 		gIBusAvgOCP2:1;	
        
        uint32_t 		gIBusAvgOCP3:1;
        uint32_t 		gVBusRunUVP1:1;    // 运行欠压1	    
        uint32_t 		gVBusRunUVP2:1;		
        uint32_t 		gVBusStbUVP:1;
        
        uint32_t 		gVBusOVP:1;	
        uint32_t 		gLVD:1;
        uint32_t 		gNtcMos:1;
        uint32_t 		gNtcBat:1;
        
        uint32_t 		gIBusOffset:1;
        uint32_t 		gMosBridgeH:1;
        uint32_t 		gMosBridgeL:1;
        uint32_t 		gWDTOver:1;
        uint32_t 		gHardFault:1;
			
				uint32_t 		fly_machine:1;

    }bits;
}UC_Error_u;    

//typedef struct 
//{
//    UC_Error_u          userErr;   
//}User_Error_t;






typedef struct
{
	UC_StateMachine_e user_state;
	UC_Error_u          userErr; 
	uint16_t  IBusAvgOCP1_Adc;
	
	uint8_t   u8_1ms_flag;    // 1ms时基标志
	UC_Flag_u  flag;          // 工具标志位   

} User_list_Struct;

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
extern DiagFlag DiagnoseFAULT;
#endif


