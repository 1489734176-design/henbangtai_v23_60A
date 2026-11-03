/**
 * @file     drv_iwdg.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions prototypes for the IWDG.
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
#ifndef __DRV_IWDG_H
#define __DRV_IWDG_H

/** Files includes */
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_IWDG
 * @{
 */

/** You can also use IWDG_ReloadCounter(); */
/** @brief 看门狗重载计数器（喂狗）宏 */
#define IWDG_RELOAD_COUNT()       IWDG->KR = KR_KEY_Reload

extern void Drv_Iwdg_Init(void);

/**
  * @}
*/

/**
  * @}
*/

#endif
