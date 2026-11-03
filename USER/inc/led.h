#ifndef __LED_H
#define __LED_H
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/* 新板：PB8=LED行1/DIRCTION，PA14=LED行2/KEY。
 * 阴极COM1/2/3/4=PA13/PA0/PA10/PB9；PA12独立照明。
 * 逻辑灯1..3对应物理LED4..6（电量/报警），逻辑灯4..6对应物理LED1..3（挡位）。 */
#define LED_LIGHT_RCC_CLOCKGPIO (RCC_AHBENR_GPIOA_Msk)
#define LED_LIGHT_PORT          GPIOA
#define LED_LIGHT_PIN           GPIO_Pin_12
#define LED_ROW1_PORT           GPIOB
#define LED_ROW1_PIN            GPIO_Pin_8
#define LED_ROW2_PORT           GPIOA
#define LED_ROW2_PIN            GPIO_Pin_14
#define LED_COM1_PIN            GPIO_Pin_13
#define LED_COM2_PIN            GPIO_Pin_0
#define LED_COM3_PIN            GPIO_Pin_10
#define LED_COM4_PIN            GPIO_Pin_9

typedef union
{
    uint8_t Word;
    struct
    {
        uint8_t gB0:1, gB1:1, gB2:1, gB3:1, gB4:1, gB5:1, gB6:1, gB7:1;
    } bits;
} BITS_UNION;
extern volatile BITS_UNION ledData;
#define LED1          ledData.bits.gB1
#define LED1_ON       (LED1=1)
#define LED1_OFF      (LED1=0)
#define LED1_FLASH    (LED1=!LED1)
#define LED2          ledData.bits.gB2
#define LED2_ON       (LED2=1)
#define LED2_OFF      (LED2=0)
#define LED2_FLASH    (LED2=!LED2)
#define LED3          ledData.bits.gB3
#define LED3_ON       (LED3=1)
#define LED3_OFF      (LED3=0)
#define LED3_FLASH    (LED3=!LED3)
#define LED4          ledData.bits.gB4
#define LED4_ON       (LED4=1)
#define LED4_OFF      (LED4=0)
#define LED4_FLASH    (LED4=!LED4)
#define LED5          ledData.bits.gB5
#define LED5_ON       (LED5=1)
#define LED5_OFF      (LED5=0)
#define LED5_FLASH    (LED5=!LED5)
#define LED6          ledData.bits.gB6
#define LED6_ON       (LED6=1)
#define LED6_OFF      (LED6=0)
#define LED6_FLASH    (LED6=!LED6)
#define LED7          ledData.bits.gB7
#define LED7_ON       (LED7=1)
#define LED7_OFF      (LED7=0)
#define LED7_FLASH    (LED7=!LED7)

#define LED_ERROR_ON()  do { LED1_ON; LED2_ON; LED3_ON; } while(0)
#define LED_ERROR_OFF() do { LED1_OFF; LED2_OFF; LED3_OFF; } while(0)

/* 1..12保持20260909参考程序编号；新增报警在12之后，兜底最后。 */
#define LED_FAULT_HW_OCP       (1)
#define LED_FAULT_BAT_TEMP     (2)
#define LED_FAULT_MOS_TEMP     (3)
#define LED_FAULT_BLOCK        (4)
#define LED_FAULT_SW_OCP       (5)
#define LED_FAULT_UVP          (6)
#define LED_FAULT_FLY_MACHINE  (7)
#define LED_FAULT_OVP          (8)
#define LED_FAULT_MOS_BRIDGE   (9)
#define LED_FAULT_LACK_PHASE   (10)
#define LED_FAULT_LOSE_STEP    (11)
#define LED_FAULT_BAT_COM      (12) /* 新板无通信，保留原编号 */
#define LED_FAULT_IBUS_OCP4    (13)
#define LED_FAULT_OTHER        (14)

typedef struct
{
    uint8_t LedDeal;
    uint8_t LedRunMode;
    uint16_t LedRunTimes;
    uint16_t LedRunCnt_Once;
    uint8_t gSocUpdate;
} User_LED_Control_t;

extern User_LED_Control_t user_led_control;
void led_init(void);
void LED_Scan(void);
void UO_Led_Handler(void);
void LedSoc_Run(uint32_t volt_vx100, uint8_t display_en, uint8_t fast_mode);
uint8_t LedError_Control(void);
void LedError_Run(void);
#endif
