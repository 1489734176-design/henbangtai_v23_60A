/**
 * @file     drv_iwdg.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the IWDG.
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
#define _DRV_IWDG_C_

/** Files includes */
#include "drv_iwdg.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_IWDG
 * @{
 */

/**
 * @brief    : This function describes the underlying configuration of the IWDG.
 * @param    : None
 * @retval   : None
 * @note     配置独立看门狗：LSI时钟32分频，重载值0xFF，
 *           超时时间约1秒（LSI≈32.768kHz，32分频≈1kHz，256计数≈0.256秒）
 */
void Drv_Iwdg_Init(void)
{
    /** Start the internal low-speed clock and wait for the clock to be ready */
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    /** Setting Clock Pre-division Frequency */
    PVU_CheckStatus();              // 等待预分频器更新完成
    IWDG_WriteAccessCmd(0x5555);    // 使能IWDG寄存器写访问
    IWDG_SetPrescaler(IWDG_Prescaler_32);  // 预分频32，时钟≈1kHz

    /** Setting overload register values */
    RVU_CheckStatus();              // 等待重载值更新完成
    IWDG_WriteAccessCmd(0x5555);    // 使能IWDG寄存器写访问
    IWDG_SetReload(0xff);           // 重载值256

    /** Loading and Enabling Counter */
    IWDG_ReloadCounter();           // 重载计数器（喂狗）
    IWDG_Enable();                  // 使能独立看门狗
}

/**
  * @}
*/

/**
  * @}
*/
