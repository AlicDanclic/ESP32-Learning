<div align="center"><h1>ESP32-Learning</h1></div>

> 注意:本项目基于PlatformIO的Arduino运行

### 第一节 Hello GPIO

**GPIO**(`General Purpose Input/Output`)也就是我们常说的通用输入输出接口,作为`MCU`的基础它具备了基本的数字输入输出功能.`ESP32`提供了`34`个`GPIO`,其中工作打压为`0V`到`3.3V`,高电平输入下限为`2.0V`,低电平输入上限为0.8V.当然,我们都知道数字输入说出讲究"严于律己,宽以待人".基本上一款好的`MCU`输出电平基本上就是3.3V和0V.

`ESP32`的内部结构出来集成了相应的上拉下拉电阻以及对应的缓冲级,还额外拥有了`ESD`保护结构所以,`ESP32`对于外部的静电破坏还是很有抗性的.

`ESP32`数字`GPIO`支持一下几个功能:

- 高阻态输入(`INPUT`)
- 下拉输入(`INPUT_PULLUP`):内部`45kΩ`上拉到`3.3V`
- 上拉输入(`INPUT_PULLDOWN`):内部`45kΩ`下拉到`GND`
- 推挽输出(`OUTPUT`):可输出高电平(`3.3V`)和低电平(`0V`)
- 开漏输出(`OUTPUT_OPEN_DRAIN`):只能拉低，高电平为高阻态,需要依靠上拉电阻

ESP32的配置也是很简单的,只需要指定`IO`口编号,和类型就行.

```C++
pinMode(4,INPUT);
pinMode(5, INPUT_PULLUP);
pinMode(15, INPUT_PULLDOWN);
pinMode(2, OUTPUT);
pinMode(16, OUTPUT_OPEN_DRAIN);
```

在基础模式下,我们只讲解最简单的数字读写,具体的模拟与`PWM`,都是之后单独开一个话题,在设置在好你需要的`GPIO`模式后,只需要进行简短的函数调用即可:

- `digitalWrite(pin, value)`：设置输出电平，`HIGH`或`LOW`
- `digitalRead(pin)`：读取输入电平，返回`HIGH`或`LOW`

实际上,我们可以查看宏定义发现这样一个语句:

```c
#define LOW               0x0
#define HIGH              0x1
```

所以读写时默认`1`或者是`0`都是可以的.

### 第二节 IO引脚

在接下来的内容中很多引脚就不能随意使用,而是要注意其功能,在这里我们放入一个简单的引脚分类,大家可以自由查看:

- **电源引脚（3.3V, GND, VIN）**：ESP32的工作电压是**3.3V**，其I/O引脚也遵循此电平，连接5V设备需注意电平转换。`VIN`引脚可用于从外部给开发板供电。
- **数字输入/输出（Digital I/O）**：大部分GPIO引脚可设置为输入或输出模式，但**GPIO34、GPIO35、GPIO36、GPIO39仅能作为输入**，无法用于输出信号。
- **模拟输入（ADC - 模数转换器）**：ESP32内置18个ADC通道。需要注意的是，当Wi-Fi开启时，**ADC2**无法正常使用。因此，在需要使用Wi-Fi且进行模拟读取时，请优先选择**ADC1**的通道（例如GPIO32、GPIO33、GPIO34、GPIO35、GPIO36、GPIO39）。
- **模拟输出（DAC - 数模转换器）**：ESP32有两路8位精度的DAC，可将数字值转换为真实的模拟电压输出，对应引脚是**GPIO25 (DAC1)** 和 **GPIO26 (DAC2)**。
- **PWM（脉冲宽度调制）**：ESP32的16路PWM非常灵活，**几乎所有输出功能的GPIO**都可以通过代码配置为PWM输出，用于控制LED亮度、舵机等。
- **串行通信**：
  - **I2C**：ESP32有2个I2C控制器，**默认**的I2C引脚是**GPIO21 (SDA)** 和 **GPIO22 (SCL)**，但在Arduino环境中，你可以通过 `Wire.begin(SDA, SCL)` 将其映射到其他任意输出引脚。
  - **SPI**：ESP32有多个SPI控制器。**VSPI** 的常用引脚是：**GPIO23 (MOSI), GPIO19 (MISO), GPIO18 (CLK), GPIO5 (CS)**。**HSPI** 也有其对应的常用引脚组。
- **触摸传感器**：ESP32内置了10个电容式触摸传感器，例如GPIO4 (T0)、GPIO0 (T1)等。这些引脚可以检测微小的电容变化，常用于实现触摸按键，并能将芯片从深度睡眠中唤醒。
- **Strapping引脚**：**GPIO0, GPIO2, GPIO12, GPIO15** 等是特殊的Strapping引脚，它们在芯片上电复位时的电平状态决定了ESP32的工作模式（如正常启动或下载模式）。在硬件设计时需确保它们处于正确的电平，作为普通IO使用时也要注意其上电瞬间的状态。
- **UART**:得益于ESP32的**GPIO交换矩阵**（GPIO Matrix），UART信号可以映射到**几乎任何GPIO引脚**。这提供了极大的布线灵活性。UART0 (**TX:GPIO1, RX:GPIO3**) 通常预留给**程序下载和日志输出**。若你的项目需要使用这个串口与PC通信，需注意避免冲突。UART1默认引脚是(**TX:GPIO17,RX:GPIO16**)

### 第三节 PWM



### 第四节 ADC于DAC



### 第五节 UART



### 第六节 I2C



### 第七节 SPI



### 第八节 蓝牙供能



### 第九节 WIFI联网

