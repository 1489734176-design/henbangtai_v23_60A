/**
 * @file     drv_adc.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the ADC.
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
#define _DRV_ADC_C_

/** Files includes */
#include "hal_rcc.h"
#include "hal_misc.h"

#include "drv_adc.h"
/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_ADC
 * @{
 */

/**
 * @brief  ADC基础初始化配置
 * @param  pAdc: ADC外设指针
 * @param  ADC_ExternalTrigConv: 外部触发转换源选择
 * @retval None
 * @note   配置ADC为12位分辨率，扫描模式，由TIM1_CCR4触发转换，数据右对齐
 */
void Drv_Adc_Basic_Init(ADC_TypeDef* pAdc, uint32_t ADC_ExternalTrigConv)
{
    /** Define the struct of ADC configuration */
    ADC_InitTypeDef  ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);

    /** Enable the ADC1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_ADC1_Msk, ENABLE);

    /**
     * Initialize the ADC to 12bit
     * ADC clock 6 frequency division
     * Single period mode
     * ADC conversion is triggered by TIM1_CCR4
     * Data right aligned
     */
    ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
    ADC_InitStructure.ADC_Prescaler              = ADC_Prescaler_4;
    ADC_InitStructure.ADC_Mode                  = ADC_Mode_Scan;
    ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;

    ADC_Init(pAdc, &ADC_InitStructure);

    ADC_ExternalTrigSourceConfig(pAdc,ADC_ExternalTrigConv,ADC_ExtTrig_Edge_Dual,ADC_ExtTrig_Shift_0);

    /** External triggering was enabled */
    ADC_ExternalTrigConvCmd(pAdc, ENABLE);
    ADC_Cmd(pAdc, ENABLE);
}

/**
 * @brief  ADC通道初始化配置（链表方式配置多通道扫描序列）
 * @param  pAdc: ADC外设指针
 * @param  pAdcChannel: ADC通道链表指针，包含通道号、采样顺序、采样时间
 * @param  s32SampleTime: 采样时间
 * @retval None
 * @note   通过链表遍历配置每个通道的采样顺序和采样时间，最后使能ANY通道转换器
 */
void Drv_Adc_Channel_Init(ADC_TypeDef* pAdc, ADC_Channel_TypeDef* pAdcChannel,uint32_t s32SampleTime)
{
     uint8_t u8Temp = 0;
    if ((pAdc == NULL) || (pAdcChannel == NULL))
    {
        return ;
    }
    do
    {
        /** Sample time selection to s32SampleTime */
        ADC_SampleTimeConfig(pAdc, pAdcChannel->sAdcChannel, s32SampleTime);
        if (u8Temp < pAdcChannel->u8Rank)
        {
            /**Configures the adc any channels conversion Max rank number by macro definition */
            ADC_AnyChannelNumCfg(pAdc, pAdcChannel->u8Rank);
        }
        /** Configures the adc any channels conversion rank and channelby macro definition */
        ADC_AnyChannelSelect(pAdc, pAdcChannel->u8Rank, pAdcChannel->sAdcChannel);

        u8Temp = pAdcChannel->u8Rank;

        pAdcChannel = pAdcChannel->pNext;
    } while(pAdcChannel != NULL);
    /** Enables the ANY channel converter */
    ADC_AnyChannelCmd(pAdc, ENABLE);
}


/**
  * @}
*/

/**
  * @}
*/
