/**
 * 项目六：WiFi连接与网络时间获取
 * 
 * 目标：搭建WiFi网络连接，获取NTP时间，实现设备联网基础功能。
 * 
 * 硬件：
 * - ESP32开发板
 * - 2.4GHz WiFi网络
 */

#include <Arduino.h>
#include "ESP32WiFiManager.h"

// WiFi配置 (请修改为你的WiFi信息)
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// 创建WiFi管理器
ESP32WiFiManager wifi;

// 时间同步状态
bool timeSynced = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目六：WiFi连接与网络时间获取");
  Serial.println("=================================");
  
  // 初始化WiFi
  wifi.begin();
  
  // 连接WiFi
  Serial.printf("正在连接WiFi: %s\n", WIFI_SSID);
  
  if (wifi.connect(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("\nWiFi连接成功!");
    Serial.printf("IP地址: %s\n", wifi.getLocalIP().c_str());
    Serial.printf("MAC地址: %s\n", wifi.getMACAddress().c_str());
    Serial.printf("信号强度: %d dBm\n", wifi.getRSSI());
    
    // 配置NTP服务器
    // 使用中国NTP服务器, 东八区
    Serial.println("\n配置NTP服务器...");
    wifi.configNTP(
      "cn.pool.ntp.org",    // 中国NTP服务器
      "time.windows.com",   // Windows时间服务器
      "time.apple.com",     // Apple时间服务器
      8 * 3600,             // 东八区偏移 (秒)
      0                     // 夏令时偏移
    );
    
    // 同步时间
    Serial.println("正在同步时间...");
    if (wifi.syncTime(15000)) { // 15秒超时
      timeSynced = true;
      Serial.println("时间同步成功!");
      Serial.printf("当前时间: %s\n", wifi.getFormattedTime().c_str());
      Serial.printf("Unix时间戳: %lu\n", (unsigned long)wifi.getTimestamp());
    } else {
      Serial.println("时间同步失败!");
    }
    
  } else {
    Serial.println("\nWiFi连接失败!");
    Serial.println("请检查SSID和密码是否正确");
  }
  
  Serial.println("=================================\n");
}

void loop() {
  // 检查WiFi连接状态
  if (!wifi.isConnected()) {
    Serial.println("[警告] WiFi已断开, 尝试重新连接...");
    if (wifi.connect(WIFI_SSID, WIFI_PASSWORD)) {
      Serial.println("[恢复] WiFi重新连接成功!");
    }
  }
  
  // 每秒显示一次时间
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 1000) {
    if (wifi.isConnected() && timeSynced) {
      // 获取详细时间信息
      int year, month, day, hour, minute, second;
      wifi.getTime(year, month, day, hour, minute, second);
      
      Serial.println("========== 网络时间 ==========");
      Serial.printf("格式化时间: %s\n", wifi.getFormattedTime().c_str());
      Serial.printf("详细时间: %04d年%02d月%02d日 %02d:%02d:%02d\n", 
        year, month, day, hour, minute, second);
      Serial.printf("时间戳: %lu\n", (unsigned long)wifi.getTimestamp());
      
      // 显示星期
      const char* weekdays[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
      time_t now = wifi.getTimestamp();
      struct tm* timeinfo = localtime(&now);
      Serial.printf("星期: %s\n", weekdays[timeinfo->tm_wday]);
      
      // 显示WiFi信息
      Serial.printf("WiFi信号: %d dBm\n", wifi.getRSSI());
      Serial.println("==============================\n");
    } else {
      Serial.println("[等待] 等待WiFi连接和时间同步...");
    }
    
    lastPrintTime = millis();
  }
}
