/** 
 * @file     drv_inc.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file is used to include the driver header files used in this project
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
#ifndef __DRV_INC_H
#define __DRV_INC_H

/** Files includes */
#include <stdint.h>
#include "HAL_conf.h"
#include "mm32_device.h"
#include "systick.h"
#include "board.h"
#include "drv_adc.h"
#include "drv_comp.h"
#include "drv_pwm.h"
#include "drv_led.h"
#include "drv_iwdg.h"
#include "drv_div.h"
#include "mm32_it.h"


/** 
 * @addtogroup MM32_User_Layer
 * @{
 */

/** 
 * @addtogroup User_Main
 * @{
 */

/**
  * @}
*/

/**
  * @}
*/

#endif
