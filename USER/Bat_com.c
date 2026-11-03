/**
 * @file     Bat_com.c
 * @brief    DAYE A1.1 电池包单线半双工通信（工具侧）
 *
 * @attention
 *   物理层：PA11 复用 USART1_TX，PA12 复用 USART1_RX，PA14 为 COM_EN 上拉使能。
 *   COM 总线空闲释放为高；PA11 高会经 Q13/Q14 拉低 COM，故 TX 平时须保持 GPIO 低
 *   释放总线，仅发送时切到 UART 复用。Q12 将 COM 反相到 RX，UART 收发均为正常极性。
 *   报文帧格式：主机 FA FB LEN CMD ... CRC；工具 AF BF LEN CMD ... CRC。LEN 为整帧字节数。
 *   CRC8：多项式 0x25，初值 0，MSB-first。
 *   时序约束：中断只按 LEN 有界组帧并发布不可变邮箱，不解析、不等待；前台负责
 *   CRC/命令/长度校验、安全许可及应答调度。任何 BMS 故障、超时、丢帧都会立即撤销
 *   电机许可并锁存 gBatComFault。
 */

#include "board.h"
#include "Bat_com.h"
#include "user_control.h"
#include "motor_control.h"
#include "parameter.h"
#include "mc_core.h"
#include "hal_uid.h"

#define BAT_RX_GAP_MS          (5u)     /* 字节间隔超过此值判定为帧断裂，丢弃未完成帧(ms) */
#define BAT_REPLY_DELAY_MS     (6u)     /* 收到主机帧后到开始应答前的等待窗口(ms) */
#define BAT_PREAMBLE_MS        (4u)     /* 首字节前的 COM-low 前导时长(ms) */
#define BAT_REPLY_LIMIT_MS     (10u)    /* 从收帧到首字节的应答总窗口上限(ms) */
#define BAT_LINK_TIMEOUT_MS    (3000u)  /* 会话超时：无有效运行帧超过此值即断链并锁存(ms) */
#define BAT_TX_TIMEOUT_MS      (50u)    /* 发送阶段的最长时长，超时判定发送异常(ms) */
/* BMS 上报状态中会立即撤销电机许可的故障位掩码（bits0..5） */
#define BAT_FAULT_MASK         (BAT_ST_BMS_FAULT | BAT_ST_CELL_ABNORMAL | BAT_ST_DIS_OVERCURRENT | \
                                BAT_ST_DIS_LOW_TEMP | BAT_ST_DIS_HIGH_TEMP | BAT_ST_CELL_UNDERVOLT)

/* 发送状态机：空闲 -> 等待应答窗口 -> COM-low 前导 -> 逐字节发送 */
typedef enum
{
    BAT_TX_IDLE,        /* 空闲，无待发应答 */
    BAT_TX_WAIT,        /* 收帧后等待应答延时 */
    BAT_TX_PREAMBLE,    /* 切 UART 前建立 COM-low 前导 */
    BAT_TX_DATA         /* 正在逐字节发送应答 */
} BatTxState;

BATDATA BatData;                /* 解析后的电池数据及派生保护标志 */
BAT_COM_STATE BatComState;      /* 通信链路状态：IDLE/LINKED/TIMEOUT */

static volatile uint32_t ComTickMs;                 /* 通信毫秒时钟，仅由 TIM1 1ms 时基递增 */
static volatile uint8_t ComActive;                  /* 通信启用标志，休眠时为 0 */
static uint8_t RxBuf[BAT_FRAME_BUF_SIZE];           /* 中断组帧缓冲 */
static volatile uint8_t RxCnt;                      /* 当前已收字节数 */
static volatile uint8_t RxExpected;                 /* 由 LEN 字段确定的整帧长度 */
static volatile uint32_t RxLastByteMs;              /* 最近一个字节的接收时刻，用于帧间隔判断 */
static volatile uint8_t RxFrame[BAT_FRAME_BUF_SIZE];/* 完整帧邮箱，前台读取前不可被覆盖 */
static volatile uint8_t RxReady;                    /* 邮箱中完整帧长度，0 表示空 */
static volatile uint8_t RxOverflow;                 /* 邮箱占用时又收到完整帧，标记丢帧 */
static volatile uint32_t RxFrameMs;                 /* 完整帧发布时刻，用于新鲜度判断 */
static uint8_t TxBuf[BAT_FRAME_BUF_SIZE];           /* 应答发送缓冲 */
static uint8_t TxLen;                               /* 应答帧总长度 */
static volatile uint8_t TxIdx;                      /* 应答发送索引 */
static volatile BatTxState TxState;                 /* 发送状态机当前状态 */
static uint32_t TxRequestMs;                        /* 触发本次应答的收帧时刻 */
static uint32_t TxPreambleMs;                        /* 前导实际开始时刻 */
static volatile uint8_t HandshakeAck;               /* 握手应答已真正发送完成(TC)标志 */
static uint8_t RuntimeValid;                        /* 已收到新鲜且安全的 D1 运行帧 */
static uint32_t SessionLastMs;                      /* 最近一次有效会话更新时刻 */

/**
 * @brief  计算 CRC8（多项式 0x25，初值 0，MSB-first）
 * @param  data 参与校验的数据指针
 * @param  len  参与校验的字节数（不含 CRC 本身）
 * @retval 计算得到的 CRC8
 */
uint8_t Bat_Com_Crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t i;
    uint8_t bit;

    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(bit = 0; bit < 8; bit++)
        {
            crc = (uint8_t)((crc & 0x80u) ? ((crc << 1) ^ BAT_CRC_POLY) : (crc << 1));
        }
    }
    return crc;
}

/**
 * @brief  通信毫秒时钟递增，由 TIM1 1ms 时基调用（不做任何解析或发送）
 */
void Bat_Com_Tick1ms(void)
{
    ComTickMs++;
}

/**
 * @brief  撤销电机许可并锁存通信故障
 *         同时置 live 与 latched gBatComFault、清工具/电机使能。
 *         用短临界区保护，避免与中断改写同一 bitfield 字时丢位。
 */
static void Bat_Inhibit(void)
{
    uint32_t mask = __get_PRIMASK();

    __disable_irq();
    user_list.userErr.bits.gBatComFault = 1;
    machine_error_hold.userErr_hold.bits.gBatComFault = 1;
    user_list.flag.bits.gToolEn = 0;
    mc_core_list.motor_en = 0;
    __set_PRIMASK(mask);
}

/**
 * @brief  仅清除实时通信故障位
 *         收到安全运行帧时清 live 故障，但不动 latch，锁存需经既有松扳机恢复流程。
 */
static void Bat_ClearLiveFault(void)
{
    uint32_t mask = __get_PRIMASK();

    __disable_irq();
    user_list.userErr.bits.gBatComFault = 0;
    __set_PRIMASK(mask);
}

/**
 * @brief  查询是否允许电机运行的最终通信许可
 *         需同时满足：通信启用、握手已完成、运行帧新鲜有效、链路已连接、
 *         会话未超时、无 BMS 故障、无 live/latched 通信故障。
 * @retval 1 允许运行，0 禁止（BAT_COM_EN 关闭时恒为 1）
 */
uint8_t Bat_Com_CanRun(void)
{
#if BAT_COM_EN
    return (uint8_t)(ComActive && HandshakeAck && RuntimeValid &&
                     BatComState == BAT_ST_LINKED &&
                     (uint32_t)(ComTickMs - SessionLastMs) < BAT_LINK_TIMEOUT_MS &&
                     !(BatData.Status & BAT_FAULT_MASK) &&
                     !user_list.userErr.bits.gBatComFault &&
                     !machine_error_hold.userErr_hold.bits.gBatComFault);
#else
    return 1;
#endif
}

/**
 * @brief  开启接收：清接收错误、复位组帧计数并使能 RXNE/错误中断
 *         先读 SR 再读 DR 可清除接收错误标志（含总线空闲造成的类 break 帧错误）。
 */
static void Bat_RxEnable(void)
{
    volatile uint32_t discard;

    /* SR then DR clears receive errors, including the break-like physical idle. */
    discard = USART1->SR;
    discard = USART1->DR;
    (void)discard;
    RxCnt = 0;
    RxExpected = 0;
    USART1->CR1 |= USART_CR1_RE_Msk;
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_ERR, ENABLE);
}

/**
 * @brief  复位收发链路
 *         关中断、释放总线、清收发/邮箱状态并重新初始化 USART，
 *         仅在通信仍启用(ComActive)时才重新开启 NVIC，避免休眠期误开中断。
 */
static void Bat_ResetTransport(void)
{
    NVIC_DisableIRQ(USART1_IRQn);
    Bsp_Com_Tx_Mode(0);
    TxState = BAT_TX_IDLE;
    TxLen = 0;
    TxIdx = 0;
    RxReady = 0;
    RxOverflow = 0;
    RxCnt = 0;
    RxExpected = 0;
    Bsp_Usart_Init();
    Bat_RxEnable();
    NVIC_ClearPendingIRQ(USART1_IRQn);
    if(ComActive)
    {
        NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
 * @brief  组装工具侧应答帧到 TxBuf：AF BF LEN CMD data... CRC
 * @param  cmd      命令字
 * @param  data     负载数据指针
 * @param  data_len 负载字节数
 * @retval 整帧长度（含帧头、LEN、CMD、负载、CRC，即 data_len + 5）
 */
static uint8_t Bat_BuildSlaveFrame(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
    uint8_t i;
    uint8_t len = (uint8_t)(data_len + 5u);

    TxBuf[0] = 0xAF;
    TxBuf[1] = 0xBF;
    TxBuf[2] = len;
    TxBuf[3] = cmd;
    for(i = 0; i < data_len; i++)
    {
        TxBuf[4u + i] = data[i];
    }
    TxBuf[len - 1u] = Bat_Com_Crc8(TxBuf, (uint8_t)(len - 1u));
    return len;
}

/**
 * @brief  构造握手应答（A5）：设备类型 + 芯片 UID（大端 4 字节）
 */
static void Bat_SendHandshakeReply(void)
{
    uint8_t data[5];
    uint32_t uid = Get_ChipsetUIDw0();

    data[0] = BAT_DEV_TOOL;
    data[1] = (uint8_t)(uid >> 24);
    data[2] = (uint8_t)(uid >> 16);
    data[3] = (uint8_t)(uid >> 8);
    data[4] = (uint8_t)uid;
    TxLen = Bat_BuildSlaveFrame(BAT_CMD_HANDSHAKE, data, sizeof(data));
}

/**
 * @brief  构造运行数据应答（D1）：母线电流(大端2字节) + 工具状态位
 *         状态位汇总运行、MOS 温度、堵转、过流及控制器故障，供 BMS 侧监控。
 */
static void Bat_SendRuntimeData(void)
{
    uint8_t data[3];
    uint8_t status = 0;
    /* 平均母线电流 ADC 值经每安培系数换算为整数安培 */
    uint16_t current_a = (uint16_t)((float)user_list.IBusAvg_Adc / CURRENT_ADC_PER_A);

    data[0] = (uint8_t)(current_a >> 8);
    data[1] = (uint8_t)current_a;
    /* 电机使能或已进入 RUN 视为运行中 */
    if(mc_core_list.motor_en || motor_control_list.bldc_state == MOTOR_RUN)
    {
        status |= TOOL_ST_RUNNING;
    }
    if(machine_error_hold.userErr_hold.bits.gNtcMos || machine_error_hold.userErr_hold.bits.gMos_overload)
    {
        status |= TOOL_ST_MOS_TEMP_ERR;
    }
    if(machine_error_hold.MC_error_hold.bits.gBlock)
    {
        status |= TOOL_ST_MOTOR_BLOCK;
    }
    if(machine_error_hold.userErr_hold.bits.gIBusAvgOCP1 ||
       machine_error_hold.userErr_hold.bits.gIBusAvgOCP2 ||
       machine_error_hold.userErr_hold.bits.gIBusAvgOCP3 ||
       machine_error_hold.userErr_hold.bits.gIBusAvgOCP4 ||
       machine_error_hold.userErr_hold.bits.gIBusPeakOCP1 ||
       machine_error_hold.userErr_hold.bits.gIBusAcmpOCP)
    {
        status |= TOOL_ST_MOTOR_OVERCURRENT;
    }
    if(machine_error_hold.userErr_hold.word || machine_error_hold.MC_error_hold.word)
    {
        status |= TOOL_ST_CTRL_FAULT;
    }
    data[2] = status;
    TxLen = Bat_BuildSlaveFrame(BAT_CMD_RUNTIME, data, sizeof(data));
}

/**
 * @brief  解析握手帧（A5）中的电池铭牌参数：标称电压、容量、串并数、充放电流上限
 */
static void Bat_ParseHandshake(const uint8_t *frame)
{
    BatData.NominalVoltage_x10 = (uint16_t)(((uint16_t)frame[4] << 8) | frame[5]);
    BatData.Capacity = frame[6];
    BatData.Bunch = frame[7];
    BatData.Parallel = frame[8];
    BatData.ChgCurrentMax_x10 = frame[9];
    BatData.DisCurrentMax = frame[10];
}

/**
 * @brief  解析运行帧（D1）：总电压、单体电压、温度、SOC 及状态字
 *         由状态字直接派生欠压/温度异常/过流/BMS 故障标志，供既有保护处理复用。
 */
static void Bat_ParseRuntime(const uint8_t *frame)
{
    uint8_t status = frame[10];

    BatData.Voltage_x10 = (uint16_t)(((uint16_t)frame[4] << 8) | frame[5]);
    BatData.CellVoltage_mV = (uint16_t)(((uint16_t)frame[6] << 8) | frame[7]);
    BatData.Temperature = (int8_t)frame[8];
    BatData.SOC = frame[9];
    BatData.Status = status;
    BatData.BatLowVolage_Flag = (uint8_t)((status & BAT_ST_CELL_UNDERVOLT) != 0);
    BatData.TempAnomaly_Flag = (uint8_t)((status & (BAT_ST_DIS_LOW_TEMP | BAT_ST_DIS_HIGH_TEMP)) != 0);
    BatData.OverCurrent_Flag = (uint8_t)((status & BAT_ST_DIS_OVERCURRENT) != 0);
    BatData.BmsFault_Flag = (uint8_t)((status & (BAT_ST_BMS_FAULT | BAT_ST_CELL_ABNORMAL)) != 0);
}

/**
 * @brief  前台处理一帧主机报文：校验、解析、准备应答
 *         校验帧头/LEN/命令/CRC；帧过旧则撤销许可。
 *         A5 握手：会话重建时清运行有效标志，避免按住扳机自动恢复；重复 A5 不续旧数据期限。
 *         D1 运行：BMS 故障立即撤销；未握手时只回 FF 重握手，不刷新有效期；
 *         已握手且安全时刷新会话、置有效并清 live 故障。
 * @param  frame       完整帧数据
 * @param  len         帧长度
 * @param  received_ms 帧接收时刻（用于新鲜度与超时判断）
 */
static void Bat_HandleFrame(const uint8_t *frame, uint8_t len, uint32_t received_ms)
{
    /* 帧头、LEN 自洽、命令合法、CRC 全部通过才继续 */
    if(len != BAT_FRAME_LEN_HS_MASTER || frame[2] != len ||
       frame[0] != 0xFA || frame[1] != 0xFB ||
       (frame[3] != BAT_CMD_HANDSHAKE && frame[3] != BAT_CMD_RUNTIME) ||
       Bat_Com_Crc8(frame, (uint8_t)(len - 1u)) != frame[len - 1u])
    {
        return;
    }
    /* 帧时间戳已超过超时窗口，视为陈旧，撤销许可 */
    if((uint32_t)(ComTickMs - received_ms) >= BAT_LINK_TIMEOUT_MS)
    {
        Bat_Inhibit();
        return;
    }
    if(frame[3] == BAT_CMD_HANDSHAKE)
    {
        /* 会话进行中又收到握手，说明链路重建，撤销许可防止自动恢复 */
        if(RuntimeValid)
        {
            Bat_Inhibit();
        }
        RuntimeValid = 0;
        HandshakeAck = 0;
        /* 仅从超时态重新握手时重设会话起点，重复 A5 不续旧运行数据期限 */
        if(BatComState == BAT_ST_TIMEOUT)
        {
            SessionLastMs = received_ms;
        }
        BatComState = BAT_ST_LINKED;
        Bat_ParseHandshake(frame);
        Bat_SendHandshakeReply();
    }
    else
    {
        Bat_ParseRuntime(frame);
        /* BMS 故障位任一置位立即撤销许可 */
        if(BatData.Status & BAT_FAULT_MASK)
        {
            Bat_Inhibit();
        }
        /* 未完成握手时只回 FF 请求重握手，不刷新运行有效期 */
        if(!HandshakeAck || BatComState != BAT_ST_LINKED)
        {
            TxLen = Bat_BuildSlaveFrame(BAT_CMD_REHANDSHAKE, 0, 0);
        }
        else
        {
            SessionLastMs = received_ms;
            RuntimeValid = 1;
            /* 安全运行帧只清 live 故障，latch 需经松扳机恢复 */
            if(!(BatData.Status & BAT_FAULT_MASK))
            {
                Bat_ClearLiveFault();
            }
            Bat_SendRuntimeData();
        }
    }
    /* 登记应答请求，进入等待窗口 */
    TxRequestMs = received_ms;
    TxIdx = 0;
    TxState = BAT_TX_WAIT;
}

/**
 * @brief  中断中逐字节组帧（按 FA/FB/LEN 有界）
 *         字节间隔超时丢弃未完成帧；组满 LEN 后发布不可变邮箱，
 *         邮箱占用时置溢出标志。仅在 TX 空闲/等待态被调用。
 * @param  data 收到的字节
 */
static void Bat_ReceiveByte(uint8_t data)
{
    uint8_t i;
    uint32_t now = ComTickMs;

    /* 字节间隔过大，判定上一未完成帧断裂 */
    if(RxCnt && (uint32_t)(now - RxLastByteMs) >= BAT_RX_GAP_MS)
    {
        RxCnt = 0;
    }
    RxLastByteMs = now;
    /* 等待帧头 FA */
    if(RxCnt == 0)
    {
        if(data == 0xFA)
        {
            RxBuf[RxCnt++] = data;
        }
        return;
    }
    /* 帧头第二字节需为 FB，否则重新对齐（允许该字节本身是新的 FA） */
    if(RxCnt == 1 && data != 0xFB)
    {
        RxCnt = (uint8_t)(data == 0xFA);
        return;
    }
    /* LEN 字段：越界则丢弃 */
    if(RxCnt == 2)
    {
        if(data < 5u || data > BAT_FRAME_BUF_SIZE)
        {
            RxCnt = 0;
            return;
        }
        RxExpected = data;
    }
    RxBuf[RxCnt++] = data;
    /* 收满整帧：发布到邮箱或标记溢出 */
    if(RxCnt >= 3u && RxCnt == RxExpected)
    {
        if(RxReady)
        {
            RxOverflow = 1;
        }
        else
        {
            for(i = 0; i < RxCnt; i++)
            {
                RxFrame[i] = RxBuf[i];
            }
            RxFrameMs = now;
            RxReady = RxCnt;
        }
        RxCnt = 0;
    }
}

/**
 * @brief  USART1 中断处理：接收组帧、逐字节发送、发送完成释放总线
 *         每个分支都同时判断标志位与对应中断使能，因 HAL 的 SR 查询不含使能状态。
 *         TXE 分支处理后立即返回，防止用同一旧 SR 快照提前触发 TC。
 *         握手帧真正 TC 完成后才置 HandshakeAck；TC 后释放 GPIO 并恢复接收。
 */
void Bat_Com_UsartIrq(void)
{
    uint32_t status = USART1->SR;
    uint32_t enabled = USART1->CR1;
    uint8_t data;

    if(!ComActive)
    {
        return;
    }
    /* 接收错误（帧错误/噪声/溢出/校验）：读走数据并丢弃未完成帧 */
    if(((status & (USART_FLAG_FE | USART_FLAG_NF | USART_FLAG_ORE)) &&
        (USART1->CR3 & USART_IT_ERR)) ||
       ((status & USART_FLAG_PE) && (enabled & USART_IT_PE)))
    {
        (void)USART_ReceiveData(USART1);
        RxCnt = 0;
    }
    /* 正常接收：仅在未发送时组帧，避免发送期间回显干扰 */
    else if((status & USART_FLAG_RXNE) && (enabled & USART_IT_RXNE))
    {
        data = (uint8_t)USART_ReceiveData(USART1);
        if(TxState == BAT_TX_IDLE || TxState == BAT_TX_WAIT)
        {
            Bat_ReceiveByte(data);
        }
    }
    /* 发送寄存器空：送下一字节，最后一字节后切到 TC 中断收尾 */
    if((status & USART_FLAG_TXE) && (enabled & USART_IT_TXE))
    {
        if(TxState == BAT_TX_DATA && TxIdx < TxLen)
        {
            USART_SendData(USART1, TxBuf[TxIdx++]);
        }
        if(TxIdx >= TxLen)
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        }
        return;
    }
    /* 发送完成：末停止位移出后才释放总线，握手帧此时才确认应答完成 */
    if((status & USART_FLAG_TC) && (enabled & USART_IT_TC))
    {
        USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        if(TxState == BAT_TX_DATA && TxIdx == TxLen)
        {
            Bsp_Com_Tx_Mode(0);
            if(TxBuf[3] == BAT_CMD_HANDSHAKE)
            {
                HandshakeAck = 1;
            }
            TxState = BAT_TX_IDLE;
            Bat_RxEnable();
        }
    }
}

/**
 * @brief  前台应答时序推进（所有计时基于 ISR 毫秒时钟，不依赖调用次数）
 *         WAIT：到 6ms 后若余窗不足或有待收数据则取消应答，否则建立 COM-low 前导；
 *         PREAMBLE：到 4ms 后送首字节进入发送；超窗则撤销许可并复位链路；
 *         DATA：发送超 50ms 判异常，撤销许可并复位。
 */
static void Bat_ServiceTx(void)
{
    uint32_t now = ComTickMs;
    uint32_t elapsed = (uint32_t)(now - TxRequestMs);

    /* 发送阶段超时：链路异常，撤销许可、锁存并复位 */
    if(TxState == BAT_TX_DATA && elapsed >= BAT_TX_TIMEOUT_MS)
    {
        Bat_Inhibit();
        HandshakeAck = 0;
        RuntimeValid = 0;
        BatComState = BAT_ST_TIMEOUT;
        Bat_ResetTransport();
        return;
    }
    if(TxState == BAT_TX_WAIT && elapsed >= BAT_REPLY_DELAY_MS)
    {
        /* 已错过精确等待点，放弃本次应答（不压短前导、不补发） */
        if(elapsed > BAT_REPLY_DELAY_MS)
        {
            TxState = BAT_TX_IDLE;
            return;
        }
        NVIC_DisableIRQ(USART1_IRQn);
        /* 清掉已经断裂的未完成帧 */
        if(RxCnt && (uint32_t)(now - RxLastByteMs) >= BAT_RX_GAP_MS)
        {
            RxCnt = 0;
        }
        /* 仍有待收/待处理数据，则让位于接收，取消本次应答 */
        if(RxCnt || RxReady || RxOverflow)
        {
            TxState = BAT_TX_IDLE;
            NVIC_EnableIRQ(USART1_IRQn);
            return;
        }
        /* 关接收、切 UART 复用，开始 COM-low 前导并记录实际起点 */
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        USART_ITConfig(USART1, USART_IT_ERR, DISABLE);
        USART1->CR1 &= ~USART_CR1_RE_Msk;
        TxState = BAT_TX_PREAMBLE;
        TxPreambleMs = ComTickMs;
        Bsp_Com_Tx_Mode(1);
        NVIC_EnableIRQ(USART1_IRQn);
    }
    else if(TxState == BAT_TX_PREAMBLE &&
            (uint32_t)(now - TxPreambleMs) >= BAT_PREAMBLE_MS)
    {
        /* 总窗或前导超限：链路异常，撤销许可并复位 */
        if(elapsed > BAT_REPLY_LIMIT_MS ||
           (uint32_t)(now - TxPreambleMs) > BAT_PREAMBLE_MS)
        {
            Bat_Inhibit();
            HandshakeAck = 0;
            RuntimeValid = 0;
            BatComState = BAT_ST_TIMEOUT;
            Bat_ResetTransport();
            return;
        }
        /* 清 TC，发出首字节并开启 TXE 中断驱动后续发送 */
        USART_ClearFlag(USART1, USART_FLAG_TC);
        TxState = BAT_TX_DATA;
        USART_SendData(USART1, TxBuf[TxIdx++]);
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    }
}

/**
 * @brief  通信前台主入口，由主循环高频调用
 *         先判会话是否超时；再取邮箱中的完整帧交由 Bat_HandleFrame 处理；
 *         若发送中又来新帧则视为冲突，撤销会话并复位以让位接收；最后推进发送时序。
 *         所有计时均基于 ISR 毫秒时钟，不受主循环执行次数影响。
 */
void Bat_Com(void)
{
    uint8_t frame[BAT_FRAME_BUF_SIZE];
    uint8_t len;
    uint8_t overflow;
    uint8_t i;
    uint32_t received_ms;

    if(!ComActive)
    {
        return;
    }
    /* 会话超时：撤销握手/运行、锁存故障并复位链路 */
    if(BatComState != BAT_ST_TIMEOUT &&
       (uint32_t)(ComTickMs - SessionLastMs) >= BAT_LINK_TIMEOUT_MS)
    {
        HandshakeAck = 0;
        RuntimeValid = 0;
        BatComState = BAT_ST_TIMEOUT;
        Bat_Inhibit();
        Bat_ResetTransport();
    }
    if(RxReady || RxOverflow)
    {
        /* 正在发送时又收到帧：TxBuf 正被使用，冲突则撤销会话并复位 */
        if(TxState == BAT_TX_PREAMBLE || TxState == BAT_TX_DATA)
        {
            HandshakeAck = 0;
            RuntimeValid = 0;
            BatComState = BAT_ST_TIMEOUT;
            Bat_Inhibit();
            Bat_ResetTransport();
            return;
        }
        /* 短暂关中断，取出邮箱内容后立即释放邮箱 */
        NVIC_DisableIRQ(USART1_IRQn);
        len = RxReady;
        received_ms = RxFrameMs;
        overflow = RxOverflow;
        for(i = 0; i < len; i++)
        {
            frame[i] = RxFrame[i];
        }
        RxReady = 0;
        RxOverflow = 0;
        NVIC_EnableIRQ(USART1_IRQn);
        /* 邮箱溢出导致丢帧：撤销许可、锁存，避免沿用旧安全状态 */
        if(overflow)
        {
            HandshakeAck = 0;
            RuntimeValid = 0;
            BatComState = BAT_ST_TIMEOUT;
            Bat_Inhibit();
            Bat_ResetTransport();
        }
        else
        {
            Bat_HandleFrame(frame, len, received_ms);
        }
    }
    Bat_ServiceTx();
}

/**
 * @brief  重启通信（上电/唤醒）：清握手/运行状态、拉高 COM_EN、复位链路
 *         复位前 ComActive 置 0 以防误开中断，链路就绪后才置位并开 NVIC。
 *         不清除任何其他保护故障，未重新满足许可前电机不得运行。
 */
void Bat_Com_Restart(void)
{
#if BAT_COM_EN
    NVIC_DisableIRQ(USART1_IRQn);
    ComActive = 0;
    HandshakeAck = 0;
    RuntimeValid = 0;
    BatComState = BAT_ST_IDLE;
    SessionLastMs = ComTickMs;
    user_list.flag.bits.gToolEn = 0;
    mc_core_list.motor_en = 0;
    COM_EN_HIGH();
    Bat_ResetTransport();
    ComActive = 1;
    NVIC_EnableIRQ(USART1_IRQn);
#endif
}

/**
 * @brief  休眠停用通信：关中断、停发送、关闭 USART 与 COM_EN 释放总线
 *         清收发/会话状态并撤销工具/电机许可；不清除其他保护故障。
 */
void Bat_Com_Sleep(void)
{
#if BAT_COM_EN
    NVIC_DisableIRQ(USART1_IRQn);
    ComActive = 0;
    USART1->CR1 &= ~(USART_IT_RXNE | USART_IT_TXE | USART_IT_TC | USART_CR1_RE_Msk);
    USART_ITConfig(USART1, USART_IT_ERR, DISABLE);
    USART_Cmd(USART1, DISABLE);
    Bsp_Com_Tx_Mode(0);
    COM_EN_LOW();
    TxState = BAT_TX_IDLE;
    RxReady = 0;
    RxOverflow = 0;
    RxCnt = 0;
    HandshakeAck = 0;
    RuntimeValid = 0;
    BatComState = BAT_ST_IDLE;
    user_list.flag.bits.gToolEn = 0;
    mc_core_list.motor_en = 0;
    NVIC_ClearPendingIRQ(USART1_IRQn);
#endif
}
