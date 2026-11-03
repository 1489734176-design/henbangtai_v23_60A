#ifndef TEST_HW_H
#define TEST_HW_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t s16;
typedef int32_t s32;
#define __IO volatile

typedef enum { DISABLE = 0, ENABLE = 1 } FunctionalState;
typedef struct { volatile uint32_t SR, DR, CR1, CR3; } USART_TypeDef;
extern USART_TypeDef test_usart;
extern uint8_t test_com_en;
extern uint32_t PI_sum_i, PI_p, pwm_duty_aim_PI_OUT;
#define USART1 (&test_usart)
#define USART1_IRQn 0
#define USART_FLAG_PE (1u << 0)
#define USART_FLAG_FE (1u << 1)
#define USART_FLAG_NF (1u << 2)
#define USART_FLAG_ORE (1u << 3)
#define USART_FLAG_RXNE (1u << 5)
#define USART_FLAG_TC (1u << 6)
#define USART_FLAG_TXE (1u << 7)
#define USART_IT_RXNE (1u << 5)
#define USART_IT_TC (1u << 6)
#define USART_IT_TXE (1u << 7)
#define USART_IT_PE (1u << 8)
#define USART_IT_ERR (1u << 0)
#define USART_CR1_RE_Msk (1u << 2)
#define COM_EN_HIGH() (test_com_en = 1)
#define COM_EN_LOW() (test_com_en = 0)
#define TIM1 ((void *)1)
#define TIM14 ((void *)14)
#define TIM_FLAG_Update 1
#define EN_PORT ((void *)1)
#define EN_PIN 1
#define DIR_PORT ((void *)2)
#define DIR_PIN (1u << 8)
#define RWD_PIN (1u << 9)
#define LIGHT_OFF 0

extern struct test_light { uint8_t state; } light_ctrl;
uint32_t __get_PRIMASK(void);
void __disable_irq(void);
void __set_PRIMASK(uint32_t value);
void NVIC_DisableIRQ(int irq);
void NVIC_EnableIRQ(int irq);
void NVIC_ClearPendingIRQ(int irq);
void Bsp_Com_Tx_Mode(uint8_t enabled);
void Bsp_Usart_Init(void);
void USART_ITConfig(USART_TypeDef *uart, uint32_t flags, FunctionalState state);
uint16_t USART_ReceiveData(USART_TypeDef *uart);
void USART_SendData(USART_TypeDef *uart, uint16_t data);
void USART_ClearFlag(USART_TypeDef *uart, uint32_t flags);
void USART_Cmd(USART_TypeDef *uart, FunctionalState state);
uint32_t Get_ChipsetUIDw0(void);
void GPIO_SetBits(void *gpio, uint16_t pin);
void GPIO_ResetBits(void *gpio, uint16_t pin);
uint16_t GPIO_ReadInputData(void *gpio);
void TIM_CtrlPWMOutputs(void *timer, FunctionalState state);
void TIM_Cmd(void *timer, FunctionalState state);
void TIM_ClearFlag(void *timer, uint32_t flags);
void Stop_Motor(void);
void Brake_Motor(void);
void pwm_set(void);
void PwmDutyUpdate(uint16_t duty);
void UVWL_HOffLPwm(void);

#endif
