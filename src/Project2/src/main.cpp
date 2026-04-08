#include <Arduino.h>
#include "ESP32Button.h"
#include "ESP32PWM.h"

// --- 硬件定义 ---
#define LED_PIN    2     // 板载LED通常是GPIO 2
#define BUTTON_PIN 0     // BOOT按键通常是GPIO 0

// --- 全局对象 ---
ESP32Button button(BUTTON_PIN, true); // GPIO 0, 低电平有效
ESP32PWM led(LED_PIN, 0, 5000, 8);    // GPIO 2, Channel 0, 5KHz, 8bit

// --- 状态定义 ---
enum SystemMode {
  MODE_MANUAL,   // 手动调光模式
  MODE_BREATHING // 呼吸灯模式
};

SystemMode currentMode = MODE_MANUAL;

// 手动模式下的亮度档位索引
int brightnessIndex = 0;
const int brightnessLevels[] = {0, 30, 60, 100}; // 档位百分比
const int numLevels = 4;

// 呼吸灯控制变量
bool breathDirectionUp = true; // true=变亮, false=变暗

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Project 2: PWM Breathing Light");

  // 初始化硬件
  button.begin();
  led.begin();
  
  // 初始状态
  led.setPercent(0);
}

void loop() {
  // 1. 核心驱动循环 (必须持续调用)
  ButtonEvent event = button.loop();
  led.loop(); 

  // 2. 处理按键交互
  if (event == EVENT_DOUBLE_CLICK) {
    // 双击：切换模式
    if (currentMode == MODE_MANUAL) {
      Serial.println("Switching to BREATHING mode");
      currentMode = MODE_BREATHING;
      breathDirectionUp = true; // 重置呼吸方向
      led.fadeTo(0, 200);       // 先快速熄灭，准备开始呼吸
    } else {
      Serial.println("Switching to MANUAL mode");
      currentMode = MODE_MANUAL;
      brightnessIndex = 0;      // 重置回关闭档位
      led.fadeTo(0, 500);       // 平滑关闭
    }
  }
  else if (event == EVENT_SHORT_PRESS) {
    // 单击：仅在手动模式下有效
    if (currentMode == MODE_MANUAL) {
      brightnessIndex++;
      if (brightnessIndex >= numLevels) {
        brightnessIndex = 0;
      }
      
      int targetLevel = brightnessLevels[brightnessIndex];
      Serial.printf("Manual Dimming: Level %d (%d%%)\n", brightnessIndex, targetLevel);
      
      // 使用 fadeTo 实现平滑换挡，体验更好
      led.fadeTo(targetLevel, 300); 
    } else {
      Serial.println("Ignored: Currently in Breathing Mode (Double click to exit)");
    }
  }

  // 3. 处理呼吸灯逻辑
  if (currentMode == MODE_BREATHING) {
    // 检查当前的渐变是否完成
    if (!led.isFading()) {
      // 如果完成，则反转方向并开始新的渐变
      if (breathDirectionUp) {
        // 向上吸气：0% -> 100%，耗时 1500ms
        led.fadeTo(100, 1500);
      } else {
        // 向下呼气：100% -> 0%，耗时 1500ms
        led.fadeTo(0, 1500);
      }
      // 反转方向标志
      breathDirectionUp = !breathDirectionUp;
    }
  }
}