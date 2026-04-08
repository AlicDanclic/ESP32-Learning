/**
 * 项目三：模拟信号采集 —— 电位器与光敏电阻
 * 
 * 目标：掌握ADC配置、多通道采样、原始值到物理量转换，实现模拟信号读取与滤波。
 * 
 * 硬件连接：
 * - 电位器: 中间引脚 -> GPIO34, 两侧引脚 -> 3.3V 和 GND
 * - 光敏电阻: 一端 -> 3.3V, 另一端 -> GPIO35 和 10kΩ电阻 -> GND (分压电路)
 */

#include <Arduino.h>
#include "ESP32ADC.h"

// 定义ADC对象
ESP32ADC potentiometer(34, ADC_ATTEN_11DB, ADC_RES_12BIT);  // 电位器
ESP32ADC ldr(35, ADC_ATTEN_11DB, ADC_RES_12BIT);            // 光敏电阻

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目三：模拟信号采集");
  Serial.println("=================================");
  
  // 初始化ADC
  potentiometer.begin();
  ldr.begin();
  
  // 设置滤波模式
  // 电位器使用平均值滤波, 采样20次
  potentiometer.setFilterMode(FILTER_AVERAGE, 20);
  
  // 光敏电阻使用EMA滤波, 平滑系数0.2
  ldr.setFilterMode(FILTER_EMA, 0, 0.2);
  
  Serial.println("ADC初始化完成!");
  Serial.println("电位器通道: GPIO34 (平均值滤波)");
  Serial.println("光敏电阻通道: GPIO35 (EMA滤波)");
  Serial.println("=================================\n");
}

void loop() {
  // ========== 电位器读取 ==========
  uint16_t potRaw = potentiometer.readRaw();
  uint32_t potVoltage = potentiometer.readVoltage();
  float potPercent = potentiometer.readPercent();
  
  // 使用滤波值
  uint16_t potFiltered = potentiometer.readFiltered();
  float potFilteredPercent = (float)potFiltered / potentiometer.getMaxValue() * 100.0;
  
  // ========== 光敏电阻读取 ==========
  uint16_t ldrRaw = ldr.readRaw();
  uint32_t ldrVoltage = ldr.readVoltage();
  float ldrPercent = ldr.readPercent();
  
  // 使用滤波值
  uint16_t ldrFiltered = ldr.readFiltered();
  float ldrFilteredPercent = (float)ldrFiltered / ldr.getMaxValue() * 100.0;
  
  // ========== 串口输出 ==========
  Serial.println("========== 电位器 ==========");
  Serial.printf("原始值: %d / %d\n", potRaw, potentiometer.getMaxValue());
  Serial.printf("滤波值: %d / %d\n", potFiltered, potentiometer.getMaxValue());
  Serial.printf("电压: %dmV (%.2f%%)\n", potVoltage, potPercent);
  Serial.printf("滤波后: %.2f%%\n", potFilteredPercent);
  
  Serial.println("\n========== 光敏电阻 ==========");
  Serial.printf("原始值: %d / %d\n", ldrRaw, ldr.getMaxValue());
  Serial.printf("滤波值: %d / %d\n", ldrFiltered, ldr.getMaxValue());
  Serial.printf("电压: %dmV (%.2f%%)\n", ldrVoltage, ldrPercent);
  Serial.printf("滤波后: %.2f%%\n", ldrFilteredPercent);
  
  // 光照强度判断 (光敏电阻值越小, 光照越强)
  String lightLevel;
  if (ldrFilteredPercent < 20) {
    lightLevel = "强光";
  } else if (ldrFilteredPercent < 50) {
    lightLevel = "中等光照";
  } else if (ldrFilteredPercent < 80) {
    lightLevel = "弱光";
  } else {
    lightLevel = "黑暗";
  }
  Serial.printf("光照强度: %s\n", lightLevel.c_str());
  
  Serial.println("\n=================================\n");
  delay(1000);
}
