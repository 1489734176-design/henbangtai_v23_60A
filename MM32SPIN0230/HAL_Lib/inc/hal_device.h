////////////////////////////////////////////////////////////////////////////////
/// @file     hal_device.h
/// @author   AE team
/// @brief    CMSIS Cortex-M Peripheral Access Layer for MindMotion
///           microcontroller devices
////////////////////////////////////////////////////////////////////////////////
/// @attention
///
/// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
/// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
/// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
/// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
/// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
/// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
///
/// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
////////////////////////////////////////////////////////////////////////////////


// Define to prevent recursive inclusion
#ifndef __HAL_DEVICE_H
#define __HAL_DEVICE_H


#define MAX_U16_DATA  (u16)65535
//----------------取非负值-------------------//
#define ABS(X)         ( ( (X) >= 0 ) ? (X) : -(X) ) 
//----------------舍去负值-------------------//
#define ABL(X)         ( ( (X) >= 0 ) ? (X) : 0 )
//----------------参数限幅-------------------//
#define Limit(x,LT,HT) ( (x) > (HT) ) ? (HT) : ( ( (x) < (LT) ) ? (LT) : (x) )
//----------------取最大值-------------------//
#define Max(x,y,z)     ( (x) >= (y) ) ? ( ( (x) >= (z) ) ? (x) : (z) ) : ( ( (y) >= (z) ) ? (y) : (z) )
//----------------取最小值-------------------//
#define Min(x,y,z)     ( (x) <= (y) ) ? ( ( (x) <= (z) ) ? (x) : (z) ) : ( ( (y) <= (z) ) ? (y) : (z) )
//----------------x 以y为目标，以最大为z步进趋近，不达到z则直接设为y-------------------//
#define Adj(x,y,z)     ( (x) < (y - z) ) ? (x + z) : ( ( (x) > (y + z) ) ? (x - z) : (y) )

#define Q15(Float_Value)	\
			((Float_Value < 0.0) ? (s16)(32768 * (Float_Value) - 0.5) \
			: (s16)(32767 * (Float_Value) + 0.5))


#include "mm32_device.h"


#endif // __HAL_device_H

/// @}


/// @}

/// @}



