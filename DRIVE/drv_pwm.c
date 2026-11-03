/**
 * @file     drv_pwm.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the driver functions for the PWM.
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
#define _DRV_PWM_C_

/** Files includes */
#include "main.h"
#include "drv_pwm.h"
#include "parameter.h"

// 输出比较模式定义（TIM1 CCMR1/CCMR2寄存器中的OCxM位）
#define CHANNLLOW       4   // 强制输出低电平模式
#define CHANNLHIGH      5   // 强制输出高电平模式
#define PWM1ENBLE       6   // PWM模式1，使能输出

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_PWM
 * @{
 */

/**
 * @brief  TIM1 PWM初始化配置
 * @param  pTim: 定时器外设指针
 * @param  u16Period: PWM周期值（自动重装载值）
 * @param  u16DeadTime: 死区时间值
 * @retval None
 * @note   配置TIM1为中心对齐模式1，PWM1模式，使能互补输出，
 *         配置刹车输入滤波，CH4用于触发ADC采样
 */
void Drv_Pwm_Init(TIM_TypeDef * pTim, uint16_t u16Period,uint16_t u16DeadTime)
{
    /** Define the struct of the PWM configuration */
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStruct;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /** Enable the TIM1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM1_Msk, ENABLE);

    /**
     * Sets the value of the automatic reload register Period for the next update event load activity
     * Set the Prescaler value used as the divisor of the TIMx clock frequency
     * Set clock split :TDTS = TIM_CKD_DIV1
     * TIM center aligned mode1
     */
    TIM_TimeBaseStructure.TIM_Period        = u16Period - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_CenterAligned1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 1;    // 重复计数器=1，每2次更新事件产生一次更新

    TIM_TimeBaseInit(pTim, &TIM_TimeBaseStructure);

    /**
     * Enable state selection in running mode
     * Enable state selection in idle mode
     * Software error lock configuration: lock closed without protection
     * DTG[7:0] dead zone generator configuration (dead zone time DT)
     */
    /**
     * TDTS = 125nS(8MHz)
     * DTG[7: 5] = 0xx => DT = DTG[7: 0] * Tdtg, Tdtg = TDTS;
     * DTG[7: 5] = 10x => DT =(64+DTG[5: 0]) * Tdtg, Tdtg = 2 * TDTS;
     * DTG[7: 5] = 110 => DT =(32+DTG[4: 0]) * Tdtg, Tdtg = 8 * TDTS;
     * DTG[7: 5] = 111=> DT =(32 + DTG[4: 0]) *  Tdtg, Tdtg = 16 * TDTS;
     */
    TIM_BDTRInitStruct.TIM_OSSRState    = TIM_OSSRState_Enable;    // 运行模式关闭状态选择
    TIM_BDTRInitStruct.TIM_OSSIState    = TIM_OSSIState_Enable;    // 空闲模式关闭状态选择
    TIM_BDTRInitStruct.TIM_LOCKLevel    = TIM_LOCKLevel_OFF;       // 锁定等级关闭
    TIM_BDTRInitStruct.TIM_DeadTime     = u16DeadTime;             // 死区时间配置

    /**
     * Brake configuration: enable brake
     * Brake input polarity: active in low level
     * Auto output enable configuration: Disable MOE bit hardware控制
     */
    // 刹车输入滤波配置：64周期滤波，防止误触发
	 TIM_BreakInputFilterConfig(pTim, TIM_COMPBKIN_COMP1, TIM_BKINF_64);
    TIM_BDTRInitStruct.TIM_Break            = TIM_Break_Enable;            // 使能刹车功能
    TIM_BDTRInitStruct.TIM_BreakPolarity    = TIM_BreakPolarity_High;      // 刹车输入高电平有效
    TIM_BDTRInitStruct.TIM_AutomaticOutput  = TIM_AutomaticOutput_Disable; // 禁止MOE位硬件自动控制
    TIM_BDTRConfig(pTim, &TIM_BDTRInitStruct);

    // 使能刹车输入滤波
	TIM_BreakInputFilterCmd(pTim, ENABLE);

    /**
     * Mode configuration: PWM mode 1
     * Output status setting: enable output
     * Complementary channel output status setting: enable output
     * Sets the pulse value to be loaded into the capture comparison register
     * Output polarity is high
     * N Output polarity is high
     */
    TIM_OCInitStructure.TIM_OCMode          = TIM_OCMode_PWM1;         // PWM模式1
    TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable;  // 主输出使能
    TIM_OCInitStructure.TIM_OutputNState    = TIM_OutputNState_Enable; // 互补输出使能
    TIM_OCInitStructure.TIM_Pulse           = 10;                      // 初始占空比
    TIM_OCInitStructure.TIM_OCPolarity      = TIM_OCPolarity_High;     // 主输出高电平有效
    TIM_OCInitStructure.TIM_OCNPolarity     = TIM_OCNPolarity_High;    // 互补输出高电平有效
    TIM_OCInitStructure.TIM_OCIdleState     = TIM_OCIdleState_Reset;   // 空闲状态低电平
    TIM_OCInitStructure.TIM_OCNIdleState    = TIM_OCNIdleState_Reset;  // 互补空闲状态低电平

    TIM_OC1Init(pTim, &TIM_OCInitStructure);
    TIM_OC2Init(pTim, &TIM_OCInitStructure);
    TIM_OC3Init(pTim, &TIM_OCInitStructure);
    TIM1->CCER=0X0000;
    /** Initialize the CCR4 trigger point */
    TIM_OCInitStructure.TIM_Pulse           = 60;                       // CH4触发点初始值
    TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Disable;  // CH4无输出
    TIM_OCInitStructure.TIM_OutputNState    = TIM_OutputNState_Disable;
    TIM_OC4Init(pTim, &TIM_OCInitStructure);

    /** Enable CH1, 2, and 3 to be preloaded */
    TIM_OC1PreloadConfig(pTim, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(pTim, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(pTim, TIM_OCPreload_Enable);

    /** Enable TIMx's preloaded register on ARR */
    TIM_ARRPreloadConfig(pTim, ENABLE);
	TIM_SelectOutputTrigger(TIM1,TIM_TRIGSource_OC4Ref);  // 选择OC4Ref作为触发输出，用于触发ADC
    /** Enable the TIM1 */
    TIM_Cmd(pTim, ENABLE);
    /** Main Output Enable:Disable the MOE bit */
    TIM_CtrlPWMOutputs(pTim, ENABLE);
}

// ==================== 6步换相表（高侧PWM调制，低侧常开） ====================
// UH_VL: U相高侧PWM，V相低侧常开，W相悬空 → 电流从A流向B
void UH_VL_HPwmLOn(void )
{
	TIM1->CCER = 0x5041;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = PWM1ENBLE;		// U+Pwm
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;		// V-H;
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}


// UH_WL: U相高侧PWM，W相低侧常开，V相悬空 → 电流从A流向C
void UH_WL_HPwmLOn(void)
{
	TIM1->CCER = 0x5401;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = PWM1ENBLE;		// U+Pwm
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;		// W-H
}

// VH_WL: V相高侧PWM，W相低侧常开，U相悬空 → 电流从B流向C
void VH_WL_HPwmLOn(void)
{
	TIM1->CCER = 0x5410;									// VH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = PWM1ENBLE;			// V+pwm
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;			// W-H
}

// VH_UL: V相高侧PWM，U相低侧常开，W相悬空 → 电流从B流向A
void VH_UL_HPwmLOn(void)
{
	TIM1->CCER = 0x5014;									// VH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
	TIM1->CCMR1BIT.OC2M = PWM1ENBLE;			// V+pwm
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}

// WH_UL: W相高侧PWM，U相低侧常开，V相悬空 → 电流从C流向A
void WH_UL_HPwmLOn(void)
{
	TIM1->CCER = 0x5104;									// WH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;
	TIM1->CCMR2BIT.OC3M = PWM1ENBLE;			// W+PWM
}

// WH_VL: W相高侧PWM，V相低侧常开，U相悬空 → 电流从C流向B
void WH_VL_HPwmLOn(void)
{
	TIM1->CCER = 0x5140;									// WH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;			// V-H
	TIM1->CCMR2BIT.OC3M = PWM1ENBLE;
}

//--------------ipd-----------

// ==================== 6步换相表（高侧常开，低侧常开，用于IPD初始位置检测） ====================
// UH_VL: U相高侧常开，V相低侧常开，W相悬空
void UH_VL_HOnLOn(void )
{
	TIM1->CCER = 0x5041;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;		// U+H
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;		// V-H;
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}

// U+W-=A+C-
void UH_WL_HOnLOn(void)
{
	TIM1->CCER = 0x5401;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;		// U+H
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;		// W-H
}

// V+W-=B+C-
void VH_WL_HOnLOn(void)
{
	TIM1->CCER = 0x5410;									// VH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;			// V+H
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;			// W-H
}

// V+U-=B+A-
void VH_UL_HOnLOn(void)
{
	TIM1->CCER = 0x5014;									// VH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;			// V+H
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}

// W+U-=C+A-
void WH_UL_HOnLOn(void)
{
	TIM1->CCER = 0x5104;									// WH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;			// W+H
}

// W+V-=C+B-
void WH_VL_HOnLOn(void)
{
	TIM1->CCER = 0x5140;									// WH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;			// V-H
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;
}

//------------------------------------

//--------------mos check-----------
// ==================== MOS管检测模式（高侧关断，低侧常开） ====================
// 用于检测各相下桥MOS管是否正常导通

void UL_HOffLOn(void )
{
	TIM1->CCER = 0x5004;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;		//
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;		//
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}


void VL_HOffLOn(void)
{
	TIM1->CCER = 0x5040;								//
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;		//
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;		//
}


void WL_HOffLOn(void)
{
	TIM1->CCER = 0x5400;									//
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;			//
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;			//
}

// ==================== MOS管检测模式（高侧常开，低侧关断） ====================
// 用于检测各相上桥MOS管是否正常关断

void UH_HOnLOff(void)
{
	TIM1->CCER = 0x5001;									//
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			//
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;			//
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;
}
///--------------------------------------------------------------------

void VH_HOnLOff(void)
{
	TIM1->CCER = 0x5010;									//
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;			//
	TIM1->CCMR1BIT.OC2M = CHANNLHIGH;
	TIM1->CCMR2BIT.OC3M = CHANNLLOW;			//
}


void WH_HOnLOff(void)
{
	TIM1->CCER = 0x5100;									//
	TIM1->CCMR1BIT.OC1M = CHANNLLOW;
	TIM1->CCMR1BIT.OC2M = CHANNLLOW;			//
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;
}

//------------------------------------

// ==================== 扩展换相表（高侧PWM调制，低侧常开，用于特殊驱动模式） ====================

// 导通相: UH+VL+WL (U相PWM，V/W相低侧常开)
void UH_VL_WL_HPwmLOn(void )
{
		TIM1->CCER = 0x5441;								// UH 输出使高侧有效, 低侧有效
		TIM1->CCMR1BIT.OC1M = PWM1ENBLE;		// U+Pwm
		TIM1->CCMR1BIT.OC2M = CHANNLHIGH;		// V-H;
		TIM1->CCMR2BIT.OC3M = CHANNLHIGH;
}

// U+W-=A+C- (UH+VH+WL)
void UH_VH_WL_HPwmLOn(void)
{
		TIM1->CCER = 0x5411;								// UH 输出使高侧有效, 低侧有效
		TIM1->CCMR1BIT.OC1M = PWM1ENBLE;		// U+Pwm
		TIM1->CCMR1BIT.OC2M = PWM1ENBLE;
		TIM1->CCMR2BIT.OC3M = CHANNLHIGH;		// W-H
}

// V+W-=B+C- (UL+VH+WL)
void UL_VH_WL_HPwmLOn(void)
{
	TIM1->CCER = 0x5414;									// VH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = CHANNLHIGH;
	TIM1->CCMR1BIT.OC2M = PWM1ENBLE;			// V+pwm
	TIM1->CCMR2BIT.OC3M = CHANNLHIGH;			// W-H
}

// V+U-=B+A- (UL+VH+WH)
void UL_VH_WH_HPwmLOn(void)
{
		TIM1->CCER = 0x5114;									// VH 输出使高侧有效, 低侧有效
		TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
		TIM1->CCMR1BIT.OC2M = PWM1ENBLE;			// V+pwm
		TIM1->CCMR2BIT.OC3M = PWM1ENBLE;
}

// W+U-=C+A- (UL+VL+WH)
void UL_VL_WH_HPwmLOn(void)
{
		TIM1->CCER = 0x5144;									// WH 输出使高侧有效, 低侧有效
		TIM1->CCMR1BIT.OC1M = CHANNLHIGH;			// U-H
		TIM1->CCMR1BIT.OC2M = CHANNLHIGH;
		TIM1->CCMR2BIT.OC3M = PWM1ENBLE;			// W+PWM
}

// W+V-=C+B- (UH+VL+WH)
void UH_VL_WH_HPwmLOn(void)
{
		TIM1->CCER = 0x5141;									// WH 输出使高侧有效, 低侧有效
		TIM1->CCMR1BIT.OC1M = PWM1ENBLE;
		TIM1->CCMR1BIT.OC2M = CHANNLHIGH;			// V-H
		TIM1->CCMR2BIT.OC3M = PWM1ENBLE;
}

// 三相全PWM输出模式（用于特殊检测）
void UVWL_HOffLPwm(void )
{
	TIM1->CCER = 0x5444;								// UH 输出使高侧有效, 低侧有效
	TIM1->CCMR1BIT.OC1M = PWM1ENBLE;		//
	TIM1->CCMR1BIT.OC2M = PWM1ENBLE;		//
	TIM1->CCMR2BIT.OC3M = PWM1ENBLE;
}

/*------------------------------------------*/

/**
 * @brief  启动电机：使能所有PWM输出通道
 * @param  None
 * @retval None
 * @note   清零所有占空比，使能OC1/OC1N/OC2/OC2N/OC3/OC3N输出
 */
void Start_Motor(void)
{
    TIM1->CCR1 = 0x00;
    TIM1->CCR2 = 0x00;
    TIM1->CCR3 = 0x00;
    //OC1/OC1N/OC2/OC2N/OC3/OC3N
    TIM1->CCER |= 0x5555;
}

/**
 * @brief  停止电机：关闭所有PWM输出
 * @param  None
 * @retval None
 * @note   清零所有占空比，禁用所有输出使能位
 */
void Stop_Motor(void)
{
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
		TIM1->CCR4 = 40;
    //OC1/OC1N/OC2/OC2N/OC3/OC3N
    TIM1->CCER &= 0xAAAA;
}

/**
 * @brief  刹车电机：下桥臂全部导通实现能耗制动
 * @param  None
 * @retval None
 * @note   所有相低侧强制输出高电平（下桥臂导通），实现快速制动
 */
void Brake_Motor(void)
{
    TIM1->CCR1 = 0x00;
    TIM1->CCR2 = 0x00;
    TIM1->CCR3 = 0x00;
		TIM1->CCR4 = 40;
    TIM1->CCMR1BIT.OC1M = CHANNLLOW;//U+H
    TIM1->CCMR1BIT.OC2M = CHANNLLOW;//V_H
    TIM1->CCMR2BIT.OC3M = CHANNLLOW;//W_H
    TIM1->CCER |= 0x5555;
}

/**
 * @brief  电机停止/刹车（根据宏配置选择）
 * @param  None
 * @retval None
 * @note   若EN_MOTOR_BREAK使能则执行刹车，否则执行自由停车
 */
void Motor_Stop_Break(void)
{
	#if EN_MOTOR_BREAK
		Brake_Motor();
	#else
		Stop_Motor();
	#endif
}

/**
 * @brief  更新PWM占空比
 * @param  u16Duty: 新的占空比值
 * @retval None
 * @note   同时更新CH1/CH2/CH3的占空比，CH4触发点根据占空比动态调整
 *         当占空比>160时，触发点=占空比-120；否则触发点固定为40
 */
void PwmDutyUpdate(uint16_t u16Duty)
{
//    else if(u16Duty < Motor_1st.BLDC.u16StartUp_PwmValue)
//    {
//        u16Duty = Motor_1st.BLDC.u16StartUp_PwmValue;
//    }

    TIM1->CCR1 = u16Duty;
    TIM1->CCR2 = u16Duty;
    TIM1->CCR3 = u16Duty;

		if(u16Duty>160)
		{
			TIM1->CCR4 = u16Duty-120;
		}
		else
		{
			TIM1->CCR4 = 40;
		}
}




/**
  * @}
*/

/**
  * @}
*/
