#include <string.h>

USART_TypeDef test_usart;
uint8_t test_com_en;
uint32_t PI_sum_i, PI_p, pwm_duty_aim_PI_OUT;
struct test_light light_ctrl;
User_list_Struct user_list;
Machine_ERROR_HOLD machine_error_hold;
Motor_Control_list motor_control_list;
Mc_core_list mc_core_list;

volatile uint32_t test_failure_line;
volatile uint32_t test_case;
volatile uint32_t test_checks;
static uint32_t primask;
static uint8_t irq_enabled;
static uint8_t tx_mode;
static uint8_t power_en;
static uint8_t sent[128];
static uint32_t sent_count;
static uint32_t motor_actions;
static uint32_t ipd_calls;

#define CHECK(condition) do { test_checks++; if(!(condition)) { test_failure_line = __LINE__; return; } } while(0)

uint32_t __get_PRIMASK(void) { return primask; }
void __disable_irq(void) { primask = 1; }
void __set_PRIMASK(uint32_t value) { primask = value; }
void NVIC_DisableIRQ(int irq) { (void)irq; irq_enabled = 0; }
void NVIC_EnableIRQ(int irq) { (void)irq; irq_enabled = 1; }
void NVIC_ClearPendingIRQ(int irq) { (void)irq; }
void Bsp_Com_Tx_Mode(uint8_t enabled) { tx_mode = enabled; }
void Bsp_Usart_Init(void) { memset(&test_usart, 0, sizeof(test_usart)); }
void USART_ITConfig(USART_TypeDef *uart, uint32_t flags, FunctionalState state)
{
    volatile uint32_t *reg = flags == USART_IT_ERR ? &uart->CR3 : &uart->CR1;
    if(state) *reg |= flags;
    else *reg &= ~flags;
}
uint16_t USART_ReceiveData(USART_TypeDef *uart)
{
    uart->SR &= ~(USART_FLAG_RXNE | USART_FLAG_PE | USART_FLAG_FE | USART_FLAG_NF | USART_FLAG_ORE);
    return (uint16_t)uart->DR;
}
void USART_SendData(USART_TypeDef *uart, uint16_t data)
{
    if(sent_count < sizeof(sent)) sent[sent_count] = (uint8_t)data;
    sent_count++;
    uart->SR &= ~(USART_FLAG_TXE | USART_FLAG_TC);
}
void USART_ClearFlag(USART_TypeDef *uart, uint32_t flags) { uart->SR &= ~flags; }
void USART_Cmd(USART_TypeDef *uart, FunctionalState state) { (void)uart; (void)state; }
uint32_t Get_ChipsetUIDw0(void) { return 0x12345678u; }
void GPIO_SetBits(void *gpio, uint16_t pin) { (void)gpio; (void)pin; power_en = 1; }
void GPIO_ResetBits(void *gpio, uint16_t pin) { (void)gpio; (void)pin; power_en = 0; }
static uint16_t dir_input_bits = RWD_PIN;
uint16_t GPIO_ReadInputData(void *gpio) { (void)gpio; return dir_input_bits; }
void TIM_CtrlPWMOutputs(void *timer, FunctionalState state) { (void)timer; (void)state; }
void TIM_Cmd(void *timer, FunctionalState state) { (void)timer; (void)state; }
void TIM_ClearFlag(void *timer, uint32_t flags) { (void)timer; (void)flags; }
void Stop_Motor(void) { }
void Brake_Motor(void) { motor_actions++; }
void pwm_set(void) { motor_actions++; }
void PwmDutyUpdate(uint16_t duty) { (void)duty; motor_actions++; }
void UVWL_HOffLPwm(void) { motor_actions++; }
void mc_core_stop_reset(void) { }
void motion_handle(void) { motor_actions++; mc_core_list.motion_processed_result = 1; }
void ipd_detect(void) { ipd_calls++; }
void motor_to_bemf_runing_pre(void) { motor_actions++; }
void align_control(void) { motor_actions++; }
u8 drag_control(void) { motor_actions++; return 1; }
void drag_control_init(void) { motor_actions++; }

static void reset_fixture(void)
{
    memset(&user_list, 0, sizeof(user_list));
    memset(&machine_error_hold, 0, sizeof(machine_error_hold));
    memset(&motor_control_list, 0, sizeof(motor_control_list));
    memset(&mc_core_list, 0, sizeof(mc_core_list));
    memset(&BatData, 0, sizeof(BatData));
    ComTickMs = 0;
    primask = 0;
    sent_count = 0;
    motor_actions = 0;
    ipd_calls = 0;
    light_ctrl.state = LIGHT_OFF;
    user_list.direction_ready = 1;
    user_list.dir_valid = 1;
    user_list.dir_retrig_need = 0;
    dir_input_bits = RWD_PIN;
    Bat_Com_Restart();
}

static void irq(uint32_t status, uint8_t data)
{
    test_usart.SR = status;
    test_usart.DR = data;
    if(irq_enabled) Bat_Com_UsartIrq();
}

static void tick(uint32_t ms)
{
    while(ms--) Bat_Com_Tick1ms();
}

static void step(uint32_t ms)
{
    while(ms--)
    {
        Bat_Com_Tick1ms();
        Bat_Com();
    }
}

static void receive(const uint8_t *frame, uint8_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        irq(USART_FLAG_RXNE | USART_FLAG_TXE | USART_FLAG_TC, frame[i]);
        if(i + 1u < len) tick(2);
    }
}

static const uint8_t handshake[] = {0xFA, 0xFB, 0x0C, 0xA5, 0, 0xC8, 0x14, 5, 1, 0x2D, 0x1E, 0x54};

static void runtime(uint8_t status)
{
    uint8_t frame[] = {0xFA, 0xFB, 0x0C, 0xD1, 0, 200, 0x0F, 0xA0, 25, 80, 0, 0};
    frame[10] = status;
    frame[11] = Bat_Com_Crc8(frame, 11);
    receive(frame, sizeof(frame));
    Bat_Com();
}

static void drain_tx(void)
{
    uint8_t limit = BAT_FRAME_BUF_SIZE;
    while(TxState == BAT_TX_DATA && TxIdx < TxLen && limit--)
    {
        tick(2);
        irq(USART_FLAG_TXE | USART_FLAG_TC, 0);
    }
    tick(2);
    irq(USART_FLAG_TC, 0);
}

static void reply(void)
{
    step(BAT_REPLY_DELAY_MS + BAT_PREAMBLE_MS);
    drain_tx();
}

static void link_safe(void)
{
    receive(handshake, sizeof(handshake));
    Bat_Com();
    reply();
    runtime(0);
    reply();
}

static void test_handshake_and_uart(void)
{
    reset_fixture();
    CHECK(Bat_Com_Crc8(handshake, 11) == 0x54);
    CHECK(!Bat_Com_CanRun() && irq_enabled && !tx_mode && test_com_en);
    step(500);
    CHECK(sent_count == 0 && TxState == BAT_TX_IDLE);
    receive(handshake, sizeof(handshake));
    CHECK(RxReady == 12 && sent_count == 0 && !HandshakeAck);
    irq(USART_FLAG_RXNE | USART_FLAG_FE, 0);
    CHECK(RxReady == 12);
    Bat_Com();
    CHECK(TxState == BAT_TX_WAIT && !HandshakeAck && !Bat_Com_CanRun());
    CHECK(BatData.NominalVoltage_x10 == 200 && BatData.DisCurrentMax == 30);
    step(5);
    CHECK(!tx_mode && sent_count == 0);
    step(1);
    CHECK(tx_mode && TxState == BAT_TX_PREAMBLE);
    step(3);
    CHECK(sent_count == 0);
    step(1);
    CHECK(sent_count == 1 && sent[0] == 0xAF && !HandshakeAck);
    drain_tx();
    CHECK(HandshakeAck && !tx_mode && TxState == BAT_TX_IDLE);
    CHECK(sent_count == BAT_FRAME_LEN_HS_SLAVE);
    CHECK(sent[1] == 0xBF && sent[2] == 10 && sent[3] == 0xA5 && sent[4] == 0xD1);
    CHECK(sent[5] == 0x12 && sent[6] == 0x34 && sent[7] == 0x56 && sent[8] == 0x78);
    CHECK(sent[9] == Bat_Com_Crc8(sent, 9));
    CHECK(!Bat_Com_CanRun());
    runtime(0);
    CHECK(Bat_Com_CanRun());
    CHECK(BatData.Voltage_x10 == 200 && BatData.CellVoltage_mV == 4000);
    reply();
    CHECK(sent_count == 18 && sent[12] == 8 && sent[13] == 0xD1);
}

static void test_framing(void)
{
    uint8_t frame[16];
    uint32_t before;
    reset_fixture();
    memcpy(frame, handshake, 12);
    frame[11] ^= 1;
    receive(frame, 12);
    Bat_Com();
    CHECK(TxState == BAT_TX_IDLE && !HandshakeAck);
    frame[3] = 0x77;
    frame[11] = Bat_Com_Crc8(frame, 11);
    receive(frame, 12);
    Bat_Com();
    CHECK(TxState == BAT_TX_IDLE && SessionLastMs == 0);
    frame[2] = 13;
    frame[3] = BAT_CMD_HANDSHAKE;
    frame[12] = Bat_Com_Crc8(frame, 12);
    receive(frame, 13);
    Bat_Com();
    CHECK(TxState == BAT_TX_IDLE);
    irq(USART_FLAG_RXNE, 0xFA);
    irq(USART_FLAG_RXNE, 0xFB);
    irq(USART_FLAG_RXNE, 0xFF);
    CHECK(RxCnt == 0);
    receive(handshake, 6);
    tick(BAT_RX_GAP_MS);
    receive(handshake, 12);
    CHECK(RxReady == 12);
    Bat_Com();
    reply();
    before = sent_count;
    irq(USART_FLAG_TXE | USART_FLAG_TC, 0);
    CHECK(sent_count == before && TxState == BAT_TX_IDLE);
    receive(handshake, 6);
    irq(USART_FLAG_ORE | USART_FLAG_RXNE, 0xFA);
    CHECK(RxCnt == 0 && RxReady == 0);
    receive(handshake, 12);
    irq(USART_FLAG_NF | USART_FLAG_RXNE, 0);
    CHECK(RxReady == 12);
}

static void test_handshake_required(void)
{
    reset_fixture();
    runtime(0);
    CHECK(!Bat_Com_CanRun() && !RuntimeValid && !HandshakeAck);
    reply();
    CHECK(sent_count == 5 && sent[3] == 0xFF);
    CHECK(SessionLastMs == 0);
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    tick(7);
    Bat_Com();
    CHECK(!tx_mode && TxState == BAT_TX_IDLE && sent_count == 0);
    runtime(0);
    CHECK(!Bat_Com_CanRun());
    reply();
    CHECK(sent_count == 5 && sent[3] == 0xFF);
}

static void test_bms_faults_and_recovery(void)
{
    uint8_t bit;
    for(bit = 0; bit < 6; bit++)
    {
        reset_fixture();
        link_safe();
        CHECK(Bat_Com_CanRun());
        user_list.flag.bits.gToolTrigger = 1;
        user_state_control();
        CHECK(user_list.user_state == TOOL_RUN && user_list.flag.bits.gToolEn);
        mc_core_list.motor_en = 1;
        runtime((uint8_t)(1u << bit));
        CHECK(!Bat_Com_CanRun() && !mc_core_list.motor_en && !user_list.flag.bits.gToolEn);
        CHECK(user_list.userErr.bits.gBatComFault && machine_error_hold.userErr_hold.bits.gBatComFault);
        CHECK(BatData.BatLowVolage_Flag == (bit == 5));
        CHECK(BatData.TempAnomaly_Flag == (bit == 3 || bit == 4));
        user_state_control();
        CHECK(user_list.user_state == TOOL_ERROR_STOP);
        reply();
        runtime(0);
        CHECK(!user_list.userErr.bits.gBatComFault && machine_error_hold.userErr_hold.bits.gBatComFault);
        user_state_control();
        CHECK(user_list.user_state == TOOL_ERROR_STOP && !Bat_Com_CanRun());
        user_list.flag.bits.gToolTrigger = 0;
        user_state_control();
        CHECK(user_list.user_state == TOOL_STOP && !machine_error_hold.userErr_hold.bits.gBatComFault);
        user_list.flag.bits.gToolTrigger = 1;
        user_state_control();
        CHECK(user_list.user_state == TOOL_RUN && user_list.flag.bits.gToolEn);
    }
    reset_fixture();
    link_safe();
    runtime(0x3F);
    CHECK(BatData.BatLowVolage_Flag && BatData.TempAnomaly_Flag && BatData.OverCurrent_Flag && BatData.BmsFault_Flag);
}

static void test_timeout_and_stale_handshake(void)
{
    uint32_t last;
    reset_fixture();
    step(3000);
    CHECK(BatComState == BAT_ST_TIMEOUT && user_list.userErr.bits.gBatComFault);
    reset_fixture();
    link_safe();
    last = SessionLastMs;
    ComTickMs = last + 2999u;
    CHECK(Bat_Com_CanRun());
    step(1);
    CHECK(!Bat_Com_CanRun() && BatComState == BAT_ST_TIMEOUT && machine_error_hold.userErr_hold.bits.gBatComFault);
    receive(handshake, 12);
    Bat_Com();
    reply();
    runtime(0);
    CHECK(!user_list.userErr.bits.gBatComFault && !Bat_Com_CanRun());
    reset_fixture();
    link_safe();
    last = SessionLastMs;
    ComTickMs = last + 1000u;
    receive(handshake, 12);
    Bat_Com();
    reply();
    CHECK(SessionLastMs == last && !RuntimeValid && !Bat_Com_CanRun());
    ComTickMs = last + 2900u;
    receive(handshake, 12);
    Bat_Com();
    reply();
    CHECK(SessionLastMs == last);
    ComTickMs = last + 3000u;
    Bat_Com();
    CHECK(BatComState == BAT_ST_TIMEOUT);
    reset_fixture();
    ComTickMs = 0xFFFFFFD0u;
    Bat_Com_Restart();
    link_safe();
    CHECK(Bat_Com_CanRun());
    last = SessionLastMs;
    ComTickMs = last + 2999u;
    CHECK(Bat_Com_CanRun());
    step(1);
    CHECK(!Bat_Com_CanRun());
}

static void test_overflow_echo_and_tx_abort(void)
{
    uint8_t i;
    reset_fixture();
    receive(handshake, 12);
    receive(handshake, 12);
    CHECK(RxReady == 12 && RxOverflow);
    Bat_Com();
    CHECK(!Bat_Com_CanRun() && BatComState == BAT_ST_TIMEOUT && machine_error_hold.userErr_hold.bits.gBatComFault);
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    step(10);
    for(i = 0; i < 12; i++) irq(USART_FLAG_RXNE, handshake[i]);
    CHECK(RxReady == 0 && RxCnt == 0);
    drain_tx();
    CHECK(HandshakeAck);
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    step(6);
    tick(5);
    Bat_Com();
    CHECK(!tx_mode && TxState == BAT_TX_IDLE && sent_count == 0 && !HandshakeAck);
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    step(10);
    ComTickMs = TxRequestMs + BAT_TX_TIMEOUT_MS;
    Bat_Com();
    CHECK(!tx_mode && TxState == BAT_TX_IDLE && BatComState == BAT_ST_TIMEOUT);
}

static void test_restart_sleep_and_unrelated_faults(void)
{
    uint32_t before;
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    step(10);
    before = sent_count;
    user_list.userErr.bits.gNtcMos = 1;
    machine_error_hold.userErr_hold.bits.gIBusAcmpOCP = 1;
    machine_error_hold.userErr_hold.bits.gBatComFault = 1;
    BatData.TempAnomaly_Flag = 1;
    Bat_Com_Restart();
    irq(USART_FLAG_TXE | USART_FLAG_TC, 0);
    CHECK(!HandshakeAck && !RuntimeValid && !tx_mode && sent_count == before);
    CHECK(user_list.userErr.bits.gNtcMos && machine_error_hold.userErr_hold.bits.gIBusAcmpOCP);
    CHECK(machine_error_hold.userErr_hold.bits.gBatComFault && BatData.TempAnomaly_Flag);
    Bat_Com_Sleep();
    CHECK(!irq_enabled && !test_com_en && !tx_mode && !Bat_Com_CanRun());
    Bat_Com_Restart();
    CHECK(irq_enabled && test_com_en && !Bat_Com_CanRun());
    reset_fixture();
    user_list.user_state = TOOL_ERROR_STOP;
    machine_error_hold.userErr_hold.bits.gBatComFault = 1;
    user_list.sleep_cnt = SLEEP_TIME;
    user_state_control();
    CHECK(user_list.user_state == TOOL_POWER_DOWN && !ComActive);
    user_list.flag.bits.gToolTrigger = 1;
    user_state_control();
    CHECK(ComActive && power_en && !Bat_Com_CanRun());
    CHECK(machine_error_hold.userErr_hold.bits.gBatComFault);
    reset_fixture();
    machine_error_hold.userErr_hold.bits.gBatComFault = 1;
    user_list.sleep_cnt = SLEEP_TIME;
    user_state_control();
    CHECK(user_list.user_state == TOOL_POWER_DOWN && !ComActive);
}

static void test_motor_start_boundaries(void)
{
    static const MC_StateMachine_e states[] = {MOTOR_MOTION, MOTOR_READY, Motor_Align, MOTOR_POSITION, MotorStart_Drag_OPENLOOP};
    uint8_t i;
    uint8_t fault;
    for(fault = 0; fault < 3; fault++)
    {
        for(i = 0; i < sizeof(states) / sizeof(states[0]); i++)
        {
            reset_fixture();
            motor_control_list.bldc_state = states[i];
            mc_core_list.motor_en = (uint8_t)(fault != 0);
            machine_error_hold.userErr_hold.bits.gBatComFault = (fault == 1);
            machine_error_hold.MC_error_hold.bits.gIPDOCP = (fault == 2);
            mc_core_list.Board_ADC_DATA.u16BemfU = Motion_ADC_THRESHOLD + 1u;
            mc_core_list.aglin_count = 201;
            mc_core_list.ipd_star_flag = 0;
            MC_Machine_State();
            CHECK(motor_control_list.bldc_state == MOTOR_STOP);
            CHECK(motor_actions == 0 && ipd_calls == 0);
        }
    }
    reset_fixture();
    link_safe();
    user_list.flag.bits.gToolEn = 1;
    motor_control_list.bldc_state = MOTOR_POSITION;
    mc_core_list.ipd_star_flag = 1;
    run_main_enable_ipd();
    CHECK(mc_core_list.motor_en && ipd_calls == 1);
    user_list.userErr.bits.gNtcMos = 1;
    mc_core_list.ipd_star_flag = 1;
    run_main_enable_ipd();
    CHECK(!mc_core_list.motor_en && ipd_calls == 1 && !mc_core_list.ipd_star_flag);
    user_list.userErr.bits.gNtcMos = 0;
    runtime(BAT_ST_BMS_FAULT);
    mc_core_list.ipd_star_flag = 1;
    run_main_enable_ipd();
    CHECK(!mc_core_list.motor_en && ipd_calls == 1);
}

static void test_receive_during_reply_wait(void)
{
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    step(4);
    irq(USART_FLAG_RXNE, 0xFA);
    step(2);
    CHECK(!tx_mode && TxState == BAT_TX_IDLE && sent_count == 0);
    CHECK(RxCnt == 1 && irq_enabled);
    tick(BAT_RX_GAP_MS);
    receive(handshake, 12);
    Bat_Com();
    reply();
    CHECK(HandshakeAck && !tx_mode);
    reset_fixture();
    receive(handshake, 12);
    Bat_Com();
    receive(handshake, 12);
    CHECK(RxReady == 12);
    Bat_Com();
    CHECK(!tx_mode && TxState == BAT_TX_WAIT && !RxReady);
    reply();
    CHECK(HandshakeAck);
}

static void test_invalid_runtime_does_not_refresh(void)
{
    uint8_t frame[12];
    uint32_t last;
    reset_fixture();
    link_safe();
    last = SessionLastMs;
    ComTickMs = last + 2900u;
    memcpy(frame, handshake, sizeof(frame));
    frame[3] = 0x22;
    frame[11] = Bat_Com_Crc8(frame, 11);
    receive(frame, sizeof(frame));
    Bat_Com();
    CHECK(SessionLastMs == last && Bat_Com_CanRun());
    frame[3] = BAT_CMD_RUNTIME;
    frame[11] = (uint8_t)(Bat_Com_Crc8(frame, 11) ^ 1u);
    receive(frame, sizeof(frame));
    Bat_Com();
    CHECK(SessionLastMs == last);
    ComTickMs = last + 3000u;
    Bat_Com();
    CHECK(!Bat_Com_CanRun() && BatComState == BAT_ST_TIMEOUT);
}

static void test_direction_interlock(void)
{
    uint8_t i;
    reset_fixture();
    link_safe();
    CHECK(Bat_Com_CanRun());

    /* 两引脚同时不生效（都为高）：方向无效，去抖后不就绪、不允许启动 */
    dir_input_bits = DIR_PIN | RWD_PIN;
    for(i = 0; i < 12; i++) user_direction_handle();
    CHECK(!user_list.dir_valid && !user_list.direction_ready);

    /* 两引脚同时生效（都为低）：同样无效 */
    dir_input_bits = 0;
    for(i = 0; i < 12; i++) user_direction_handle();
    CHECK(!user_list.dir_valid && !user_list.direction_ready);

    /* 选择单一方向(FWD)，但电位器保持按下：方向有效但需重扣，未就绪 */
    user_list.flag.bits.gToolTrigger = 1;
    dir_input_bits = RWD_PIN;
    for(i = 0; i < 12; i++) user_direction_handle();
    CHECK(user_list.dir_valid && user_list.dir_retrig_need && !user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state != TOOL_RUN && !user_list.flag.bits.gToolEn);

    /* 松开电位器：解除重扣锁定，方向就绪，但松开状态下仍不启动 */
    user_list.flag.bits.gToolTrigger = 0;
    user_direction_handle();
    CHECK(!user_list.dir_retrig_need && user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state != TOOL_RUN);

    /* 重新按下电位器：满足启动条件，电机可转 */
    user_list.flag.bits.gToolTrigger = 1;
    user_direction_handle();
    CHECK(user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state == TOOL_RUN && user_list.flag.bits.gToolEn);

    /* 运行途中两引脚同时生效：方向变无效，立即停机 */
    dir_input_bits = 0;
    for(i = 0; i < 12; i++) user_direction_handle();
    CHECK(!user_list.dir_valid);
    user_state_control();
    CHECK(user_list.user_state == TOOL_STOP && !user_list.flag.bits.gToolEn);

    /* 恢复单一方向后仍须松开电位器再按下才能重启 */
    dir_input_bits = RWD_PIN;
    for(i = 0; i < 12; i++) user_direction_handle();
    CHECK(user_list.dir_valid && user_list.dir_retrig_need && !user_list.direction_ready);
    user_state_control();
    CHECK(user_list.user_state != TOOL_RUN && !user_list.flag.bits.gToolEn);
    user_list.flag.bits.gToolTrigger = 0;
    user_direction_handle();
    user_list.flag.bits.gToolTrigger = 1;
    user_direction_handle();
    user_state_control();
    CHECK(user_list.user_state == TOOL_RUN && user_list.flag.bits.gToolEn);
}

uint32_t run_tests(void)
{
#define RUN(number, function) do { test_case = number; function(); if(test_failure_line) return test_failure_line; } while(0)
    RUN(1, test_handshake_and_uart);
    RUN(2, test_framing);
    RUN(3, test_handshake_required);
    RUN(4, test_bms_faults_and_recovery);
    RUN(5, test_timeout_and_stale_handshake);
    RUN(6, test_overflow_echo_and_tx_abort);
    RUN(7, test_restart_sleep_and_unrelated_faults);
    RUN(8, test_motor_start_boundaries);
    RUN(9, test_receive_during_reply_wait);
    RUN(10, test_invalid_runtime_does_not_refresh);
    RUN(11, test_direction_interlock);
    return 0;
}
