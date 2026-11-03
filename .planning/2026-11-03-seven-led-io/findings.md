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
