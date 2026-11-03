/**
 * @file     drv_led.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions prototypes for the led.
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
#ifndef __DRV_LED_H
#define __DRV_LED_H

/** Files includes */
#include "HAL_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_LED
 * @{
 */

/** @brief 点亮LED（置位指定GPIO引脚） */
extern void Led_On(GPIO_TypeDef* pGpio,uint16_t u16Pin);
/** @brief 熄灭LED（复位指定GPIO引脚） */
extern void Led_Off(GPIO_TypeDef* pGpio,uint16_t u16Pin);
/** @brief 翻转LED状态（取反指定GPIO引脚） */
extern void Led_Toggle(GPIO_TypeDef* pGpio,uint16_t u16Pin);


/**
  * @}
*/

/**
  * @}
*/

#endif
