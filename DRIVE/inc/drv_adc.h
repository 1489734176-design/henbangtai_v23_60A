/**
 * @file     drv_adc.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions prototypes for the ADC.
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
#ifndef __DRV_ADC_H
#define __DRV_ADC_H

/** Files includes */
#include <stdio.h>
#include "hal_adc.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_ADC
 * @{
 */



/**
 * @brief ADC通道链表结构体
 * @note  用于链式配置多通道扫描序列，每个节点包含通道号、采样顺序和指向下一个节点的指针
 */
typedef struct ADC_Channel
{
    uint8_t u8Rank;                 // 采样顺序（rank编号，从0开始）
    uint8_t sAdcChannel;            // ADC通道号
    struct ADC_Channel * pNext;     // 指向下一个通道节点
}ADC_Channel_TypeDef;


/** @brief 读取ADC转换结束标志位 */
#define READ_ADC_EOC_FLAG()      READ_BIT(ADC1->ADSTA, ADC_IT_EOC)
/** @brief 清除ADC转换结束标志位 */
#define CLEAN_ADC_EOC_FLAG()     SET_BIT(ADC1->ADSTA, ADC_IT_EOC)

/** @brief ADC顺序采样：获取指定通道的ADC转换值（12位，低12位有效） */
#define GET_ADC_VALUE(Channel)      (READ_REG(*(&(ADC1->ADDR0) + Channel)) & 0xFFF)

extern void Drv_Adc_Basic_Init(ADC_TypeDef* pAdc, uint32_t ADC_ExternalTrigConv);
extern void Drv_Adc_Channel_Init(ADC_TypeDef* pAdc, ADC_Channel_TypeDef* pAdcChannel,uint32_t s32SampleTime);

/**
  * @}
*/

/**
  * @}
*/


#endif
