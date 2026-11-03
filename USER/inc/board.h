/**
 * @file     board.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the functions prototypes for the board level support package.
 *
 * @attention
 *
 * THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
 * CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
 * TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
 * HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
 * CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
 *
 * <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
 */

/** Define to prevent recursive inclusion */
#ifndef __BOARD_H
#define __BOARD_H

/** Files includes */
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/** Interrupt priorities */
/**
 * The order of priority decreases from small to large
 * 0 : highest priority-first
 */
#define SYSTICK_INTERRUPT           (2)   // SysTick中断优先级
#define TIM1_UPDATE_INTERRUPT       (0)   // TIM1更新中断优先级（最高）
#define ADC1_INTERRUPT              (0)   // ADC1中断优先级（最高）
#define TIM14_INTERRUPT             (0)   // TIM14中断优先级（最高）

// BLDC六步换相步序定义
#define STEP_5_UV 		5   // U+ W- 导通
#define STEP_4_UW 		4   // U+ V- 导通 
#define STEP_6_VW 		6   // V+ U- 导通 
#define STEP_2_VU 		2   // V+ W- 导通 
#define STEP_3_WU 		3   // W+ U- 导通 
#define STEP_1_WV 		1   // W+ V- 导通 

/** ADC interface */
// BEMF ADC通道定义
#define BEMF_U_CHANNEL              ADC_Channel_9 	// BEMF U相
#define BEMF_U_RANK                 (0)
#define BEMF_V_CHANNEL              ADC_Channel_8	// BEMF V相
#define BEMF_V_RANK                 (1)
#define BEMF_W_CHANNEL              ADC_Channel_7   // BEMF W相
#define BEMF_W_RANK                 (2)

#define OP_O_CHANNEL                ADC_Channel_5   // 运放输出（峰值电流）
#define OP_O_RANK                   (3)
#define ISUMAVG_CHANNEL        			ADC_Channel_4  	
#define ISUMAVG_RANK             		(4)

#define VR_CHANNEL                  ADC_Channel_0   // 扳机电压/调速信号	
#define VR_RANK                     (5)
#define VBUS_CHANNEL                ADC_Channel_1   // 母线电压	
#define VBUS_RANK                   (6)
#define T_BAT_CHANNEL                ADC_Channel_2   // PA3：预留电池温度采样
#define T_BAT_RANK                   (7)

#define T_MOS_CHANNEL                ADC_Channel_3   // MOS温度NTC	
#define T_MOS_RANK                   (8)



#define FLY_CHANNEL                ADC_Channel_0// 无需PWM触发，单独采集
//#define T_MOS_RANK                   (8)

#define ADC_GPIO_CLK               (RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk)

#define ADC_TRIG_O_PORT            	GPIOA
#define ADC_TRIG_O_PIN            	GPIO_Pin_12
#define ADC_TRIG_O_PIN_SRC        	GPIO_PinSource12
#define ADC_TRIG_O_PIN_AF         	GPIO_AF_6

#define VR_PORT                     GPIOA
#define VR_PIN                      GPIO_Pin_1

#define VBUS_PORT                   GPIOA
#define VBUS_PIN                    GPIO_Pin_2

#define ISUMAVG_PORT             		GPIOA
#define ISUMAVG_PIN             		GPIO_Pin_5

#define IRMS_PORT                   GPIOB
#define IRMS_PIN                    GPIO_Pin_0


#define EN_PIN_RCC_CLOCKGPIO          (RCC_AHBENR_GPIOA_Msk)
#define EN_PORT                      GPIOA
#define EN_PIN                       GPIO_Pin_11
#define EN_PIN_SRC                   GPIO_PinSource11
#define EN_PIN_AF                    GPIO_AF_1



#define DIR_RCC_CLOCKGPIO          (RCC_AHBENR_GPIOB_Msk)
#define DIR_PORT                   GPIOB
// PB8 与 LED 行1复用；仅在扫描输入窗口读取，低=正转，高=反转。
#define DIR_PIN                    GPIO_Pin_8

#define KEY_RCC_CLOCKGPIO          (RCC_AHBENR_GPIOA_Msk)
#define KEY_PORT                   GPIOA
// 按键引脚
#define KEY_PIN                    GPIO_Pin_14

#define KEY_PIN_SRC                GPIO_PinSource14
#define KEY_PIN_AF                 GPIO_AF_1

#define T_BAT_PORT                 GPIOA
#define T_BAT_PIN                  GPIO_Pin_3

#define T_MOS_PORT                   GPIOA
// MOS温度NTC引脚
#define T_MOS_PIN                    GPIO_Pin_4

/* COM 单线半双工：PA11 高经 Q13/Q14 拉低 COM，GPIO 低释放；Q12 将 COM 反相到 RX。
   PA11=USART1_TX，PA12=USART1_RX，PA14=COM_EN 使能上拉。 */
#define COM_TX_RCC_CLOCKGPIO		(RCC_AHBENR_GPIOA_Msk)
#define COM_TX_PORT					GPIOA
#define COM_TX_PIN					GPIO_Pin_11
#define COM_TX_PIN_SRC				GPIO_PinSource11
#define COM_TX_PIN_AF				GPIO_AF_5

#define COM_RX_PORT					GPIOA
#define COM_RX_PIN					GPIO_Pin_12
#define COM_RX_PIN_SRC				GPIO_PinSource12
#define COM_RX_PIN_AF				GPIO_AF_5

#define COM_EN_RCC_CLOCKGPIO		(RCC_AHBENR_GPIOA_Msk)
#define COM_EN_PORT					GPIOA
#define COM_EN_PIN					GPIO_Pin_14
#define COM_EN_PIN_SRC				GPIO_PinSource14
#define COM_EN_PIN_AF				GPIO_AF_1
#define COM_EN_HIGH()				(COM_EN_PORT->BSRR = COM_EN_PIN)
#define COM_EN_LOW()				(COM_EN_PORT->BRR  = COM_EN_PIN)

/** BEMF_ADC */
#define BEMF_U_PORT                 GPIOA
#define BEMF_U_PIN                  GPIO_Pin_9

#define BEMF_V_PORT                 GPIOA
#define BEMF_V_PIN                  GPIO_Pin_8

#define BEMF_W_PORT                 GPIOB
#define BEMF_W_PIN                  GPIO_Pin_2

/** BEMF_COMP */
#define COMP_SNS_GPIO_CLK       		(RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk)	// COMP Sensorless BEMF GPIO Port

/** BEMF_COMP */
#define COMP_BEMF_U_PORT            GPIOA
#define COMP_BEMF_U       					GPIO_Pin_9  	

#define COMP_BEMF_V_PORT            GPIOA
#define COMP_BEMF_V       					GPIO_Pin_8  	

#define COMP_BEMF_W_PORT            GPIOB
#define COMP_BEMF_W       					GPIO_Pin_2  

#define COMP_BEMF_COM_PORT          GPIOB
#define COMP_BEMF_COM     					GPIO_Pin_1

/** COMP interface */
#define COMP_NUMBER                 COMP1
#define COMP_NON_INVERTING          COMP_NonInvertingInput_IO3  
#define COMP_INVERTING              COMP_InvertingInput_IO3
#define COMP_CRV_VOLTAGE_SELECT     98													//  (x+1)/256		//6mos:98	12mos:135

#define COMP_GPIO_CLK               (RCC_AHBENR_GPIOB_Msk)
#define COMP_INP_PORT               GPIOB
#define COMP_INP_PIN                GPIO_Pin_0

/** OPAMP interface */
#define OPAMP_GPIO_CLK              (RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk)
#define OPAMP1_INP_PORT             GPIOA
#define OPAMP1_INP_PIN              GPIO_Pin_6
#define OPAMP1_INM_PORT             GPIOA
#define OPAMP1_INM_PIN              GPIO_Pin_7
#define OPAMP1_OUT_PORT             GPIOB
#define OPAMP1_OUT_PIN            	GPIO_Pin_0

#define OPAMP2_INP_PORT             GPIOA
#define OPAMP2_INP_PIN              GPIO_Pin_3
#define OPAMP2_INM_PORT             GPIOA
#define OPAMP2_INM_PIN              GPIO_Pin_4
#define OPAMP2_OUT_PORT             GPIOA
#define OPAMP2_OUT_PIN            	GPIO_Pin_5

/** DEBUG_IO interface */
#define DEBUG_IO_RCC_CLOCKGPIO      (RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk)	// Debug GPIO clock
#define DEBUG_IO1_PORT               GPIOA																	// Debug GPIO Port
#define DEBUG_IO1_PIN                GPIO_Pin_5														// Debug GPIO Pin
#define DEBUG_IO1_LOW()              DEBUG_IO1_PORT->BRR = DEBUG_IO1_PIN		// Debug GPIO Output low
#define DEBUG_IO1_HIGH()             DEBUG_IO1_PORT->BSRR = DEBUG_IO1_PIN		// Debug GPIO Output high
#define DEBUG_IO1_TOGGLE()           DEBUG_IO1_PORT->ODR ^= DEBUG_IO1_PIN		// Debug GPIO toggle

#define DEBUG_IO2_PORT               GPIOA																	// Debug GPIO Port
#define DEBUG_IO2_PIN                GPIO_Pin_12														// Debug GPIO Pin
#define DEBUG_IO2_LOW()              DEBUG_IO2_PORT->BRR = DEBUG_IO2_PIN		// Debug GPIO Output low
#define DEBUG_IO2_HIGH()             DEBUG_IO2_PORT->BSRR = DEBUG_IO2_PIN		// Debug GPIO Output high
#define DEBUG_IO2_TOGGLE()           DEBUG_IO2_PORT->ODR ^= DEBUG_IO2_PIN		// Debug GPIO toggle

/** LED interface 
#define LED_RCC_CLOCKGPIO           (RCC_AHBENR_GPIOB_Msk)

#define LED1_PORT                   GPIOB
#define LED1_PIN                    GPIO_Pin_8
#define LED1_ON()                   LED1_PORT->BRR = LED1_PIN
#define LED1_OFF()                  LED1_PORT->BSRR = LED1_PIN
#define LED1_TOGGLE()               LED1_PORT->ODR ^= LED1_PIN

#define LED2_PORT                   GPIOA
#define LED2_PIN                    GPIO_Pin_11
#define LED2_ON()                   LED2_PORT->BRR  = LED2_PIN
#define LED2_OFF()                  LED2_PORT->BSRR = LED2_PIN
#define LED2_TOGGLE()               LED2_PORT->ODR ^= LED2_PIN

#define LED3_PORT                   GPIOA
#define LED3_PIN                    GPIO_Pin_12
#define LED3_ON()                   LED3_PORT->BRR  = LED3_PIN
#define LED3_OFF()                  LED3_PORT->BSRR = LED3_PIN
#define LED3_TOGGLE()               LED3_PORT->ODR ^= LED3_PIN

#define LED_On_PORT               	GPIOA
#define LED_On_PIN                	GPIO_Pin_1
#define LED_On_OFF()              	LED_On_PORT->BRR  = LED_On_PIN
#define LED_On_ON()               	LED_On_PORT->BSRR = LED_On_PIN
#define LED_On_TOGGLE()            	LED_On_PORT->ODR ^= LED_On_PIN
*/

/** FG interface */
#define FG_RCC_CLOCKGPIO           	(RCC_AHBENR_GPIOA_Msk)
#define FG_PORT                   	GPIOA
// FG信号输出引脚
#define FG_PIN                    	GPIO_Pin_9
#define FG_HIGH()                 	FG_PORT->BRR = FG_PIN
#define FG_LOW()                  	FG_PORT->BSRR = FG_PIN
#define FG_TOGGLE()               	FG_PORT->ODR ^= FG_PIN

/** PowerOn interface */
#define PowerOn_RCC_CLOCKGPIO     	(RCC_AHBENR_GPIOB_Msk)
#define PowerOn_PORT              	GPIOB
#define PowerOn_PIN               	GPIO_Pin_8
#define PowerOn_HIGH()             	PowerOn_PORT->BSRR  = PowerOn_PIN
#define PowerOn_LOW()              	PowerOn_PORT->BRR = PowerOn_PIN
#define PowerOn_TOGGLE()           	PowerOn_PORT->ODR ^= PowerOn_PIN

/** Key interface */
#define RF_RCC_CLOCKGPIO            (RCC_AHBENR_GPIOB_Msk)
#define RF_PORT                   	GPIOB
#define RF_PIN                    	GPIO_Pin_9

/** Key interface */
#define KEY1_PORT                  KEY_PORT
#define KEY1_PIN                   KEY_PIN


/** Key interface */





/** PWM interface */
// PWM低侧驱动方式选择
#define PWM_L_USE_IO   0  // 低侧使用IO直接驱动
#define PWM_L_USE_TIM  1  // 低侧使用定时器PWM驱动

#define BLDC1_GPIO_CLK               (RCC_AHBENR_GPIOA_Msk | RCC_AHBENR_GPIOB_Msk)

#if(0)

#define BLDC1_UH_PORT                GPIOB
#define BLDC1_UH_PIN                 GPIO_Pin_7
#define BLDC1_VH_PORT                GPIOB
#define BLDC1_VH_PIN                 GPIO_Pin_5
#define BLDC1_WH_PORT                GPIOB
#define BLDC1_WH_PIN                 GPIO_Pin_3

#define BLDC1_UL_PORT                GPIOB
#define BLDC1_UL_PIN                 GPIO_Pin_6
#define BLDC1_VL_PORT                GPIOB
#define BLDC1_VL_PIN                 GPIO_Pin_4
#define BLDC1_WL_PORT                GPIOA
#define BLDC1_WL_PIN                 GPIO_Pin_15

//#define BLDC1_BKP_PORT               GPIOA
//#define BLDC1_BKP_PIN                GPIO_Pin_6

#define BLDC1_UH_PIN_SRC             GPIO_PinSource7
#define BLDC1_VH_PIN_SRC             GPIO_PinSource5
#define BLDC1_WH_PIN_SRC             GPIO_PinSource3
#define BLDC1_UL_PIN_SRC             GPIO_PinSource6
#define BLDC1_VL_PIN_SRC             GPIO_PinSource4
#define BLDC1_WL_PIN_SRC             GPIO_PinSource15
//#define BLDC1_BKP_PIN_SRC            GPIO_PinSource6

#define BLDC1_UH_PIN_AF              GPIO_AF_5
#define BLDC1_VH_PIN_AF              GPIO_AF_7
#define BLDC1_WH_PIN_AF              GPIO_AF_7
#define BLDC1_UL_PIN_AF              GPIO_AF_1
#define BLDC1_VL_PIN_AF              GPIO_AF_7
#define BLDC1_WL_PIN_AF              GPIO_AF_5
//#define BLDC1_BKP_PIN_AF             GPIO_AF_2

#endif


#if(1)
#define BLDC1_UH_PORT                GPIOB
#define BLDC1_UH_PIN                 GPIO_Pin_4
#define BLDC1_VH_PORT                GPIOB
#define BLDC1_VH_PIN                 GPIO_Pin_3
#define BLDC1_WH_PORT                GPIOA
#define BLDC1_WH_PIN                 GPIO_Pin_15

#define BLDC1_UL_PORT                GPIOB
#define BLDC1_UL_PIN                 GPIO_Pin_7
#define BLDC1_VL_PORT                GPIOB
#define BLDC1_VL_PIN                 GPIO_Pin_6
#define BLDC1_WL_PORT                GPIOB
#define BLDC1_WL_PIN                 GPIO_Pin_5

//#define BLDC1_BKP_PORT               GPIOA
//#define BLDC1_BKP_PIN                GPIO_Pin_6

#define BLDC1_UH_PIN_SRC             GPIO_PinSource4
#define BLDC1_VH_PIN_SRC             GPIO_PinSource3
#define BLDC1_WH_PIN_SRC             GPIO_PinSource15
#define BLDC1_UL_PIN_SRC             GPIO_PinSource7
#define BLDC1_VL_PIN_SRC             GPIO_PinSource6
#define BLDC1_WL_PIN_SRC             GPIO_PinSource5
//#define BLDC1_BKP_PIN_SRC            GPIO_PinSource6

#define BLDC1_UH_PIN_AF              GPIO_AF_3
#define BLDC1_VH_PIN_AF              GPIO_AF_1
#define BLDC1_WH_PIN_AF              GPIO_AF_1
#define BLDC1_UL_PIN_AF              GPIO_AF_6
#define BLDC1_VL_PIN_AF              GPIO_AF_0
#define BLDC1_WL_PIN_AF              GPIO_AF_6



#endif

#if(0)
#define BLDC1_UH_PORT                GPIOB
#define BLDC1_UH_PIN                 GPIO_Pin_9
#define BLDC1_VH_PORT                GPIOB
#define BLDC1_VH_PIN                 GPIO_Pin_8
#define BLDC1_WH_PORT                GPIOB
#define BLDC1_WH_PIN                 GPIO_Pin_7

#define BLDC1_UL_PORT                GPIOB
#define BLDC1_UL_PIN                 GPIO_Pin_6
#define BLDC1_VL_PORT                GPIOB
#define BLDC1_VL_PIN                 GPIO_Pin_5
#define BLDC1_WL_PORT                GPIOB
#define BLDC1_WL_PIN                 GPIO_Pin_4

//#define BLDC1_BKP_PORT               GPIOA
//#define BLDC1_BKP_PIN                GPIO_Pin_6

#define BLDC1_UH_PIN_SRC             GPIO_PinSource9
#define BLDC1_VH_PIN_SRC             GPIO_PinSource8
#define BLDC1_WH_PIN_SRC             GPIO_PinSource7
#define BLDC1_UL_PIN_SRC             GPIO_PinSource6
#define BLDC1_VL_PIN_SRC             GPIO_PinSource5
#define BLDC1_WL_PIN_SRC             GPIO_PinSource4
//#define BLDC1_BKP_PIN_SRC            GPIO_PinSource6

#define BLDC1_UH_PIN_AF              GPIO_AF_0
#define BLDC1_VH_PIN_AF              GPIO_AF_3
#define BLDC1_WH_PIN_AF              GPIO_AF_3
#define BLDC1_UL_PIN_AF              GPIO_AF_1
#define BLDC1_VL_PIN_AF              GPIO_AF_4
#define BLDC1_WL_PIN_AF              GPIO_AF_0



#endif

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Bsp
 * @{
 */


extern void Bsp_Gpio_Init(void);
extern void Peripheral_Init(void);
void Board_ADC_Init(void);
void Bsp_Adc_Init(void);
void En_Port_init(void);
void Bsp_Com_init(void);                       /* 通信端口(PA11/12/14)初始化 */
void Bsp_Com_Tx_Mode(uint8_t uart_enable);     /* COM_TX 引脚 UART/GPIO 模式切换 */
void Bsp_Usart_Init(void);                     /* USART1 初始化(4800 8N1) */

/**
  * @}
*/

/**
  * @}
*/


#endif
