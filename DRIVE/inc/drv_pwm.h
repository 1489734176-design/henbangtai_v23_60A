/**
 * @file     drv_pwm.h
 * @author   MindMotion Motor Team : Wesson
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
#ifndef __DRV_PWM_H
#define __DRV_PWM_H

/** Files includes */
#include "mm32_device.h"
#include "HAL_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_PWM
 * @{
 */

/** @brief 设置TIM1通道1比较值（占空比） */
#define SET_CCR1_VAL(Value)         WRITE_REG(TIM1->CCR1, Value)
/** @brief 设置TIM1通道2比较值（占空比） */
#define SET_CCR2_VAL(Value)         WRITE_REG(TIM1->CCR2, Value)
/** @brief 设置TIM1通道3比较值（占空比） */
#define SET_CCR3_VAL(Value)         WRITE_REG(TIM1->CCR3, Value)
/** @brief 设置TIM1通道4比较值（ADC触发点） */
#define SET_CCR4_VAL(Value)         WRITE_REG(TIM1->CCR4, Value)

/** @brief 禁用PWM输出（清除MOE位） */
#define DISABLE_PWMOUT()            CLEAR_BIT(TIM1->BDTR, TIM_BDTR_MOE_Msk)
/** @brief 使能PWM输出（置位MOE位） */
#define ENABLE_PWMOUT()             SET_BIT(TIM1->BDTR, TIM_BDTR_MOE_Msk)

/** @brief 读取TIM1更新中断标志 */
#define READ_TIM1_UPDATE_FLAG()     READ_BIT(TIM1->SR, TIM_IT_Update)
/** @brief 清除TIM1更新中断标志 */
#define CLEAN_TIM1_UPDATE_FLAG()    CLEAR_BIT(TIM1->SR, TIM_IT_Update)

/** @brief 使能TIM1刹车输入 */
#define TIM1_BREAK_ENABLE()         SET_BIT(TIM1->BDTR,TIM_BDTR_BKE)
/** @brief 禁用TIM1刹车输入 */
#define TIM1_BREAK_DISABLE()        CLEAR_BIT(TIM1->BDTR,TIM_BDTR_BKE)
/** @brief 读取TIM1刹车中断标志 */
#define READ_TIM1_BREAK_FLAG()      READ_BIT(TIM1->SR, TIM_IT_Break)
/** @brief 清除TIM1刹车中断标志 */
#define CLEAN_TIM1_BREAK_FLAG()     CLEAR_BIT(TIM1->SR, TIM_IT_Break)

extern void Drv_Pwm_Init(TIM_TypeDef * pTim, uint16_t u16Period,uint16_t u16DeadTime);


// ==================== 6步换相函数声明（高侧PWM调制，低侧常开） ====================

/** @brief U相高侧PWM + V相低侧常开（U+V-） */
extern void UH_VL_HPwmLOn(void);
/** @brief W相高侧PWM + U相低侧常开（W+U-） */
extern void WH_UL_HPwmLOn(void);
/** @brief W相高侧PWM + V相低侧常开（W+V-） */
extern void WH_VL_HPwmLOn(void);
/** @brief V相高侧PWM + W相低侧常开（V+W-） */
extern void VH_WL_HPwmLOn(void);
/** @brief U相高侧PWM + W相低侧常开（U+W-） */
extern void UH_WL_HPwmLOn(void);
/** @brief V相高侧PWM + U相低侧常开（V+U-） */
extern void VH_UL_HPwmLOn(void);




//U+V-=A+B-
void UH_VL_HOnLOn(void );
//U+W-=A+C-
void UH_WL_HOnLOn(void);
//V+W-=B+C-
void VH_WL_HOnLOn(void);

//V+U-=B+A-
void VH_UL_HOnLOn(void);
//W+U-=C+A-
void WH_UL_HOnLOn(void);

//W+V-=C+B-
void WH_VL_HOnLOn(void);




void UL_HOffLOn(void);
void VL_HOffLOn(void);
void WL_HOffLOn(void);
void UH_HOnLOff(void);
void VH_HOnLOff(void);
void WH_HOnLOff(void);

void UVWL_HOffLPwm(void);









/** @brief 启动电机 */
void Start_Motor(void);
/** @brief 停止电机 */
void Stop_Motor(void);
/** @brief 刹车电机 */
void Brake_Motor(void);
/** @brief 更新PWM占空比 */
void PwmDutyUpdate(uint16_t u16Duty);

/**
  * @}
*/

/**
  * @}
*/

#endif
