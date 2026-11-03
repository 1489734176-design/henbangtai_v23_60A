/**
 * @file     drv_led.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the led.
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
#define _DRV_LED_C_

/** Files includes */
#include "drv_led.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_LED
 * @{
 */

/**
 * @brief  点亮LED（置位指定GPIO引脚）
 * @param  pGpio: GPIO端口指针
 * @param  u16Pin: 引脚号
 * @retval None
 * @note   通过BSRR寄存器置位引脚，高电平有效
 */
void Led_On(GPIO_TypeDef* pGpio,uint16_t u16Pin)
{
    pGpio->BSRR = u16Pin;
}

/**
 * @brief  熄灭LED（复位指定GPIO引脚）
 * @param  pGpio: GPIO端口指针
 * @param  u16Pin: 引脚号
 * @retval None
 * @note   通过BRR寄存器复位引脚，低电平有效
 */
void Led_Off(GPIO_TypeDef* pGpio,uint16_t u16Pin)
{
    pGpio->BRR = u16Pin;
}

/**
 * @brief  翻转LED状态（取反指定GPIO引脚）
 * @param  pGpio: GPIO端口指针
 * @param  u16Pin: 引脚号
 * @retval None
 * @note   通过ODR寄存器异或操作翻转引脚状态
 */
void Led_Toggle(GPIO_TypeDef* pGpio,uint16_t u16Pin)
{
    pGpio->ODR ^= u16Pin;
}

/**
  * @}
*/

/**
  * @}
*/
