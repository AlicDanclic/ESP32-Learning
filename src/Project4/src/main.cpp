/**
 * 项目四：触摸传感器 —— 简易触摸开关
 * 
 * 目标：体验ESP32电容触摸功能，实现无接触式触摸开关控制LED。
 * 
 * 硬件连接：
 * - 触摸电极: 导线或铜箔 -> GPIO4 (T0)
 * - LED: GPIO2 (开发板自带LED)
 */

#include <Arduino.h>
#include "ESP32Touch.h"

// 定义触摸传感器 (GPIO4, 触摸通道T0)
ESP32Touch touch(4, 40);

// LED引脚
const int LED_PIN = 2;

// LED状态
bool ledState = false;

// 触摸计数
int touchCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目四：触摸传感器 —— 简易触摸开关");
  Serial.println("=================================");
  
  // 初始化LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 初始化触摸传感器
  touch.begin();
  
  // 自动校准 (确保无触摸时调用)
  Serial.println("正在校准触摸传感器...");
  delay(1000); // 等待稳定
  touch.calibrate(0.7); // 灵敏度0.7
  
  Serial.printf("基准值: %d\n", touch.getBaseline());
  Serial.printf("阈值: %d\n", touch.getThreshold());
  Serial.println("触摸传感器初始化完成!");
  Serial.println("触摸GPIO4引脚即可控制LED");
  Serial.println("=================================\n");
}

void loop() {
  // 处理触摸事件
  TouchEvent event = touch.loop();
  
  // 根据事件执行操作
  switch (event) {
    case TOUCH_EVENT_PRESS:
      Serial.println("[事件] 触摸按下");
      
      // 切换LED状态
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      
      // 计数
      touchCount++;
      
      Serial.printf("[状态] LED: %s, 触摸次数: %d\n", 
        ledState ? "ON" : "OFF", touchCount);
      break;
      
    case TOUCH_EVENT_RELEASE:
      Serial.println("[事件] 触摸释放");
      break;
      
    case TOUCH_EVENT_HOLD:
      Serial.println("[事件] 长按触发");
      // 长按可以执行其他操作, 比如重置计数
      if (touchCount > 0) {
        Serial.println("[操作] 重置计数器");
        touchCount = 0;
      }
      break;
      
    case TOUCH_EVENT_NONE:
    default:
      break;
  }
  
  // 可选: 实时显示触摸值 (调试用)
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 500) {
    uint16_t touchValue = touch.readRaw();
    bool isTouching = touch.isTouched();
    Serial.printf("[调试] 触摸值: %d, 状态: %s\n", 
      touchValue, isTouching ? "触摸中" : "无触摸");
    lastDebugTime = millis();
  }
}
