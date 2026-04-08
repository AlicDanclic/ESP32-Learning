/**
 * 项目五：外部中断与脉冲计数
 * 
 * 目标：掌握中断服务函数编写规范、多引脚中断管理，实现按键计数、频率测量。
 * 
 * 硬件连接：
 * - 按键: GPIO18 -> 按键 -> GND (使用内部上拉)
 * - LED: GPIO2 (开发板自带LED, 用于指示)
 */

#include <Arduino.h>

// 引脚定义
const int BUTTON_PIN = 18;    // 按键引脚
const int LED_PIN = 2;        // LED引脚

// 计数变量 (使用volatile, 因为在中断中修改)
volatile unsigned long pulseCount = 0;      // 脉冲计数
volatile unsigned long lastPulseTime = 0;   // 上次脉冲时间
volatile unsigned long pulseInterval = 0;   // 脉冲间隔

// 状态变量
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;    // 消抖延迟

// 频率测量变量
unsigned long lastFreqCalcTime = 0;
float currentFrequency = 0.0;

// ========== 中断服务函数 ==========
void IRAM_ATTR onPulse() {
  unsigned long now = millis();
  
  // 简单的软件消抖
  if (now - lastPulseTime > DEBOUNCE_DELAY) {
    pulseInterval = now - lastPulseTime;
    lastPulseTime = now;
    pulseCount++;
    
    // 切换LED状态 (视觉反馈)
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目五：外部中断与脉冲计数");
  Serial.println("=================================");
  
  // 初始化引脚
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // 内部上拉
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 配置外部中断
  // 模式: FALLING (下降沿触发, 按键按下时)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onPulse, FALLING);
  
  Serial.println("外部中断已配置!");
  Serial.printf("按键引脚: GPIO%d\n", BUTTON_PIN);
  Serial.println("触发模式: FALLING (下降沿)");
  Serial.println("按下按键即可计数");
  Serial.println("=================================\n");
}

void loop() {
  // ========== 计算频率 ==========
  unsigned long now = millis();
  if (now - lastFreqCalcTime >= 1000) { // 每秒计算一次频率
    // 计算频率 (Hz)
    if (pulseInterval > 0) {
      currentFrequency = 1000.0 / pulseInterval;
    } else {
      currentFrequency = 0.0;
    }
    
    lastFreqCalcTime = now;
  }
  
  // ========== 串口输出 ==========
  static unsigned long lastPrintTime = 0;
  if (now - lastPrintTime >= 500) { // 每500ms输出一次
    // 临时关闭中断, 安全读取计数变量
    noInterrupts();
    unsigned long count = pulseCount;
    unsigned long interval = pulseInterval;
    interrupts();
    
    Serial.println("========== 脉冲计数统计 ==========");
    Serial.printf("总脉冲数: %lu\n", count);
    Serial.printf("脉冲间隔: %lu ms\n", interval);
    Serial.printf("估计频率: %.2f Hz\n", currentFrequency);
    
    // 判断按键速度
    if (interval > 0 && interval < 200) {
      Serial.println("按键速度: 快速");
    } else if (interval >= 200 && interval < 500) {
      Serial.println("按键速度: 中速");
    } else if (interval >= 500) {
      Serial.println("按键速度: 慢速");
    } else {
      Serial.println("按键速度: 无");
    }
    
    Serial.println("=================================\n");
    lastPrintTime = now;
  }
}

/**
 * 扩展功能: 多引脚中断管理示例
 * 
 * 如果需要管理多个中断引脚, 可以使用以下模式:
 */

/*
// 多引脚中断管理示例
const int BUTTON1_PIN = 18;
const int BUTTON2_PIN = 19;

volatile unsigned long count1 = 0;
volatile unsigned long count2 = 0;

void IRAM_ATTR onButton1() {
  count1++;
}

void IRAM_ATTR onButton2() {
  count2++;
}

void setupMultiInterrupt() {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), onButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), onButton2, FALLING);
}
*/
