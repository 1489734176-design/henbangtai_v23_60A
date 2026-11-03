/**
 * @file     drv_sqrt.h
 * @author   Motor TEAM
 * @brief    This file provides all the driver functions prototypes for the PWM.
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
#ifndef __DRV_SQRT_H
#define __DRV_SQRT_H

/** Files includes */
#include "HAL_device.h"
#include "HAL_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_SQRT
 * @{
 */

/** @brief 硬件平方根加速器初始化 */
extern void Drv_Sqrt_Init(void);
/** @brief 硬件平方根运算（输入32位整数，输出16位平方根结果） */
extern int16_t Hw_Sqrt(int32_t m);

/**
  * @}
*/

/**
  * @}
*/

#endif
