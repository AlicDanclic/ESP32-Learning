# ESP32Button 类使用说明

这是一个轻量级、非阻塞的 ESP32 按键处理类，能够轻松实现复杂的按键交互。

## ✨ 核心功能

- **软件消抖**：自动过滤机械抖动。
- **多事件支持**：精准识别 **单击**、**双击**、**长按**。
- **非阻塞设计**：基于 `millis()`，不影响主程序运行。

## 🚀 快速上手

### 1. 引入文件

将 `ESP32Button.h` 和 `ESP32Button.cpp` 放入你的项目文件夹（src/include）。

### 2. 最小代码示例

```
#include "ESP32Button.h"

// 定义按键 (GPIO 0, 默认低电平触发)
ESP32Button btn(0); 

void setup() {
    Serial.begin(115200);
    btn.begin(); // 初始化
}

void loop() {
    // 必须在 loop 中不断调用
    ButtonEvent e = btn.loop();

    // 处理事件
    switch (e) {
        case EVENT_SHORT_PRESS:  Serial.println("单击"); break;
        case EVENT_DOUBLE_CLICK: Serial.println("双击"); break;
        case EVENT_LONG_PRESS:   Serial.println("长按"); break;
    }
}
```

## ⚙️ API 参考

| **方法**                      | **说明**                                                     |
| ----------------------------- | ------------------------------------------------------------ |
| **初始化**                    |                                                              |
| `ESP32Button(pin, activeLow)` | 构造函数。`activeLow`: true=低电平触发(默认), false=高电平触发。 |
| `void begin()`                | 初始化 GPIO 模式 (必须在 setup 中调用)。                     |
| **核心逻辑**                  |                                                              |
| `ButtonEvent loop()`          | 状态机处理函数，返回当前事件（单击/双击/长按/无）。          |
| `bool isPressed()`            | 返回按键当前的实时物理状态（true=按下）。                    |
| **参数设置**                  |                                                              |
| `setDebounceTime(ms)`         | 设置消抖时间 (默认 50ms)。                                   |
| `setDoubleClickTime(ms)`      | 设置双击判定间隔 (默认 300ms)。                              |
| `setLongPressTime(ms)`        | 设置长按判定时间 (默认 1000ms)。                             |