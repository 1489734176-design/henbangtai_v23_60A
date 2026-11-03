/**
 * @file     drv_comp.h
 * @author   MindMotion Motor Team : Wesson
 * @brief    This file provides all the functions prototypes for the COMP.
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
#ifndef __DRV_COMP_H
#define __DRV_COMP_H

/** Files includes */
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"

/**
 * @addtogroup MM32_Hardware_Driver_Layer
 * @{
 */

/**
 * @addtogroup Drv_COMP
 * @{
 */

//(Note: there needs "()" for the macro "Bit()", since it may be prefixed with "!")
/** @brief 生成第n位的位掩码 */
#define Bit(n) ((uint32_t)1<<(n))

// 位定义宏，用于位操作
#define Bit0   Bit(0)
#define Bit1   Bit(1)
#define Bit2   Bit(2)
#define Bit3   Bit(3)
#define Bit4   Bit(4)
#define Bit5   Bit(5)
#define Bit6   Bit(6)
#define Bit7   Bit(7)
#define Bit8   Bit(8)
#define Bit9   Bit(9)
#define Bit10  Bit(10)
#define Bit11  Bit(11)
#define Bit12  Bit(12)
#define Bit13  Bit(13)
#define Bit14  Bit(14)
#define Bit15  Bit(15)
#define Bit16  Bit(16)
#define Bit17  Bit(17)
#define Bit18  Bit(18)
#define Bit19  Bit(19)
#define Bit20  Bit(20)
#define Bit21  Bit(21)
#define Bit22  Bit(22)
#define Bit23  Bit(23)
#define Bit24  Bit(24)
#define Bit25  Bit(25)
#define Bit26  Bit(26)
#define Bit27  Bit(27)
#define Bit28  Bit(28)
#define Bit29  Bit(29)
#define Bit30  Bit(30)
#define Bit31  Bit(31)


//bit width for macro 'setv()' —— 位宽度定义，用于setv()宏
#define BW1    1
#define BW2    2
#define BW3    3
#define BW4    4
#define BW5    5
#define BW6    6
#define BW7    7
#define BW8    8
#define BW9    9
#define BW10   10
#define BW11   11
#define BW12   12
#define BW13   13
#define BW14   14
#define BW15   15
#define BW16   16
#define BW17   17
#define BW18   18
#define BW19   19
#define BW20   20
#define BW21   21
#define BW22   22
#define BW23   23
#define BW24   24
#define BW25   25
#define BW26   26
#define BW27   27
#define BW28   28
#define BW29   29
#define BW30   30
#define BW31   31
#define BW32   32

//bit shift for macro 'setv()' —— 位偏移定义，用于setv()宏
#define BS0    0
#define BS1    1
#define BS2    2
#define BS3    3
#define BS4    4
#define BS5    5
#define BS6    6
#define BS7    7
#define BS8    8
#define BS9    9
#define BS10   10
#define BS11   11
#define BS12   12
#define BS13   13
#define BS14   14
#define BS15   15
#define BS16   16
#define BS17   17
#define BS18   18
#define BS19   19
#define BS20   20
#define BS21   21
#define BS22   22
#define BS23   23
#define BS24   24
#define BS25   25
#define BS26   26
#define BS27   27
#define BS28   28
#define BS29   29
#define BS30   30
#define BS31   31

//set bits and clear bits in a register —— 寄存器位操作宏
#define setb(reg,bit)  reg |= (bit)           // 置位寄存器中的指定位
#define clrb(reg,bit)  reg &= ~(bit)          // 清零寄存器中的指定位

//fill bits value in a register (BW: bit width; BS: bit shift) —— 向寄存器的指定字段写入值
//(Note: there needs "{}" for the macro "setv()", since it contains more than one statements)
#define setv(reg,BS,BW,value)  {reg &= ~((((uint32_t)1<<(BW))-1)<<(BS)); reg |= ((uint32_t)(value)<<(BS));}

//write '1' to clear bit in a register —— 写1清零寄存器位（用于W1C标志位）
#define write_1_clrb(reg,bit)  reg |= (bit)

//check bit value in a register —— 检查寄存器中的指定位是否为1
//(Note: there needs "()" since it may be prefixed with "!")
#define chkb(reg,bit)  (reg & (bit))

//Comparator Module —— 比较器1模块（用于BEMF过零检测）
////////-------------------------------------/////////
/** @brief 选择COMP1正端输入为U相BEMF（PA3/PA5/PA6/PB0可选） */
#define SELECT_COMP1_INP_AS_BEMF_U()        setv(COMP->COMP1_CSR,BS7,BW2,0) //0: PA3;  1:PA5;  2: PA6;  3:PB0
/** @brief 选择COMP1正端输入为V相BEMF */
#define SELECT_COMP1_INP_AS_BEMF_V()        setv(COMP->COMP1_CSR,BS7,BW2,1) //0: PA3;  1:PA5;  2: PA6;  3:PB0
/** @brief 选择COMP1正端输入为W相BEMF */
#define SELECT_COMP1_INP_AS_BEMF_W()        setv(COMP->COMP1_CSR,BS7,BW2,3) //0: PA3;  1:PA5;  2: PA6;  3:PB0
/** @brief 选择COMP1负端输入为BEMF中性点（PA4/PA7/PA8/CRV可选） */
#define SELECT_COMP1_INM_AS_BEMF_O()       	setv(COMP->COMP1_CSR,BS4,BW3,2) //0: PA4;  1:PA7;  2: PA8; 	3:CRV
/** @brief 读取COMP1输出状态 */
#define GET_COMP1_OUT()                     (COMP->COMP1_CSR & 0x40000000)


//Comparator Module —— 比较器2模块（用于BEMF过零检测，当前使用）
////////-------------------------------------/////////

#if 0
// 比较器2输入选择定义（备用方案）
#define   COMP2_CSR_INM0_PA4            COMP_CSR_INM_SEL_0
#define   COMP2_CSR_INM1_PA7            COMP_CSR_INM_SEL_1
#define   COMP2_CSR_INM2_PB1            COMP_CSR_INM_SEL_2
#define   COMP2_CSR_INM3_CRV            COMP_CSR_INM_SEL_3

#define   COMP2_CSR_INP0_PB0            COMP_CSR_INP_SEL_0
#define   COMP2_CSR_INP1_PB2            COMP_CSR_INP_SEL_1
#define   COMP2_CSR_INP2_PA8            COMP_CSR_INP_SEL_2
#define   COMP2_CSR_INP3_PA9            COMP_CSR_INP_SEL_3

#define COMP2_CSR_INP_BITS	(0x00000180)
#define SELECT_COMP2_INP_AS_BEMF_U()    {COMP->COMP2_CSR &= (~COMP2_CSR_INP_BITS); COMP->COMP2_CSR |= COMP2_CSR_INP3_PA9;}
#define SELECT_COMP2_INP_AS_BEMF_V()    {COMP->COMP2_CSR &= (~COMP2_CSR_INP_BITS); COMP->COMP2_CSR |= COMP2_CSR_INP2_PA8;}
#define SELECT_COMP2_INP_AS_BEMF_W()    {COMP->COMP2_CSR &= (~COMP2_CSR_INP_BITS); COMP->COMP2_CSR |= COMP2_CSR_INP1_PB2;}

#define COMP2_CSR_INM_BITS	(0x00000070)
#define SELECT_COMP2_INM_AS_BEMF_O()    {COMP->COMP2_CSR &= (~COMP2_CSR_INM_BITS); COMP->COMP2_CSR |= COMP2_CSR_INM0_PA4;}
#endif

#if 1

/** @brief COMP2正端输入通道编码：U相BEMF对应值3 */
#define COMP_CH_BEMF_U   3
/** @brief COMP2正端输入通道编码：V相BEMF对应值2 */
#define COMP_CH_BEMF_V   2
/** @brief COMP2正端输入通道编码：W相BEMF对应值1 */
#define COMP_CH_BEMF_W   1


/** @brief 选择COMP2正端输入为U相BEMF（PB0/PB2/PA8/PA9可选，U=PA9） */
#define SELECT_COMP2_INP_AS_BEMF_U()        setv(COMP2->COMPx_CSR,BS7,BW2,3) //0: PB0;  1:PB2;  2: PA8;  3:PA9
/** @brief 选择COMP2正端输入为V相BEMF（V=PA8） */
#define SELECT_COMP2_INP_AS_BEMF_V()        setv(COMP2->COMPx_CSR,BS7,BW2,2) //0: PB0;  1:PB2;  2: PA8;  3:PA9
/** @brief 选择COMP2正端输入为W相BEMF（W=PB2） */
#define SELECT_COMP2_INP_AS_BEMF_W()        setv(COMP2->COMPx_CSR,BS7,BW2,1) //0: PB0;  1:PB2;  2: PA8;  3:PA9
/** @brief 选择COMP2负端输入为BEMF中性点（PA4/PA7/PB1/CRV可选，中性点=PB1） */
#define SELECT_COMP2_INM_AS_BEMF_O()       	setv(COMP2->COMPx_CSR,BS4,BW3,2) //0: PA4;  1:PA7;  2: PB1; 	3:CRV
#endif

/** @brief 读取COMP2输出状态 */
#define GET_COMP2_OUT()                     (COMP2->COMPx_CSR & 0x40000000)

/** @brief 读取当前使用的比较器输出（当前使用COMP2） */
#define GET_COMP_OUT()      				        GET_COMP2_OUT()			//	GET_COMP4_OUT()

/**
 * @brief 比较器输入配置结构体
 * @note  用于配置比较器的正端输入、负端输入和CRV选择
 */
typedef struct
{
    uint32_t   sCompNonInvertingInput;  // 正端输入选择
    uint32_t      sCompInvertingInput;  // 负端输入选择
    uint8_t                     u8CompCrvSelect;  // 内部参考电压(CRV)选择
}COMP_Input_TypeDef;


void select_comp_ch_for_sector(u8 sector_index,u8 dir);
extern void Drv_Comp_Init(COMP_TypeDef *comp,COMP_Input_TypeDef * pCompInput);

/**
  * @}
*/

/**
  * @}
*/

#endif
