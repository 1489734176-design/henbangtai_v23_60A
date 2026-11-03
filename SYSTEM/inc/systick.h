/**
 * @file     systick.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides config and functions for the systick.
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
#ifndef __SYSTICK_H
#define __SYSTICK_H

/** Files includes */
#include "HAL_conf.h"
#include "mm32_device.h"

/**
 * @addtogroup MM32_System_Layer
 * @{
 */

/**
 * @addtogroup Systick
 * @{
 */


/* SysTick初始化 */
extern void Systick_Init(uint32_t ticks);

/* 获取SysTick计数 */
extern uint32_t Get_Systick_Cnt(void);

/* 获取SysTick当前计数值 */
extern uint32_t Get_Systick_Val(void);

/* 获取SysTick溢出标志 */
extern uint32_t Get_Over_Flag(void);

/* 清除SysTick溢出标志 */
extern void Clear_Over_Flag(void);

/* SysTick中断计数递增 */
extern void Inc_Systicks(void);

/* 毫秒级延时 */
extern void Systick_Delay(volatile uint32_t Delay);

/* 微秒级延时 */
void systick_delay_us(u32 n_us);
/**
  * @}
*/

/**
  * @}
*/

#endif
