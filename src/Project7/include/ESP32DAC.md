# ESP32DAC 类使用说明

这是一个功能完善的 ESP32 DAC 封装类，支持直流电压输出和多种波形生成。

## ✨ 核心功能

- **双通道输出**: 支持 DAC1 (GPIO25) 和 DAC2 (GPIO26)。
- **多种输出方式**: 支持原始值、百分比、电压值设置。
- **波形生成**: 内置正弦波、三角波、锯齿波、方波生成功能。
- **非阻塞设计**: 波形生成基于 `millis()`, 不影响主程序运行。

## 🚀 快速上手

### 1. 引入文件

将 `ESP32DAC.h` 和 `ESP32DAC.cpp` 放入你的项目文件夹。

### 2. 最小代码示例

```cpp
#include "ESP32DAC.h"

// 定义DAC对象: 通道1 (GPIO25)
ESP32DAC dac(DAC_CHANNEL_1);

void setup() {
    Serial.begin(115200);
    dac.begin(); // 初始化DAC
    
    // 输出1.65V (50%)
    dac.setVoltage(1650);
}

void loop() {
    // 如果需要波形生成, 必须调用loop()
    dac.loop();
}
```

## ⚙️ API 参考

### 1. 初始化与配置

| **方法** | **说明** |
|----------|----------|
| `ESP32DAC(channel)` | **构造函数**。• `channel`: DAC通道 (`DAC_CHANNEL_1` 或 `DAC_CHANNEL_2`) |
| `void begin()` | **初始化DAC**。必须在 `setup()` 中调用。 |
| `void off()` | **关闭输出**。停止波形并输出0V。 |

### 2. 输出控制

| **方法** | **说明** |
|----------|----------|
| `void setValue(value)` | **设置原始值**。范围 0-255。 |
| `void setPercent(percent)` | **设置百分比**。范围 0.0-100.0。 |
| `void setVoltage(voltage)` | **设置电压**。范围 0-3300mV。 |
| `void setVoltageFloat(voltage)` | **设置电压**。范围 0.0-3.3V。 |
| `uint8_t getCurrentValue()` | **获取当前值**。返回 0-255。 |

### 3. 波形生成

| **方法** | **说明** |
|----------|----------|
| `void startWaveform(type, freq, amp, offset)` | **开始波形**。• `type`: 波形类型 • `freq`: 频率(Hz) • `amp`: 幅度(%) • `offset`: 偏移(%) |
| `void stopWaveform()` | **停止波形**。 |
| `void setWaveformParams(freq, amp, offset)` | **修改波形参数**。 |
| `void loop()` | **波形驱动**。必须在主 `loop()` 中持续调用。 |

### 4. 常量定义

#### DACChannel (DAC通道)

| 常量 | GPIO引脚 | 说明 |
|------|----------|------|
| `DAC_CHANNEL_1` | GPIO25 | DAC通道1 |
| `DAC_CHANNEL_2` | GPIO26 | DAC通道2 |

#### WaveformType (波形类型)

| 常量 | 说明 |
|------|------|
| `WAVE_DC` | 直流 |
| `WAVE_SINE` | 正弦波 |
| `WAVE_TRIANGLE` | 三角波 |
| `WAVE_SAWTOOTH` | 锯齿波 |
| `WAVE_SQUARE` | 方波 |

## 💡 常见应用场景

### 场景1: 可调电压输出

```cpp
#include "ESP32DAC.h"

ESP32DAC dac(DAC_CHANNEL_1);

void setup() {
    Serial.begin(115200);
    dac.begin();
}

void loop() {
    // 从0V渐变到3.3V
    for (int i = 0; i <= 3300; i += 100) {
        dac.setVoltage(i);
        Serial.printf("Voltage: %dmV\n", i);
        delay(100);
    }
}
```

### 场景2: 正弦波生成

```cpp
#include "ESP32DAC.h"

ESP32DAC dac(DAC_CHANNEL_1);

void setup() {
    Serial.begin(115200);
    dac.begin();
    // 生成1Hz正弦波, 幅度100%, 偏移50%
    dac.startWaveform(WAVE_SINE, 1.0, 100.0, 50.0);
}

void loop() {
    dac.loop(); // 驱动波形
}
```

### 场景3: 双通道输出

```cpp
#include "ESP32DAC.h"

ESP32DAC dac1(DAC_CHANNEL_1);
ESP32DAC dac2(DAC_CHANNEL_2);

void setup() {
    Serial.begin(115200);
    dac1.begin();
    dac2.begin();
    
    // 通道1: 2Hz正弦波
    dac1.startWaveform(WAVE_SINE, 2.0, 100.0, 50.0);
    
    // 通道2: 1Hz三角波
    dac2.startWaveform(WAVE_TRIANGLE, 1.0, 80.0, 50.0);
}

void loop() {
    dac1.loop();
    dac2.loop();
}
```

### 场景4: 音频信号生成

```cpp
#include "ESP32DAC.h"

ESP32DAC dac(DAC_CHANNEL_1);

// 简单音阶频率
float notes[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C4-C5
int currentNote = 0;
unsigned long noteStartTime = 0;
const unsigned long NOTE_DURATION = 500; // 每个音符500ms

void setup() {
    Serial.begin(115200);
    dac.begin();
    dac.startWaveform(WAVE_SINE, notes[0], 100.0, 50.0);
    noteStartTime = millis();
}

void loop() {
    dac.loop();
    
    // 切换音符
    if (millis() - noteStartTime > NOTE_DURATION) {
        currentNote = (currentNote + 1) % 8;
        dac.setWaveformParams(notes[currentNote], 100.0, 50.0);
        noteStartTime = millis();
    }
}
```

## ⚠️ 注意事项

1. **输出范围**: ESP32 DAC 输出范围为 0-3.3V, 8位分辨率 (256级)。
2. **驱动能力**: DAC 输出驱动能力有限, 直接驱动负载时可能需要缓冲电路。
3. **精度**: 实际输出电压可能有±50mV的误差, 高精度应用需要校准。
4. **引脚独占**: DAC 引脚 (GPIO25, GPIO26) 被 DAC 占用时不能用于其他功能。
5. **波形频率**: 软件生成的波形频率受 `loop()` 调用频率限制, 高频波形可能不够平滑。
