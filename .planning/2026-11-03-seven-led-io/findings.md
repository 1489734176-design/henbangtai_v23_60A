# Findings

- 资料目录存在旧图 NX32F002TS-蓝汇V11-集成电钻.pdf 和新图 NX32F002-28T-方灯-7LED.pdf。
- 当前 USER/led.c 为 PA10 单灯驱动，LED_SCAN_EN=0，原扫描调用已删除。
- 当前报警编号最高 17；需对照参考代码重映射。
- 参考程序位于用户指定 shenghong/20260909 工程。

## 已核对原理图
- 新图：LED 行1/DIRCTION=PB8；行2/KEY=PA14；COM1=PA13；COM2=PA0；COM3=PA10；COM4=PB9；EN=PA11；LED+=PA12。
- 物理 LED1/2/3 在行1接 COM1/2/3，物理 LED4/5/6/7 在行2接 COM1/2/3/4。
- 参考逻辑灯1/2/3用于电量和同步故障闪烁，对应物理LED4/5/6；逻辑灯4/5/6为挡位，对应物理LED1/2/3。
- 新图 TBAT=PA3/ADC2，TMOS=PA4/ADC3；PWM/BEMF/电流/扳机引脚不变。两个 R002 并联仍为 1 mOhm。
- MOS NTC=100K/B3950，上拉4.7K；现程序10K/B3435，上拉10K，需保持95C保护/85C恢复目标并换算ADC。
- 用户确认新板按DS/TBAT检测电池温度；外部信号类型/NTC参数及第7灯用途等待答复。
- 参考报警1..12依次为硬件过流、电池温度、MOS温度、堵转、软件过流(错误字bit1..4)、欠压、飞钻、过压、桥臂、缺相、失步、通信；13为兜底。当前额外gIBusAvgOCP4(bit5)应独立13，兜底14。
- 当前方向逻辑直接读PB8/PB9，需改读扫描缓存；保留停稳换向与松手重新扣动联锁。
- Keil AC6 6.22 安装位置 D:/Users/mym02/AppData/Local/Keil_v5，UV4/UV4.exe 可用。
- pdftoppm渲染成功；pdftotext不在PATH，直接视觉核对两页完整图纸。

## 2026-11-03 取消10s/30s照明延时

- 参考USER/inc/parameter.h:25为SLEEP_TIME=5000ul；USER/user_control.c的TOOL_STOP和TOOL_ERROR_STOP均按++sleep_cnt>SLEEP_TIME休眠，故障扳机按住时计数清零。
- 参考USER/led.c:485起的UO_Led_Handler在TOOL_POWER_DOWN关闭矩阵，其余时间显示电量/按住故障报警；没有独立10s/30s照明计时。
- 当前两处休眠条件附加light_ctrl.state==LIGHT_OFF，使照明10s/30s策略影响整板休眠。应移除此条件及Light_Handler状态机；新板PA12照明随TOOL_POWER_DOWN统一关灯。
- 本轮起始源码已被用户提交；现有修改仅Keil UI/构建日志和未跟踪.claude，保留。

## 2026-11-03 烧录口/扫描复用反馈

- 用户实板反馈：初版未改IO时可重复烧录，适配后只能烧一次；要求扫描逻辑尤其烧录口复用按参考。
- 本轮HEAD=2cfbb33，tracked工作区干净；用户已提交手感及过流参数修改，必须保留。
- 当前led_init将PA13固定推挽输出，9槽扫描从不释放PA13；PA14有两槽输入窗口。参考led_init先将灯/COM浮空，12槽扫描中7..10全部模拟输入高阻，11准备输入，12采样。
- 参考与当前均设置PA13/PA14 AF1，参考没有恢复AF0代码；不能仅把AF1归因于本次差异，也不能说参考高阻窗口等于恢复SWD功能。
- vendor GPIO_AFIODeInit的AFRH复位值0xF00FFFFF，注明PA14 SWCLK、PA13 SWDIO，即SWD复用选择AF0；具体调试接管行为仍需进一步证据。
- 用户烧录器快捷方式解析：D:/烧录机/LCD_ALL_PRGMR_V41/LCD_ALL_PRGMR_V41.exe。正读取软件说明及本地芯片资料，未操作烧录/擦除。

- 更强启动差异：当前Bsp_Gpio_Init提前调用Bsp_Key_Init将PA14改AF1，发生在100ms校准之前；参考及Git初版均不在这里初始化KEY，直到校准后才接管。已移除共享DIR/KEY的早期调用。
- 完整参考模式扩展：6灯12槽变为7灯13槽；点灯1..7，全部高阻8..11，输入准备12，采样并行低电平收尾13。250us高阻及62.5us上拉稳定保持；ADC独立因此不启用ADC复用标志。
- 本地MM32 DS表6确认SWD AF0；官方GPIO_SWD_Remap示例明确复用后需手动复位再下载，不支持“高阻等于恢复SWD”说法。资料在D:/mym/资料/MM32/0230/DS_MM32SPIN0230_SC.pdf、UM_MM32SPIN0230_SC_v1.1.pdf及LibSamples_MM32SPIN0230_V1.6.10/Samples/LibSamples/GPIO/GPIO_SWD_Remap/gpio_swd_remap.c。
- LCD本地版本说明支持设置SWD频率、MM32SPIN023x支持需烧录器固件>=13.25；未找到目标上电/复位连接步骤。旧固件板需真正MCU复位并及时连接，不修改读保护或NRST选项。
