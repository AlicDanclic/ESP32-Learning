# ESP32Touch 类使用说明

这是一个轻量级、非阻塞的 ESP32 电容触摸处理类，能够轻松实现触摸开关、触摸滑条等交互功能。

## ✨ 核心功能

- **电容触摸**: 利用 ESP32 内置的电容触摸传感器, 无需额外硬件。
- **自动校准**: 自动检测环境基准值, 适应不同触摸介质。
- **事件支持**: 支持 **按下**、**释放**、**长按** 三种事件。
- **非阻塞设计**: 基于 `millis()`, 不影响主程序运行。

## 🚀 快速上手

### 1. 引入文件

将 `ESP32Touch.h` 和 `ESP32Touch.cpp` 放入你的项目文件夹。

### 2. 最小代码示例

```cpp
#include "ESP32Touch.h"

// 定义触摸传感器 (GPIO 4, 默认阈值40)
ESP32Touch touch(4);

void setup() {
    Serial.begin(115200);
    touch.begin(); // 初始化
}

void loop() {
    // 必须在 loop 中不断调用
    TouchEvent e = touch.loop();

    // 处理事件
    switch (e) {
        case TOUCH_EVENT_PRESS:   Serial.println("触摸按下"); break;
        case TOUCH_EVENT_RELEASE: Serial.println("触摸释放"); break;
        case TOUCH_EVENT_HOLD:    Serial.println("长按触发"); break;
    }
}
```

## ⚙️ API 参考

### 1. 初始化与配置

| **方法** | **说明** |
|----------|----------|
| `ESP32Touch(pin, threshold)` | **构造函数**。• `pin`: 触摸引脚 (T0-T9) • `threshold`: 触摸阈值 (默认40) |
| `void begin()` | **初始化触摸**。必须在 `setup()` 中调用。 |
| `void setThreshold(threshold)` | **设置阈值**。值越小越敏感。 |
| `uint16_t getThreshold()` | **获取阈值**。返回当前阈值。 |
| `void calibrate(sensitivity)` | **自动校准**。在无触摸时调用, 自动设置阈值。 |

### 2. 核心逻辑

| **方法** | **说明** |
|----------|----------|
| `TouchEvent loop()` | **状态机处理函数**。返回当前事件 (按下/释放/长按/无)。 |
| `bool isTouched()` | **实时检测**。返回当前是否被触摸 (无消抖)。 |
| `bool getState()` | **获取状态**。返回消抖后的触摸状态。 |
| `uint16_t readRaw()` | **读取原始值**。值越小表示触摸越强。 |

### 3. 参数设置

| **方法** | **说明** |
|----------|----------|
| `setDebounceTime(ms)` | **设置消抖时间**。默认 50ms。 |
| `setHoldTime(ms)` | **设置长按时间**。默认 500ms。 |
| `getBaseline()` | **获取基准值**。无触摸时的参考值。 |

### 4. 触摸引脚对应表

| 触摸通道 | GPIO引脚 | 备注 |
|----------|----------|------|
| T0 | GPIO 4 | 推荐 |
| T1 | GPIO 0 | Boot引脚, 慎用 |
| T2 | GPIO 2 | Boot引脚, 慎用 |
| T3 | GPIO 15 | 推荐 |
| T4 | GPIO 13 | 推荐 |
| T5 | GPIO 12 | 推荐 |
| T6 | GPIO 14 | 推荐 |
| T7 | GPIO 27 | 推荐 |
| T8 | GPIO 33 | 推荐 (仅输入) |
| T9 | GPIO 32 | 推荐 (仅输入) |

### 5. 触摸事件类型

| 常量 | 说明 |
|------|------|
| `TOUCH_EVENT_NONE` | 无事件 |
| `TOUCH_EVENT_PRESS` | 触摸按下 |
| `TOUCH_EVENT_RELEASE` | 触摸释放 |
| `TOUCH_EVENT_HOLD` | 长按触发 |

## 💡 常见应用场景

### 场景1: 触摸开关控制LED

```cpp
#include "ESP32Touch.h"

ESP32Touch touch(4);
const int LED_PIN = 2;
bool ledState = false;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    touch.begin();
}

void loop() {
    TouchEvent e = touch.loop();
    
    if (e == TOUCH_EVENT_PRESS) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        Serial.println(ledState ? "LED ON" : "LED OFF");
    }
}
```

### 场景2: 触摸调光

```cpp
#include "ESP32Touch.h"
#include "ESP32PWM.h"

ESP32Touch touch(4);
ESP32PWM led(2, 0, 5000, 8);
int brightness = 0;

void setup() {
    Serial.begin(115200);
    touch.begin();
    led.begin();
}

void loop() {
    TouchEvent e = touch.loop();
    
    if (e == TOUCH_EVENT_PRESS) {
        brightness += 25;
        if (brightness > 100) brightness = 0;
        led.setPercent(brightness);
        Serial.printf("Brightness: %d%%\n", brightness);
    }
}
```

### 场景3: 触摸滑条 (多通道)

```cpp
#include "ESP32Touch.h"

ESP32Touch touch1(4);
ESP32Touch touch2(15);
ESP32Touch touch3(13);

void setup() {
    Serial.begin(115200);
    touch1.begin();
    touch2.begin();
    touch3.begin();
}

void loop() {
    TouchEvent e1 = touch1.loop();
    TouchEvent e2 = touch2.loop();
    TouchEvent e3 = touch3.loop();
    
    if (e1 == TOUCH_EVENT_PRESS) Serial.println("位置1");
    if (e2 == TOUCH_EVENT_PRESS) Serial.println("位置2");
    if (e3 == TOUCH_EVENT_PRESS) Serial.println("位置3");
}
```

## ⚠️ 注意事项

1. **触摸介质**: 可以使用导线、铜箔、铝箔等导电材料作为触摸电极。
2. **灵敏度**: 阈值越小越敏感, 但可能产生误触发。建议通过 `calibrate()` 自动校准。
3. **防水**: 电容触摸对水敏感, 潮湿环境可能需要重新校准。
4. **引脚选择**: GPIO 0 和 GPIO 2 是 Boot 引脚, 使用时需谨慎。
5. **电源干扰**: 使用电池供电时触摸效果最佳, 电源适配器可能引入干扰。
