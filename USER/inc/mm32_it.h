/** 
 * @file     mm32_it.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the functions prototypes for the Interrupt interface.
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

#ifndef __MM32_IT_H
#define __MM32_IT_H

/** Files includes */
#include "main.h"

/** 
 * @addtogroup MM32_User_Layer
 * @{
 */

/** 
 * @addtogroup User_MM32_IT
 * @{
 */

// 中断初始化函数
extern void Interrupt_Init(void);  // 初始化所有中断（ADC、TIM等）

// NVIC 配置（实现在 mm32_it.c）
extern void NVIC_Configure(uint8_t ch, uint8_t pri);


/**
  * @}
*/

/**
  * @}
*/


#endif
