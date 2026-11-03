/**
 * @file     drv_comp.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the COMP.
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
#define _DRV_COMP_C_

/** Files includes */
#include "drv_comp.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_COMP
 * @{
 */

/**
 * @brief    : This function describes the underlying configuration of the comparator.
 * @param      sSelection : Comp number
 *             pCompInput : Comp struct,include NonInvertingInput/InvertingInput/CrvSelect
 * @retval   : None
 */

/**
 * @brief BEMF过零检测比较器正端输入通道选择表（正转方向）
 * @note  根据6步换相顺序，依次选择悬空相的BEMF输入到比较器正端
 *        扇区0:W相, 扇区1:V相, 扇区2:U相, 扇区3:W相, 扇区4:V相, 扇区5:U相
 */
 uint8_t Comp_CH_Tab[6] =
{
	 COMP_CH_BEMF_W,       // 扇区0: 检测W相反电动势
   COMP_CH_BEMF_V,       // 扇区1: 检测V相反电动势
   COMP_CH_BEMF_U,       // 扇区2: 检测U相反电动势
	 COMP_CH_BEMF_W,       // 扇区3: 检测W相反电动势
   COMP_CH_BEMF_V,       // 扇区4: 检测V相反电动势
   COMP_CH_BEMF_U        // 扇区5: 检测U相反电动势
};

/**
 * @brief BEMF过零检测比较器正端输入通道选择表（反转方向）
 * @note  反转时换相顺序相反，通道选择表与正转不同
 *        扇区0:V相, 扇区1:W相, 扇区2:U相, 扇区3:V相, 扇区4:W相, 扇区5:U相
 */
 uint8_t Comp_CH_Tab1[6] =
{
	 COMP_CH_BEMF_V,       // 扇区0: 检测V相反电动势
	 COMP_CH_BEMF_W,       // 扇区1: 检测W相反电动势
   COMP_CH_BEMF_U,       // 扇区2: 检测U相反电动势
   COMP_CH_BEMF_V,       // 扇区3: 检测V相反电动势
	 COMP_CH_BEMF_W,       // 扇区4: 检测W相反电动势
   COMP_CH_BEMF_U        // 扇区5: 检测U相反电动势
};

/**
 * @brief  根据扇区索引和转向选择比较器输入通道，并配置过零检测边沿
 * @param  sector_index: 当前扇区索引(0~5)
 * @param  dir: 旋转方向，1=正转，0=反转
 * @retval None
 * @note   偶数扇区(0,2,4)检测下降沿过零，奇数扇区(1,3,5)检测上升沿过零
 */
void select_comp_ch_for_sector(u8 sector_index,u8 dir)
{
	if(dir==1)
	{
		// 正转：使用正转通道选择表
		setv(COMP2->COMPx_CSR,BS7,BW2,Comp_CH_Tab[sector_index]);
	}
	else
	{
		// 反转：使用反转通道选择表
		setv(COMP2->COMPx_CSR,BS7,BW2,Comp_CH_Tab1[sector_index]);
	}
	if((sector_index&0x01)==0)//falling
	{
		// 偶数扇区：检测下降沿过零点
	     EXTI->RTSR &= ~EXTI_Line20;
	       EXTI->FTSR &= ~EXTI_Line20;

			EXTI->FTSR |= EXTI_Line20;
	}
	else
	{
		// 奇数扇区：检测上升沿过零	点
			 EXTI->RTSR &= ~EXTI_Line20;
	       EXTI->FTSR &= ~EXTI_Line20;

			 EXTI->RTSR  |= EXTI_Line20;


	}
}

/**
 * @brief  比较器初始化配置
 * @param  comp: 比较器外设指针
 * @param  pCompInput: 比较器输入配置结构体，包含正端/负端输入选择和CRV选择
 * @retval None
 * @note   配置比较器输出到TIM1刹车输入，使能CRV（内部参考电压），
 *         输出无迟滞，低功耗模式，128周期滤波
 */
void Drv_Comp_Init(COMP_TypeDef *comp,COMP_Input_TypeDef * pCompInput)
{
    COMP_InitTypeDef COMP_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_COMP_Msk, ENABLE);

    COMP_DeInit(comp);
    COMP_StructInit(&COMP_InitStructure);
    COMP_InitStructure.COMP_NonInvert = pCompInput->sCompNonInvertingInput;
    COMP_InitStructure.COMP_Invert    = pCompInput->sCompInvertingInput;
    COMP_InitStructure.COMP_Output            = COMP_Output_TIM1BKIN;       // 比较器输出到TIM1刹车输入
    COMP_InitStructure.COMP_OutputPol         = COMP_Pol_NonInvertedOut;     // 输出不反相
    COMP_InitStructure.COMP_Hysteresis        = COMP_Hysteresis_Medium;      // 中等迟滞
    COMP_InitStructure.COMP_Mode              = COMP_Mode_Low1_Power;        // 低功耗模式
    COMP_InitStructure.COMP_OFLT              = COMP_Filter_128_Period;      // 128周期滤波，抑制BEMF噪声

    COMP_Init(comp, &COMP_InitStructure);

    COMP_CrvCmd(comp,ENABLE);   // 使能内部参考电压(CRV)


    if (COMP_InvertingInput_IO3 == pCompInput->sCompInvertingInput)
    {
      COMP_SetCrv(comp,COMP_CRV_Src_VDDA,pCompInput->u8CompCrvSelect);
    }
    COMP_Cmd(comp, ENABLE);
}



/**
  * @}
*/

/**
  * @}
*/
