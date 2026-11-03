/**
 * @file     drv_div.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions prototypes for the DIV.
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
#ifndef __DRV_DIV_H
#define __DRV_DIV_H

/** Files includes */
#include "mm32_device.h"
#include "HAL_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_DIV
 * @{
 */

/** @brief 是否使用硬件除法器：1=使用HWDIV，0=使用软件除法 */
#define USE_HWDIV       1

extern void Drv_Hwdiv_Init(void);
extern int32_t Division(int32_t m, int32_t n);

#if USE_HWDIV
/** @brief 除法宏：使用硬件除法器 */
#define DIVISION(a,b)   Division(a,b)
#else
/** @brief 除法宏：使用软件除法 */
#define DIVISION(a,b)   (a/b)
#endif

/**
  * @}
*/

/**
  * @}
*/

#endif
