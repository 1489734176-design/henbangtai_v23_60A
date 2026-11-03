#ifndef __LED_H
#define __LED_H
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/*
 * 全板只保留一只灯（LIGHT），由 LED+ 网络经 Q10(2N7002) 驱动。
 * 该灯兼作故障提示灯与照明灯：有故障时闪故障码，无故障时按照明规则亮灭。
 * PA10 输出高电平使 Q10 导通，点亮灯。
 *
 * 原 6 灯复用矩阵（LED_Scan / LedSoc_Run / COM_PIN1~4）已随硬件取消，
 * LED_SCAN_EN 在 parameter.h 中置 0。
 */
#define LED_LIGHT_RCC_CLOCKGPIO     (RCC_AHBENR_GPIOA_Msk)
#define LED_LIGHT_PORT              GPIOA
#define LED_LIGHT_PIN               GPIO_Pin_10

#define LED_ERROR_ON()              GPIO_SetBits(LED_LIGHT_PORT, LED_LIGHT_PIN)
#define LED_ERROR_OFF()             GPIO_ResetBits(LED_LIGHT_PORT, LED_LIGHT_PIN)

/*
 * 故障编号 = LED 闪烁次数。
 * 1/3/4/5/6/7/8/10/11 为功能需求表第 23 项规定；
 * 2、9 需求表留空，其中 2 沿用软件原有的"电池温度异常"编号；
 * 需求表未列但软件已有的其余故障，按顺序排在 11 之后。
 */
#define LED_FAULT_BLOCK             (1)     /* 堵转保护 */
#define LED_FAULT_BAT_TEMP          (2)     /* 电池包温度异常（需求表未列） */
#define LED_FAULT_UVP               (3)     /* 欠压保护 */
#define LED_FAULT_MOS_TEMP          (4)     /* MOS 管温度保护 */
#define LED_FAULT_IBUS_OCP1         (5)     /* 一级母线过流保护 */
#define LED_FAULT_IBUS_OCP2         (6)     /* 二级母线过流保护 */
#define LED_FAULT_PHASE_OCP_AVG     (7)     /* 相电流过流保护（软件峰值/平均） */
#define LED_FAULT_PHASE_OCP_HW      (8)     /* 相电流过流保护（硬件比较器） */
                                            /* 9 预留 */
#define LED_FAULT_OVP               (10)    /* 过压保护 */
#define LED_FAULT_HW_OCP            (11)    /* 硬件过流（短路电流）保护 */
#define LED_FAULT_LACK_PHASE        (12)    /* 缺相（需求表未列） */
#define LED_FAULT_LOSE_STEP         (13)    /* 失步（需求表未列） */
#define LED_FAULT_FLY_MACHINE       (14)    /* 上电飞钻（需求表未列） */
#define LED_FAULT_BAT_COM           (15)    /* 电池通信故障（需求表未列） */
#define LED_FAULT_MOS_BRIDGE        (16)    /* 驱动 MOS 桥臂异常（需求表未列） */
#define LED_FAULT_OTHER             (17)    /* 其他未分类故障 */

/* LED 故障显示控制 */
typedef struct
{
    uint8_t     LedDeal;        /* 单次闪烁触发请求 */
    uint8_t     LedRunMode;     /* 闪烁模式 0:两段亮灭 1:单段亮灭 */
    uint16_t    LedRunTimes;    /* 同一故障码还需重复显示的轮数 */
    uint16_t    LedRunCnt_Once; /* 当前故障的闪烁次数（即故障编号） */
} User_LED_Control_t;

/* 照明（LIGHT）状态机 */
typedef enum
{
    LIGHT_OFF = 0,
    LIGHT_ON,
    LIGHT_WAIT_10S,
    LIGHT_WAIT_30S,
} Light_State_e;

typedef struct
{
    Light_State_e   state;
    uint32_t        cnt;        /* 1ms 计数 */
} Light_Ctrl_t;

extern User_LED_Control_t user_led_control;
extern Light_Ctrl_t       light_ctrl;

void    led_init(void);
void    UO_Led_Handler(void);
uint8_t LedError_Control(void);
void    LedError_Run(void);
void    Light_Handler(void);

#endif
