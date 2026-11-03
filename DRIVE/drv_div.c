/**
 * @file     drv_div.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the div.
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
#define _DRV_DIV_C_

/** Files includes */
#include "drv_div.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_DIV
 * @{
 */

/**
 * @brief  硬件除法器有符号模式初始化
 * @param  None
 * @retval None
 * @note   清除USIGN位，配置为有符号除法模式
 */
static void HDIV_SignInit(void)
{
    HWDIV->CR &= (~HWDIV_CR_USIGN) ;
}

/**
 * @brief  硬件除法器初始化
 * @param  None
 * @retval None
 * @note   使能HWDIV时钟，配置为有符号除法模式
 */
void Drv_Hwdiv_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_HWDIV_Msk, ENABLE);

    HDIV_SignInit();
}

/**
 * @brief  硬件除法运算
 * @param  m: 被除数（有符号32位整数）
 * @param  n: 除数（有符号32位整数）
 * @retval 商值（有符号32位整数），除数为0或被除数为0时返回0
 * @note   使用硬件除法器HWDIV执行除法，结果从QUOTR寄存器读取
 */
int32_t Division(int32_t m, int32_t n)
{
    if(n == 0)
    {
        return 0;
    }
    if(m == 0)
    {
        return 0;
    }

    HWDIV->DVDR = m;    // 写入被除数
    HWDIV->DVSR = n;    // 写入除数
    return (HWDIV->QUOTR);  // 读取商值
}

/**
  * @}
*/

/**
  * @}
*/
