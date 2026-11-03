/**
 * @file     board.c
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the functions for the board level support package.
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
#define _BOARD_C_

/** Files includes */
#include "board.h"
#include "drv_inc.h"
#include "parameter.h"
#include "led.h"
#include "Bat_com.h"
/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Bsp
 * @{
 */

/**
  * @brief 运放GPIO初始化
  *        配置运放1的输出端和正负输入端为模拟输入模式
  * @param None
  * @retval None
  */
void Bsp_Op_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    /* GPIO Ports Clock Enable */
    RCC_AHBPeriphClockCmd(OPAMP_GPIO_CLK, ENABLE);

    /*Configure GPIO pin : OPAMP1_Pin */
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;

	  GPIO_InitStructure.GPIO_Pin     = OPAMP1_OUT_PIN;
    GPIO_Init(OPAMP1_OUT_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = OPAMP1_INM_PIN;
    GPIO_Init(OPAMP1_INM_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = OPAMP1_INP_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;

    GPIO_Init(OPAMP1_INP_PORT, &GPIO_InitStructure);
}
/**
  * @brief ADC GPIO初始化
  *        配置电压、电流、BEMF、温度等ADC通道对应GPIO为模拟输入
  * @param None
  * @retval None
  */

void Bsp_Adc_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    RCC_AHBPeriphClockCmd(ADC_GPIO_CLK, ENABLE);			// OK

    /* PA1：数字模式高电平按下、下拉输入；ADC模式为模拟输入。 */
#if(TRG_DIGITAL_EN)
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPD;
#else
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;
#endif
    GPIO_InitStructure.GPIO_Pin     = VR_PIN;
    GPIO_Init(VR_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;

    /* VBUS（母线电压） */
    GPIO_InitStructure.GPIO_Pin     = VBUS_PIN;
    GPIO_Init(VBUS_PORT, &GPIO_InitStructure);

	/* ISUMAVG（平均电流） */
	GPIO_InitStructure.GPIO_Pin     = ISUMAVG_PIN;
    GPIO_Init(ISUMAVG_PORT, &GPIO_InitStructure);

	/* IRMS（电流有效值/峰值检测） */
	GPIO_InitStructure.GPIO_Pin     = IRMS_PIN;
    GPIO_Init(IRMS_PORT, &GPIO_InitStructure);

//	   GPIO_InitStructure.GPIO_Pin     = DIR_PIN;
//    GPIO_Init(DIR_PORT, &GPIO_InitStructure);

	/* PA3：DS/TBAT 预留采样，不参与当前保护判定。 */
	GPIO_InitStructure.GPIO_Pin = T_BAT_PIN;
	GPIO_Init(T_BAT_PORT, &GPIO_InitStructure);

	/* T_MOS（MOS温度） */
	GPIO_InitStructure.GPIO_Pin     = T_MOS_PIN;
    GPIO_Init(T_MOS_PORT, &GPIO_InitStructure);

    /* BEMF U相 */
    GPIO_InitStructure.GPIO_Pin     = BEMF_U_PIN;
    GPIO_Init(BEMF_U_PORT, &GPIO_InitStructure);

    /* BEMF V相 */
    GPIO_InitStructure.GPIO_Pin     = BEMF_V_PIN;
    GPIO_Init(BEMF_V_PORT, &GPIO_InitStructure);

    /* BEMF W相 */
    GPIO_InitStructure.GPIO_Pin     = BEMF_W_PIN;
    GPIO_Init(BEMF_W_PORT, &GPIO_InitStructure);

}

/**
  * @brief 比较器GPIO初始化
  *        配置比较器输入端为模拟输入
  */
void Bsp_Comp_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
		/* COMP Clock Enable */
    RCC_AHBPeriphClockCmd((COMP_GPIO_CLK), ENABLE);

		/*Configure GPIO pin : COMP_Pin */
    GPIO_InitStructure.GPIO_Pin = COMP_INP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_Init(COMP_INP_PORT, &GPIO_InitStructure);
}

/**
  * @brief 调试IO初始化
  *        配置PA5为推挽输出，用于调试时指示
  * @param None
  * @retval None
  */
void Bsp_Debug_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    RCC_AHBPeriphClockCmd(DEBUG_IO_RCC_CLOCKGPIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin     =  DEBUG_IO1_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_Init(DEBUG_IO1_PORT, &GPIO_InitStructure);
}

//void Bsp_LED_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    GPIO_StructInit(&GPIO_InitStructure);
//    RCC_AHBPeriphClockCmd(LED_RCC_CLOCKGPIO, ENABLE);
//
//    GPIO_InitStructure.GPIO_Pin     =  LED1_PIN;
//    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
//    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
//    GPIO_Init(LED1_PORT, &GPIO_InitStructure);
//}

/**
  * @brief 按键GPIO初始化
  *        配置按键引脚为上拉输入
  * @param None
  * @retval None
  */
void Bsp_Key_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHBPeriphClockCmd(KEY_RCC_CLOCKGPIO, ENABLE);
    GPIO_PinAFConfig(KEY_PORT, KEY_PIN_SRC, KEY_PIN_AF);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = KEY_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &gpio);
}

/**
  * @brief 使能端口初始化
  *        配置PA11为推挽输出（电源保持EN），并置高使能
  */
void En_Port_init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

	/* LED Clock Enable */
    RCC_AHBPeriphClockCmd(EN_PIN_RCC_CLOCKGPIO, ENABLE);		// OK
	
	GPIO_SetBits(EN_PORT, EN_PIN);
	/* PA11 用作 EN，保持普通 GPIO 功能。 */
	GPIO_PinAFConfig(EN_PORT, EN_PIN_SRC, EN_PIN_AF);
	GPIO_InitStructure.GPIO_Pin     =  EN_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_Init(EN_PORT, &GPIO_InitStructure);
}

/**
  * @brief COM_TX(PA11) 引脚模式切换
  *        发送时切到 USART 复用推挽；空闲时切回普通输出低，释放 COM 总线。
  *        先写低再配置，避免切换瞬间误拉低 COM。
  * @param uart_enable 1=UART 复用发送，0=GPIO 输出低释放总线
  */
void Bsp_Com_Tx_Mode(uint8_t uart_enable)
{
#if BAT_COM_EN
    GPIO_InitTypeDef gpio;

    GPIO_StructInit(&gpio);
    GPIO_ResetBits(COM_TX_PORT, COM_TX_PIN);
    gpio.GPIO_Pin = COM_TX_PIN;
    gpio.GPIO_Speed = GPIO_Speed_High;
    gpio.GPIO_Mode = uart_enable ? GPIO_Mode_AF_PP : GPIO_Mode_Out_PP;
    GPIO_Init(COM_TX_PORT, &gpio);
#else
    (void)uart_enable;
#endif
}

/**
  * @brief 通信端口初始化
  *        配置 COM_EN(PA14) 输出高使能，TX(PA11) 初始为 GPIO 低释放总线，
  *        RX(PA12) 复用浮空输入。不在此启用中断，由通信生命周期管理。
  */
void Bsp_Com_init(void)
{
#if BAT_COM_EN
    GPIO_InitTypeDef gpio;

    RCC_AHBPeriphClockCmd(COM_EN_RCC_CLOCKGPIO | COM_TX_RCC_CLOCKGPIO, ENABLE);
    GPIO_StructInit(&gpio);
    COM_EN_HIGH();
    GPIO_PinAFConfig(COM_EN_PORT, COM_EN_PIN_SRC, COM_EN_PIN_AF);
    gpio.GPIO_Pin = COM_EN_PIN;
    gpio.GPIO_Speed = GPIO_Speed_High;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(COM_EN_PORT, &gpio);

    Bsp_Com_Tx_Mode(0);
    GPIO_PinAFConfig(COM_TX_PORT, COM_TX_PIN_SRC, COM_TX_PIN_AF);
    GPIO_PinAFConfig(COM_RX_PORT, COM_RX_PIN_SRC, COM_RX_PIN_AF);
    gpio.GPIO_Pin = COM_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_FLOATING;
    GPIO_Init(COM_RX_PORT, &gpio);
#endif
}


/**
  * @brief USART 初始化（电池包单线半双工通信，DAYE A1.1）
  *        4800bps / 8 数据位 / 1 停止位 / 无校验，使能接收中断
  */
void Bsp_Usart_Init(void)
{
    USART_InitTypeDef USART_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART1_Msk, ENABLE);
    USART_DeInit(USART1);

    USART_StructInit(&USART_InitStruct);
    USART_InitStruct.USART_BaudRate            = 4800;
    USART_InitStruct.USART_WordLength          = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits            = USART_StopBits_1;
    USART_InitStruct.USART_Parity              = USART_Parity_No;
    USART_InitStruct.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStruct);

    USART_Cmd(USART1, ENABLE);
    NVIC_SetPriority(USART1_IRQn, 2);
}

/**
  * @brief 方向检测引脚初始化
  *        PB8为方向/LED行1复用输入，PB9由LED COM4驱动
  */
void Bsp_dir_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);



	/* LED Clock Enable */
    RCC_AHBPeriphClockCmd(DIR_RCC_CLOCKGPIO, ENABLE);		// OK
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_1);
	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStructure.GPIO_Pin     =  DIR_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_Init(DIR_PORT, &GPIO_InitStructure);


}
/**
  * @brief PWM GPIO初始化
  *        配置6路PWM输出引脚（UH/VH/WH/UL/VL/WL）为复用推挽输出
  * @param None
  * @retval None
  */

void Bsp_Pwm_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

		/* PWM Clock Enable */
    RCC_AHBPeriphClockCmd(BLDC1_GPIO_CLK, ENABLE);

		/*selects the pin to used as Alternate function of PWM*/
    GPIO_PinAFConfig(BLDC1_UH_PORT, BLDC1_UH_PIN_SRC, BLDC1_UH_PIN_AF);
    GPIO_PinAFConfig(BLDC1_VH_PORT, BLDC1_VH_PIN_SRC, BLDC1_VH_PIN_AF);
    GPIO_PinAFConfig(BLDC1_WH_PORT, BLDC1_WH_PIN_SRC, BLDC1_WH_PIN_AF);

    GPIO_PinAFConfig(BLDC1_UL_PORT, BLDC1_UL_PIN_SRC, BLDC1_UL_PIN_AF);
    GPIO_PinAFConfig(BLDC1_VL_PORT, BLDC1_VL_PIN_SRC, BLDC1_VL_PIN_AF);
    GPIO_PinAFConfig(BLDC1_WL_PORT, BLDC1_WL_PIN_SRC, BLDC1_WL_PIN_AF);

		/*Configure GPIO pin : PWM_Pin - UH */
    GPIO_InitStructure.GPIO_Pin     = BLDC1_UH_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_Init(BLDC1_UH_PORT, &GPIO_InitStructure);

    /* VH */
    GPIO_InitStructure.GPIO_Pin = BLDC1_VH_PIN;
    GPIO_Init(BLDC1_VH_PORT, &GPIO_InitStructure);

    /* WH */
    GPIO_InitStructure.GPIO_Pin = BLDC1_WH_PIN;
    GPIO_Init(BLDC1_WH_PORT, &GPIO_InitStructure);

    /* UL */
    GPIO_InitStructure.GPIO_Pin = BLDC1_UL_PIN;
    GPIO_Init(BLDC1_UL_PORT, &GPIO_InitStructure);

    /* VL */
    GPIO_InitStructure.GPIO_Pin = BLDC1_VL_PIN;
    GPIO_Init(BLDC1_VL_PORT, &GPIO_InitStructure);

    /* WL */
    GPIO_InitStructure.GPIO_Pin = BLDC1_WL_PIN;
    GPIO_Init(BLDC1_WL_PORT, &GPIO_InitStructure);
}

/**
  * @brief 初始化所有配置的GPIO（ADC、比较器、PWM）
  * @param None
  * @retval None
  */
void Bsp_Gpio_Init(void)
{
    Bsp_Op_Init();
		Bsp_Comp_Init();
    Bsp_Adc_Init();
		Bsp_Pwm_Init();
//		led_init();

//		Bsp_LED_Init();
//		LED1_ON();
		/* DIR/KEY共享灯脚由校准后的led_init接管，先保留PA14的SWD启动窗口。 */
}
/**
  * @brief ADC初始化函数
  *        配置ADC扫描通道序列（BEMF_U/V/W、OP_O、ISUMAVG、VR、VBUS、T_BAT、T_MOS）
  *        使用TIM1_CC4触发ADC转换
  * @param None
  * @retval None
  */
void Board_ADC_Init(void)
{
    /*ADC  RANK Array*/
    ADC_Channel_TypeDef sUserAdc1Channel[9];

    /* Configure the ADC RANK Sequence - Rank0: BEMF_U */
    sUserAdc1Channel[0].u8Rank 			= BEMF_U_RANK;
    sUserAdc1Channel[0].sAdcChannel = BEMF_U_CHANNEL;
    sUserAdc1Channel[0].pNext 			= &sUserAdc1Channel[1];

	/* Rank1: BEMF_V */
    sUserAdc1Channel[1].u8Rank 			= BEMF_V_RANK;
    sUserAdc1Channel[1].sAdcChannel = BEMF_V_CHANNEL;
    sUserAdc1Channel[1].pNext 			= &sUserAdc1Channel[2];

	/* Rank2: BEMF_W */
    sUserAdc1Channel[2].u8Rank 			= BEMF_W_RANK;
    sUserAdc1Channel[2].sAdcChannel = BEMF_W_CHANNEL;
    sUserAdc1Channel[2].pNext 			= &sUserAdc1Channel[3];

	/* Rank3: OP_O（运放输出，峰值电流检测） */
    sUserAdc1Channel[3].u8Rank 			= OP_O_RANK;
    sUserAdc1Channel[3].sAdcChannel = OP_O_CHANNEL;
    sUserAdc1Channel[3].pNext 			= &sUserAdc1Channel[4];

	/* Rank4: ISUMAVG（平均电流） */
    sUserAdc1Channel[4].u8Rank 			= ISUMAVG_RANK;
    sUserAdc1Channel[4].sAdcChannel = ISUMAVG_CHANNEL;
    sUserAdc1Channel[4].pNext 			= &sUserAdc1Channel[5];

    /* Rank5: VR（扳机电压/调速信号） */
    sUserAdc1Channel[5].u8Rank 			= VR_RANK ;
    sUserAdc1Channel[5].sAdcChannel = VR_CHANNEL;
    sUserAdc1Channel[5].pNext 			= &sUserAdc1Channel[6];

	/* Rank6: VBUS（母线电压） */
    sUserAdc1Channel[6].u8Rank 			= VBUS_RANK;
    sUserAdc1Channel[6].sAdcChannel = VBUS_CHANNEL ;
    sUserAdc1Channel[6].pNext =  &sUserAdc1Channel[7];


	/* Rank7: T_BAT（预留电池温度） */
	sUserAdc1Channel[7].u8Rank 			= T_BAT_RANK;
    sUserAdc1Channel[7].sAdcChannel = T_BAT_CHANNEL ;
    sUserAdc1Channel[7].pNext =  &sUserAdc1Channel[8];


	/* Rank8: T_MOS（MOS温度） */
	sUserAdc1Channel[8].u8Rank 			= T_MOS_RANK;
    sUserAdc1Channel[8].sAdcChannel = T_MOS_CHANNEL ;
    sUserAdc1Channel[8].pNext =NULL;

	     /* Select the ADC sample time*/
    Drv_Adc_Channel_Init(ADC1, sUserAdc1Channel, ADC_SampleTime_3_5);
    /* Select the ADC external trigger source of the ADC is T1_CC4*/
    Drv_Adc_Basic_Init(ADC1, ADC_ExtTrig_T1_CC4);


}

volatile COMP_TypeDef *pComp;

/**
  * @brief 比较器初始化函数
  *        配置比较器用于电流检测（非反相端接IO3，反相端接IO3，CRV选择135/256）
  */
void Board_Comp_Init(void)
{
    COMP_Input_TypeDef sUserCompInput;
	  /* Select the inverting input of the comparator */
    sUserCompInput.sCompInvertingInput = COMP_INVERTING;
	  /* Select the non inverting input of the comparator*/
    sUserCompInput.sCompNonInvertingInput = COMP_NON_INVERTING;
	  /* Select comparator external reference voltage */
    sUserCompInput.u8CompCrvSelect = COMP_CRV_VOLTAGE_SELECT;
	  /* Initializes the COMP according to the specified parameters in the COMP_Input_TypeDef */
    Drv_Comp_Init(COMP_NUMBER,&sUserCompInput);
}

/**
* @brief    : 运放底层配置
*        使能OPAMP1
* @param    : None
* @retval   : None
*/
void Board_Opamp_Init(void)
{
	 /* op-amp Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_OPA1_Msk, ENABLE);
    /*Enable the specified OPAMP1 peripheral*/
    OPAMP_Cmd(OPAMP1,ENABLE);
}

/**
  * @brief TIM2初始化函数（32位计数器，用于通用计时）
  * @param None
  * @retval None
  */
void Drv_TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		/* TIM2 Clock Enable */
		RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM2_Msk, ENABLE); //Note: TIM2 is a 32-bit up-counter/down-counter

		/* Configure TIM2 */
    TIM_TimeBaseStructure.TIM_Period = 0x000FFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CR1_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(TIM2, DISABLE);
    TIM_UpdateDisableConfig(TIM2, DISABLE);
    TIM_UpdateRequestConfig(TIM2, TIM_UpdateSource_Regular);
		/* Clear Update flag */
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		/* Reset cnt */
    TIM_SetCounter(TIM2, 0);
		/* Enable TIM2 */
    TIM_Cmd(TIM2, ENABLE);
}

///**
//  * @brief TIM3 Initialization Function
//  * @param None
//  * @retval None
//  */
/**
  * @brief TIM13初始化函数（2us计数周期，用于特定定时）
  */
void Drv_TIM13_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

		/* TIM13 Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM13_Msk, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = 24; 		// 0xFFFF
		TIM_TimeBaseStructure.TIM_Prescaler = 119; 		// 2us计数一次
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CR1_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
		TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);

		/* Enable TIM13 */
    TIM_Cmd(TIM13, ENABLE);
}

/**
  * @brief TIM14初始化函数（0.5us计数周期，用于BEMF延迟换相计时）
  */
void Drv_TIM14_Init_1(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

		/* TIM14 Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM14_Msk, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = 0xffff; 		// 0xFFFF
		TIM_TimeBaseStructure.TIM_Prescaler = 7; 		// 0.5us计数一次
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CR1_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
		TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

		/* Enable TIM14 */
    TIM_Cmd(TIM14, ENABLE);
}

/**
  * @brief TIM6初始化函数（2us计数周期，默认不使能）
  */
void Drv_TIM6_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

		/* TIM6 Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM6_Msk, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = 0xFFFF; 		// 0xFFFF
		TIM_TimeBaseStructure.TIM_Prescaler = 119; 		// 2us计数一次
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CR1_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
		TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

		/* Enable TIM6 */
    TIM_Cmd(TIM6, DISABLE);
}




/**
  * @brief 比较器无感BEMF初始化函数
  *        配置COMP2用于BEMF过零检测，非反相端接BEMF_V，反相端接BEMF_COM
  *        EXTI_Line20上升沿触发中断
  * @param None
  * @retval None
  */

void Board_SNS_Comp_Init(void)
{
	   EXTI_InitTypeDef EXTI_InitStruct;
		 NVIC_InitTypeDef NVIC_InitStruct;
	COMP_InitTypeDef COMP_InitStructure;
	/* COMP Clock Enable */

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	/* COMP Clock Enable */
	RCC_APB1PeriphClockCmd((RCC_APB1ENR_COMP_Msk), ENABLE);


	/* Configure COMP2 - 用于BEMF过零检测 */
	COMP_DeInit(COMP2);
//	COMP_InitStructure.COMP_Output    	= COMP_Output_TIM14IC1;
	COMP_StructInit(&COMP_InitStructure);
//	COMP_InitStructure.COMP_Output    	= COMP_Output_TIM14IC1;
	COMP_InitStructure.COMP_Output =	COMP_Output_None;
	COMP_InitStructure.COMP_OutputPol 	= COMP_Pol_NonInvertedOut;
	COMP_InitStructure.COMP_Hysteresis 	= COMP_Hysteresis_High;
	COMP_InitStructure.COMP_Mode       	= COMP_Mode_Low2_Power;
	COMP_InitStructure.COMP_OFLT      	= COMP_Filter_128_Period;
	COMP_InitStructure.COMP_Invert=COMP_InvertingInput_IO1;
	COMP_InitStructure.COMP_NonInvert=COMP_NonInvertingInput_IO1;
	COMP_InitStructure.COMP_OutAnaSel=COMP_AnalogOutput_Sync;
	COMP_Init(COMP2, &COMP_InitStructure);
	/* 设置比较器数字滤波器 */
	COMP2->COMPx_CSR&=(~(0x0000000F<<25));
	COMP2->COMPx_CSR|=((0x00000009<<25));
	/* 选择COMP2输入：INP=BEMF_V, INM=BEMF_COM（中性点） */
	SELECT_COMP2_INP_AS_BEMF_V();
	SELECT_COMP2_INM_AS_BEMF_O();

	/* Enable COMP2 */
	COMP_Cmd(COMP2, ENABLE);



//    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA_Msk, ENABLE);

//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
//

//    GPIO_StructInit(&GPIO_InitStruct);
//    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_2 ;
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_High;
//    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
//    GPIO_Init(GPIOA, &GPIO_InitStruct);
	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_EXTI, ENABLE);

    /* EXTI_Line20上升沿中断配置（比较器输出） */
    EXTI_StructInit(&EXTI_InitStruct);
    EXTI_InitStruct.EXTI_Line    = EXTI_Line20;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = COMP1_2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief 比较器无感模式GPIO初始化
  *        配置BEMF_U/V/W/COM引脚为模拟输入，用于无感BEMF检测
  * @param None
  * @retval None
  */
void Bsp_Comp_Sensorless_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		/* COMP Clock Enable */
		RCC_AHBPeriphClockCmd((COMP_SNS_GPIO_CLK), ENABLE);

		/*Configure BEMF pin U */
		GPIO_InitStructure.GPIO_Pin  = COMP_BEMF_U;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
		GPIO_Init(COMP_BEMF_U_PORT, &GPIO_InitStructure);

		/* BEMF V */
		GPIO_InitStructure.GPIO_Pin  = COMP_BEMF_V;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
		GPIO_Init(COMP_BEMF_V_PORT, &GPIO_InitStructure);

		/* BEMF W */
		GPIO_InitStructure.GPIO_Pin  =  COMP_BEMF_W;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
		GPIO_Init(COMP_BEMF_W_PORT, &GPIO_InitStructure);

		/* BEMF COM（中性点） */
		GPIO_InitStructure.GPIO_Pin  =  COMP_BEMF_COM;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
		GPIO_Init(COMP_BEMF_COM_PORT, &GPIO_InitStructure);
}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */

//void Drv_TIM14_Init(uint32_t period)
//{
//		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//		/* TIM14 Clock Enable */
//		RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM14_Msk, ENABLE);
//
//		/* Configure TIM14 */
//		TIM_TimeBaseStructure.TIM_Period = period;
//		TIM_TimeBaseStructure.TIM_Prescaler = 0;
//		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CR1_CKD_DIV1;
//		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
//		TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
//		/* Clear Update flag */
//		TIM_ClearFlag(TIM14, TIM_FLAG_Update);
//		/* Reset cnt */
//		TIM_SetCounter(TIM14, 0);
//
//		if(Motor_1st.SNLS.u8SNLS_mode_Flag == 1)
//		{
//			/* Enable TIM14 */
//			TIM_Cmd(TIM14, ENABLE);
//		}
//		else
//		{
//			TIM_Cmd(TIM14, DISABLE);
//		}
//}

/**
  * @brief 初始化所有配置的外设
  *        依次初始化ADC、比较器、运放、PWM、TIM13、TIM14、无感BEMF比较器
  * @param None
  * @retval None
  */
void Peripheral_Init(void)
{
    Board_ADC_Init();
    Board_Comp_Init();
    Board_Opamp_Init();
	  Drv_Pwm_Init(TIM1, PWM_PRIOD_LOAD, DEADTIME_LOAD);

		Drv_TIM13_Init();
		Drv_TIM14_Init_1();
//		Drv_TIM6_Init();
		/* Configure COMP BEMF Detect*/
		Bsp_Comp_Sensorless_Init();
		Board_SNS_Comp_Init();

		/* Configure TIM14 */
//		Drv_TIM14_Init(Motor_1st.BLDC_CMP.u32TIM14Period);

		/*Initialize WDT*/
//    Drv_Iwdg_Init();
	  /*Initialize divider*/
//    Drv_Hwdiv_Init();
}


/**
  * @}
*/

/**
  * @}
*/
