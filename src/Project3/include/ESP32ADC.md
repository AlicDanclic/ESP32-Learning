# ESP32ADC 类使用说明

这是一个功能完善的 ESP32 ADC 封装类，支持多通道采样、多种滤波算法和物理量转换。

## ✨ 核心功能

- **多通道支持**: 支持 ESP32 所有 ADC 引脚 (GPIO32-39, GPIO0, GPIO2, GPIO4, GPIO12-15, GPIO25-27)。
- **灵活配置**: 可配置衰减模式和分辨率，适应不同量程需求。
- **多种滤波**: 内置平均值、中值、指数移动平均 (EMA) 滤波算法。
- **物理量转换**: 自动将原始值转换为电压、百分比或自定义范围。

## 🚀 快速上手

### 1. 引入文件

将 `ESP32ADC.h` 和 `ESP32ADC.cpp` 放入你的项目文件夹。

### 2. 最小代码示例

```cpp
#include "ESP32ADC.h"

// 定义ADC对象: GPIO 34, 11dB衰减, 12位分辨率
ESP32ADC adc(34, ADC_ATTEN_11DB, ADC_RES_12BIT);

void setup() {
    Serial.begin(115200);
    adc.begin(); // 初始化ADC
}

void loop() {
    // 读取原始值
    uint16_t raw = adc.readRaw();
    
    // 读取电压
    uint32_t voltage = adc.readVoltage();
    
    // 读取百分比
    float percent = adc.readPercent();
    
    Serial.printf("Raw: %d, Voltage: %dmV, Percent: %.2f%%\n", raw, voltage, percent);
    delay(500);
}
```

## ⚙️ API 参考

### 1. 初始化与配置

| **方法** | **说明** |
|----------|----------|
| `ESP32ADC(pin, atten, res)` | **构造函数**。• `pin`: ADC引脚 • `atten`: 衰减模式 (默认11dB) • `res`: 分辨率 (默认12bit) |
| `void begin()` | **初始化ADC**。必须在 `setup()` 中调用。 |
| `void setAttenuation(atten)` | **设置衰减模式**。改变量程范围。 |
| `void setResolution(res)` | **设置分辨率**。改变精度 (9-12bit)。 |
| `void calibrate()` | **校准ADC**。使用内部参考电压进行校准。 |
| `void setOffset(offset)` | **设置偏移量**。手动设置校准偏移。 |

### 2. 数据读取

| **方法** | **说明** |
|----------|----------|
| `uint16_t readRaw()` | **读取原始值**。返回 0-4095 (取决于分辨率)。 |
| `uint32_t readVoltage()` | **读取电压**。返回毫伏值 (mV)。 |
| `float readVoltageFloat()` | **读取电压**。返回伏特值 (V)。 |
| `float readPercent()` | **读取百分比**。返回 0-100%。 |
| `float mapToRange(min, max)` | **范围映射**。将ADC值映射到指定范围。 |

### 3. 滤波功能

| **方法** | **说明** |
|----------|----------|
| `void setFilterMode(mode, samples, alpha)` | **设置滤波模式**。• `mode`: 滤波类型 • `samples`: 采样次数 • `alpha`: EMA系数 |
| `uint16_t readFiltered()` | **读取滤波值**。根据设置的滤波模式返回滤波后的值。 |
| `uint32_t readFilteredVoltage()` | **读取滤波电压**。返回滤波后的电压值。 |

### 4. 常量定义

#### ADCAttenuation (衰减模式)

| 常量 | 衰减值 | 量程范围 |
|------|--------|----------|
| `ADC_ATTEN_0DB` | 0dB | 0-1.1V |
| `ADC_ATTEN_2_5DB` | 2.5dB | 0-1.5V |
| `ADC_ATTEN_6DB` | 6dB | 0-2.2V |
| `ADC_ATTEN_11DB` | 11dB | 0-3.3V (推荐) |

#### ADCResolution (分辨率)

| 常量 | 位数 | 最大值 |
|------|------|--------|
| `ADC_RES_9BIT` | 9位 | 511 |
| `ADC_RES_10BIT` | 10位 | 1023 |
| `ADC_RES_11BIT` | 11位 | 2047 |
| `ADC_RES_12BIT` | 12位 | 4095 (默认) |

#### ADCFilterMode (滤波模式)

| 常量 | 说明 |
|------|------|
| `FILTER_NONE` | 无滤波 |
| `FILTER_AVERAGE` | 平均值滤波 |
| `FILTER_MEDIAN` | 中值滤波 (抗干扰) |
| `FILTER_EMA` | 指数移动平均 (平滑) |

## 💡 常见应用场景

### 场景1: 电位器读取

```cpp
ESP32ADC potentiometer(34);

void setup() {
    Serial.begin(115200);
    potentiometer.begin();
    // 使用平均值滤波, 采样20次
    potentiometer.setFilterMode(FILTER_AVERAGE, 20);
}

void loop() {
    // 映射到 0-100% 范围
    float value = potentiometer.readPercent();
    Serial.printf("Potentiometer: %.1f%%\n", value);
    delay(100);
}
```

### 场景2: 光敏电阻读取

```cpp
ESP32ADC ldr(35);

void setup() {
    Serial.begin(115200);
    ldr.begin();
    // 使用EMA滤波, 平滑系数0.2
    ldr.setFilterMode(FILTER_EMA, 0, 0.2);
}

void loop() {
    // 读取光照强度 (百分比, 越亮值越高)
    float light = ldr.readPercent();
    Serial.printf("Light: %.1f%%\n", light);
    delay(200);
}
```

### 场景3: 多通道采样

```cpp
ESP32ADC adc1(34, ADC_ATTEN_11DB, ADC_RES_12BIT);
ESP32ADC adc2(35, ADC_ATTEN_11DB, ADC_RES_12BIT);

void setup() {
    Serial.begin(115200);
    adc1.begin();
    adc2.begin();
}

void loop() {
    uint16_t val1 = adc1.readRaw();
    uint16_t val2 = adc2.readRaw();
    Serial.printf("CH1: %d, CH2: %d\n", val1, val2);
    delay(500);
}
```

## ⚠️ 注意事项

1. **ADC引脚**: ESP32 的 ADC1 (GPIO32-39) 在WiFi使用时可能受影响, ADC2 (GPIO0,2,4,12-15,25-27) 在WiFi使用时不可用。
2. **衰减选择**: 测量 0-3.3V 信号时, 请使用 `ADC_ATTEN_11DB`。
3. **分辨率**: 12位分辨率提供最高精度, 但读取速度稍慢。
4. **滤波延迟**: 平均值和中值滤波会增加读取时间, 根据需求选择合适的采样次数。
