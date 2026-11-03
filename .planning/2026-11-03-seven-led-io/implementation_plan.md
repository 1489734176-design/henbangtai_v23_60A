# Seven LED IO Implementation Plan

**Goal:** 适配用户指定新图与参考灯驱动及报警编号。
**Architecture:** 保留当前电机控制与保护结构，在board/led层更改IO映射；PWM中断分时扫描7灯与输入采样；1ms任务只读缓存。
**Spec:** 用户请求与 findings.md 中核实的原理图连接。
**Tech Stack:** bare-metal C，Keil AC6 6.22，ARM指令仿真回归。

## Decisions
- 在用户指定工程目录实施，保持参考工程只读；当前tracked源代码干净，无需额外worktree。
- 用户确认DS/TBAT本版仅预留，关闭电池温度检测，不新增阈值假设。
- GPIO扫描先断开两个阳极，再拉高全部公共阴极；选中行高/列低点灯；采样前公共端全高、两行上拉输入，等待一个PWM周期后读取。
- 9槽扫描：7个灯槽+输入准备槽+采样槽。新图没有与ADC共用的灯脚，无需参考中的ADC空窗。
- 输入首次有效采样前禁止启动；首次稳定方向确认允许当前上电扳机启动。确认完成后的方向变化才要求电机停稳且松开扳机，采样失效也保留此联锁。
- 参考程序Wrench_EN=0时不亮挡位灯，维持此规则，不擅自改变产品挡位功能。

## Tasks
- [x] 报警编号回归先失败后通过；新增运行换向/制动集成回归也先复现再修复。
- [x] 更新USER/inc/board.h、board.c、led.h、led.c、parameter.h、Bat_com.h；恢复扫描及有效采样缓存。
- [x] 更新USER/user_control.c、main.c、mm32_it.c的方向处理、采样和初始化；DS/TBAT按用户要求预留。
- [x] 执行ARM回归和现有相关测试，完成warning-reviewed Keil clean rebuild。
- [x] 独立审查源代码与引脚互斥，记录实板未测试项目和交付文件。

## Review Focus
- 切换行列时鬼影或KEY/DIRCTION被LED输出读成输入。
- PB9仍被旧双方向逻辑访问，PA11/12/14被旧通信重配置。
- 电机运行或制动中误换向、首次采样前启动、换向不松手自动重启。
- 新MOS NTC参数造成保护温度错误；DS用户明确仅预留，不启用保护。
- 故障闪灯数与参考映射不一致、LED7遗漏、休眠后继续亮灯。
