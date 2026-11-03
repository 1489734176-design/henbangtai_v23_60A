/**
 * @file     drv_sqrt.c
 * @author   Motor TEAM
 * @brief    This file provides all the driver functions for the SQRT.
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
#define _DRV_SQRT_C_

/** Files includes */
#include "drv_sqrt.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_SQRT
 * @{
 */

/**
 * @brief  硬件平方根加速器初始化
 * @param  None
 * @retval None
 * @note   使能HWSQRT时钟
 */
void Drv_Sqrt_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_HWSQRT, ENABLE);
}

/**
 * @brief  硬件平方根运算
 * @param  m: 被开方数（32位整数）
 * @retval 平方根结果（16位整数），输入为0时返回0
 * @note   使用硬件平方根加速器HWSQRT执行开方运算，结果从RESULT寄存器读取
 */
int16_t Hw_Sqrt(int32_t m)
{
    if(m == 0)
    {
        return 0;
    }

    SQRT->SQR = m;          // 写入被开方数
    return (SQRT->RESULT);  // 读取平方根结果
}
/**
  * @}
*/

/**
  * @}
*/
