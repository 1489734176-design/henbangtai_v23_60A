/**
 * @file     systick.c
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
#define _SYSTICK_C_

/** Files includes */

#include "systick.h"

/**
 * @addtogroup MM32_System_Layer
 * @{
 */

/**
 * @addtogroup Systick
 * @{
 */



/* SysTick计数器，每递增一次代表一个SysTick周期 */
static volatile uint32_t _u32Systicks = 0;
volatile uint32_t PLATFORM_DelayTick;

/**
* @brief    : This function will be used to initialize SysTick.
* @param    : ticks
* SystemFrequency / 1000    1ms raise interrupt
* SystemFrequency / 100000	 10us raise interrupt
* SystemFrequency / 1000000 1us raise interrupt
* @retval   : None
*/
void Systick_Init(uint32_t ticks)
{
    SysTick_Config(ticks);
	/* 选择外部时钟源 */
	SysTick_CLKSourceConfig(SysTick_CLKSource_EXTCLK);
	/* 设置SysTick中断优先级为最高 */
	NVIC_SetPriority(SysTick_IRQn, 0x0);
}

/* SysTick中断计数递增 */
void Inc_Systicks(void)
{
    _u32Systicks ++;
}

/* 获取SysTick计数 */
uint32_t Get_Systick_Cnt(void)
{
    return _u32Systicks;
}

/* 获取SysTick当前计数值 */
uint32_t Get_Systick_Val(void)
{
    return SysTick->VAL;
}

/* 清除SysTick溢出标志 */
void Clear_Over_Flag(void)
{
    SysTick->CTRL;
}

/* 获取SysTick溢出标志 */
uint32_t Get_Over_Flag(void)
{
    return SysTick->CTRL & (1 << 16);
}

/* 毫秒级延时 */
void Systick_Delay(volatile uint32_t Delay)
{
    uint32_t tickstart = 0;
    tickstart = Get_Systick_Cnt();
    while ((Get_Systick_Cnt() - tickstart) < Delay) {
    }
}

/* 挂起SysTick中断 */
void Suspend_Systicks(void)
{
    CLEAR_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

/* 恢复SysTick中断 */
void Resume_Systicks(void)
{
    SET_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

/* 微秒级延时（基于SysTick当前值计算） */
void systick_delay_us(u32 n_us)
{
	u32 systick_count_temp;
	u32 xus_count;
	u32 diff_count=0;
	/* 将微秒数转换为SysTick计数值（约15个计数/us） */
	xus_count=((15*n_us)>>1);
	systick_count_temp=SysTick->VAL;
	while(diff_count<xus_count)
	{
		diff_count=((systick_count_temp-SysTick->VAL)&0x00ffffff);
	}
}


/**
  * @}
*/

/**
  * @}
*/
