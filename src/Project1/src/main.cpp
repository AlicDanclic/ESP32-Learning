#include <Arduino.h>
#include "ESP32Button.h"

// ================= 硬件定义 =================
// 使用板载 BOOT 键 (GPIO 0) 和板载 LED (GPIO 2)
#define BUTTON_PIN  0 
#define LED_PIN     2 

// ================= 全局变量 =================
// 实例化按键对象 (GPIO 0, 低电平有效)
ESP32Button myButton(BUTTON_PIN, true);

// 记录 LED 的逻辑状态 (true = 亮)
bool ledState = false;

// ================= 辅助函数 =================

// 更新 LED 物理状态
void updateLed() {
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Serial.printf("LED 状态已更新: %s\n", ledState ? "ON" : "OFF");
}

// 演示双击效果：快速闪烁3次
void blinkEffect() {
  Serial.println("触发双击特效: 闪烁!");
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // 翻转
    delay(100);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // 翻转回原样
    delay(100);
  }
}

// ================= 主程序 =================

void setup() {
  // 1. 初始化串口
  Serial.begin(115200);
  Serial.println("\n=== ESP32 GPIO & Button Demo ===");
  Serial.println("1. 单击 -> 翻转 LED (开/关)");
  Serial.println("2. 双击 -> 快速闪烁");
  Serial.println("3. 长按 -> 强制关闭 LED");

  // 2. 初始化 LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 默认关闭

  // 3. 初始化按键
  myButton.begin();
  
  // (可选) 设置参数
  // myButton.setLongPressTime(1000); // 1秒长按
}

void loop() {
  // 核心：不断检测按键事件
  ButtonEvent event = myButton.loop();

  switch (event) {
    // ------------------------------------------
    // 功能 1: 单击 -> 状态翻转 (Toggle)
    // ------------------------------------------
    case EVENT_SHORT_PRESS:
      Serial.println("[事件] 单击检测: 切换 LED 状态");
      ledState = !ledState; // 逻辑取反
      updateLed();
      break;

    // ------------------------------------------
    // 功能 2: 双击 -> 特殊功能 (例如闪烁提示)
    // ------------------------------------------
    case EVENT_DOUBLE_CLICK:
      Serial.println("[事件] 双击检测");
      blinkEffect();
      break;

    // ------------------------------------------
    // 功能 3: 长按 -> 强制关闭 (Reset/Off)
    // ------------------------------------------
    case EVENT_LONG_PRESS:
      Serial.println("[事件] 长按检测: 强制关闭 LED");
      ledState = false;
      updateLed();
      break;

    case EVENT_NONE:
      // 无事件发生
      break;
  }

  // ------------------------------------------
  // 功能 4 (可选): 实时电平检测
  // 如果你需要 "按住亮，松开灭" 的点动模式，
  // 可以注释掉上面的 switch，改用下面的代码：
  // ------------------------------------------
  /*
  if (myButton.isPressed()) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  */
}