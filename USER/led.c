#include "board.h"
#include "led.h"

#include "user_control.h"
#include "parameter.h"

User_LED_Control_t user_led_control;
Light_Ctrl_t       light_ctrl;

/** 照明延时（单位 ms，1ms 任务调用一次） */
#define LIGHT_DELAY_NORMAL_MS       (10000u)    /* 无故障松开开关：延时 10s 熄灭 */
#define LIGHT_DELAY_FAULT_MS        (30000u)    /* 故障时不松开开关：延时 30s 熄灭 */

/**
 * @brief  灯的初始化
 *        单灯由 LED+ 网络经 Q10 驱动，推挽输出，高电平点亮
 */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHBPeriphClockCmd(LED_LIGHT_RCC_CLOCKGPIO, ENABLE);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = LED_LIGHT_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LED_LIGHT_PORT, &GPIO_InitStruct);

    LED_ERROR_OFF();                    /* 上电默认熄灭 */
    light_ctrl.state = LIGHT_OFF;
    light_ctrl.cnt   = 0;
}

/**
 * @brief  判断是否需要进入照明状态机
 * @retval 1: 有故障锁存 0: 无故障
 */
static uint8_t Light_Fault_Active(void)
{
    return (uint8_t)((machine_error_hold.userErr_hold.word != 0) ||
                     (machine_error_hold.MC_error_hold.word != 0));
}

/**
 * @brief  照明功能状态机
 *        故障时松开开关  -> 立即熄灭
 *        无故障时松开开关-> 延时 10s 熄灭
 *        故障时不松开开关-> 延时 30s 熄灭
 *        无故障且按住    -> 点亮
 */
void Light_Handler(void)
{
    uint8_t fault = Light_Fault_Active();
    uint8_t trig  = user_list.flag.bits.gToolTrigger;

    if(user_list.user_state == TOOL_POWER_DOWN)
    {
        light_ctrl.state = LIGHT_OFF;
        light_ctrl.cnt   = 0;
        return;
    }

    if(trig)
    {
        if(fault)
        {
            if(light_ctrl.state != LIGHT_WAIT_30S)
            {
                light_ctrl.state = LIGHT_WAIT_30S;
                light_ctrl.cnt   = 0;
            }
            if(light_ctrl.cnt < LIGHT_DELAY_FAULT_MS)
            {
                light_ctrl.cnt++;
            }
        }
        else
        {
            light_ctrl.state = LIGHT_ON;
            light_ctrl.cnt   = 0;
        }
    }
    else if(fault || light_ctrl.state == LIGHT_WAIT_30S)
    {
        /* 松手时故障锁存可能已清除，仍须立即结束故障显示。 */
        light_ctrl.state = LIGHT_OFF;
        light_ctrl.cnt   = 0;
    }
    else
    {
        if(light_ctrl.state == LIGHT_ON)
        {
            light_ctrl.state = LIGHT_WAIT_10S;
            light_ctrl.cnt   = 0;
        }
        if(light_ctrl.state == LIGHT_WAIT_10S)
        {
            if(++light_ctrl.cnt >= LIGHT_DELAY_NORMAL_MS)
            {
                light_ctrl.state = LIGHT_OFF;
            }
        }
    }
}

/**
 * @brief  LED 显示调度
 *        故障闪烁优先于照明；按住开关时闪故障码，松开且有故障时立即熄灭
 */
void UO_Led_Handler(void)
{
    Light_Handler();

    if(light_ctrl.state == LIGHT_WAIT_30S &&
       light_ctrl.cnt < LIGHT_DELAY_FAULT_MS && LedError_Control())
    {
        LedError_Run();
    }
    else
    {
        user_led_control.LedDeal        = 0;
        user_led_control.LedRunTimes    = 0;
        user_led_control.LedRunCnt_Once = 0;
        if(light_ctrl.state == LIGHT_ON || light_ctrl.state == LIGHT_WAIT_10S)
        {
            LED_ERROR_ON();
        }
        else
        {
            LED_ERROR_OFF();
        }
    }
}

/**
 * @brief  故障编号判定：把锁存故障翻译成 LED 闪烁次数（见 led.h 编号表）
 * @retval 1: 有待显示的故障 0: 无故障
 */
uint8_t LedError_Control(void)
{
    uint8_t ret = 0;

    #if(RESTAR_EN)
    /* 堵转在重启次数未用尽时不算故障 */
    if((machine_error_hold.userErr_hold.word == 0) &&
       ((machine_error_hold.MC_error_hold.word & 0xfffb) == 0) &&
       (machine_error_hold.MC_error_hold.bits.gBlock == 0 || (user_list.Load_exist < RESTAR_COUNT)))
    #else
    if((machine_error_hold.userErr_hold.word == 0) && ((machine_error_hold.MC_error_hold.word) == 0))
    #endif
    {
        user_led_control.LedRunCnt_Once = 0;
        user_led_control.LedDeal        = 0;
        user_led_control.LedRunTimes    = 0;
        return 0;
    }

    if(machine_error_hold.userErr_hold.word || machine_error_hold.MC_error_hold.word)
    {
        if((0 == user_led_control.LedDeal) && (!user_led_control.LedRunTimes))
        {
            user_led_control.LedDeal     = 1;
            user_led_control.LedRunTimes = 10;

            if(machine_error_hold.MC_error_hold.bits.gBlock)                                            /* 1  堵转保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_BLOCK;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gVBusStbUVP ||
                    machine_error_hold.userErr_hold.bits.gVBusRunUVP1 ||
                    machine_error_hold.userErr_hold.bits.gVBusRunUVP2)                                  /* 3  欠压保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_UVP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gNtcMos ||
                    machine_error_hold.userErr_hold.bits.gMos_overload)                                 /* 4  MOS 管温度保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_MOS_TEMP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gIBusAvgOCP1)                                  /* 5  一级母线过流保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_IBUS_OCP1;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gIBusAvgOCP2)                                  /* 6  二级母线过流保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_IBUS_OCP2;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gIBusPeakOCP1)                                 /* 7  相电流过流保护（峰值） */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_PHASE_OCP_AVG;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gIBusAvgOCP3 ||
                    machine_error_hold.userErr_hold.bits.gIBusAvgOCP4)                                  /* 8  相电流过流保护（平均） */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_PHASE_OCP_HW;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gVBusOVP)                                      /* 10 过压保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_OVP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gIBusAcmpOCP)                                  /* 11 硬件过流（短路电流）保护 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_HW_OCP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gNtcBat)                                       /* 2  电池包温度异常 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_BAT_TEMP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.MC_error_hold.bits.gLackPhase)                                   /* 1 缺相 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_BLOCK;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.MC_error_hold.bits.gLosStep)                                     /* 13 失步 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_LOSE_STEP;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.fly_machine)                                   /* 14 上电飞钻 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_FLY_MACHINE;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gBatComFault)                                  /* 15 电池通信故障 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_BAT_COM;
                user_led_control.LedRunMode     = 1;
            }
            else if(machine_error_hold.userErr_hold.bits.gMosBridgeH ||
                    machine_error_hold.userErr_hold.bits.gMosBridgeL)                                   /* 16 桥臂异常 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_MOS_BRIDGE;
                user_led_control.LedRunMode     = 1;
            }
            else                                                                                        /* 17 其他 */
            {
                user_led_control.LedRunCnt_Once = LED_FAULT_OTHER;
                user_led_control.LedRunMode     = 1;
            }
        }

        ret = user_led_control.LedRunCnt_Once ? 1 : 0;
    }

    return ret;
}

/**
 * @brief  故障码闪烁执行
 *         节奏保持原实现：灭 500ms / 亮 200ms 为一段，段间连闪，
 *         一轮结束后间隔 1500ms 再重复，共重复 LedRunTimes 轮
 */
void LedError_Run(void)
{
    static uint16_t bzCnt  = 0;     /* 当前剩余闪烁段数 */
    static uint16_t bzTime = 0;     /* 段内计时 */

    if(user_led_control.LedDeal)
    {
        LED_ERROR_OFF();

        bzCnt  = user_led_control.LedRunCnt_Once;
        bzTime = 0;

        user_led_control.LedDeal = 0;
    }

    if(!user_led_control.LedRunTimes)
    {
        LED_ERROR_OFF();

        bzCnt  = 0;
        bzTime = 0;
        return;
    }

    if(bzCnt)
    {
        bzTime++;

        if((bzTime >= 1) && (bzTime <= 500))        /* 灭 500ms */
        {
            LED_ERROR_OFF();
        }
        else if(bzTime < 700)                       /* 亮 200ms */
        {
            LED_ERROR_ON();
        }

        if(user_led_control.LedRunMode == 0)
        {
            if((bzTime >= 700) && (bzTime <= 900))
            {
                LED_ERROR_OFF();
            }
            else if((bzTime >= 900) && (bzTime <= 1100))
            {
                LED_ERROR_ON();
            }

            if(bzTime > 1100)
            {
                bzTime = 0;
                bzCnt--;
            }
        }
        else
        {
            if(bzTime > 700)
            {
                bzTime = 0;
                bzCnt--;
            }
        }
    }
    else
    {
        LED_ERROR_OFF();

        if(bzTime++ >= 1500)                        /* 轮间隔 1500ms */
        {
            bzTime = 0;
            bzCnt  = user_led_control.LedRunCnt_Once;
            user_led_control.LedRunTimes--;
        }
    }
}
