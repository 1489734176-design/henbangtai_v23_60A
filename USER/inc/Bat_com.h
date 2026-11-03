/**
 * @file     Bat_com.h
 * @brief    DAYE A1.1 电池包单线半双工通信接口（工具侧）
 *           定义协议常量、BMS/工具状态位、电池数据结构及对外函数。
 */
#ifndef __BAT_COM_H
#define __BAT_COM_H

#include <string.h>
#include "mm32_device.h"
#include "hal_conf.h"

#define BAT_COM_EN                     (1)      /* 电池通信功能总开关，0 时许可恒为通过 */
#define BAT_CMD_HANDSHAKE              (0xA5u)  /* 握手命令字 */
#define BAT_CMD_RUNTIME                (0xD1u)  /* 运行数据命令字 */
#define BAT_CMD_REHANDSHAKE            (0xFFu)  /* 未握手时回复请求重握手 */
#define BAT_DEV_TOOL                   (0xD1u)  /* 设备类型：工具 */
#define BAT_CRC_POLY                   (0x25u)  /* CRC8 多项式 */

#define BAT_FRAME_LEN_HS_MASTER        (12u)    /* 主机握手帧长度 */
#define BAT_FRAME_LEN_RT_MASTER        (12u)    /* 主机运行帧长度 */
#define BAT_FRAME_LEN_HS_SLAVE         (10u)    /* 工具握手应答长度 */
#define BAT_FRAME_LEN_RT_SLAVE         (8u)     /* 工具运行应答长度 */
#define BAT_FRAME_BUF_SIZE             (16u)    /* 收发缓冲/邮箱容量 */

/* BMS 上报状态位（bits0..5，任一置位立即撤销电机许可） */
#define BAT_ST_BMS_FAULT               (1u << 0)  /* BMS 故障 */
#define BAT_ST_CELL_ABNORMAL           (1u << 1)  /* 电芯异常 */
#define BAT_ST_DIS_OVERCURRENT         (1u << 2)  /* 放电过流 */
#define BAT_ST_DIS_LOW_TEMP            (1u << 3)  /* 放电低温 */
#define BAT_ST_DIS_HIGH_TEMP           (1u << 4)  /* 放电高温 */
#define BAT_ST_CELL_UNDERVOLT          (1u << 5)  /* 电芯欠压 */

/* 工具侧上报给 BMS 的状态位 */
#define TOOL_ST_RUNNING                (1u << 0)  /* 电机运行中 */
#define TOOL_ST_MOS_TEMP_ERR           (1u << 1)  /* MOS 温度故障 */
#define TOOL_ST_CTRL_FAULT             (1u << 3)  /* 控制器故障 */
#define TOOL_ST_MOTOR_BLOCK            (1u << 4)  /* 电机堵转 */
#define TOOL_ST_MOTOR_OVERCURRENT      (1u << 5)  /* 电机过流 */

/* 通信链路状态 */
typedef enum
{
    BAT_ST_IDLE = 0,    /* 空闲/未连接 */
    BAT_ST_LINKED,      /* 已握手连接 */
    BAT_ST_TIMEOUT      /* 会话超时断链 */
} BAT_COM_STATE;

/* 解析后的电池数据及派生保护标志 */
typedef struct
{
    uint8_t Bunch;                  /* 串数 */
    uint8_t Parallel;               /* 并数 */
    uint8_t Capacity;               /* 容量 */
    uint16_t NominalVoltage_x10;    /* 标称电压(0.1V) */
    uint8_t ChgCurrentMax_x10;      /* 最大充电电流(0.1A) */
    uint8_t DisCurrentMax;          /* 最大放电电流(A) */
    uint16_t Voltage_x10;           /* 实时总电压(0.1V) */
    uint16_t CellVoltage_mV;        /* 单体电压(mV) */
    int8_t Temperature;             /* 温度(℃) */
    uint8_t SOC;                    /* 剩余电量(%) */
    uint8_t Status;                 /* BMS 原始状态字 */
    uint8_t BatLowVolage_Flag;      /* 派生：电芯欠压 */
    uint8_t TempAnomaly_Flag;       /* 派生：温度异常 */
    uint8_t OverCurrent_Flag;       /* 派生：放电过流 */
    uint8_t BmsFault_Flag;          /* 派生：BMS 故障/电芯异常 */
} BATDATA;

extern BATDATA BatData;             /* 电池数据全局实例 */
extern BAT_COM_STATE BatComState;   /* 通信链路状态全局实例 */

uint8_t Bat_Com_Crc8(const uint8_t *data, uint8_t len);  /* 计算 CRC8 */
void Bat_Com_Tick1ms(void);                              /* 1ms 时基递增（TIM1 调用） */
void Bat_Com_UsartIrq(void);                             /* USART1 中断处理 */
void Bat_Com(void);                                      /* 通信前台主入口 */
uint8_t Bat_Com_CanRun(void);                            /* 查询电机运行许可 */
void Bat_Com_Restart(void);                              /* 重启通信 */
void Bat_Com_Sleep(void);                                /* 休眠停用通信 */

#endif
