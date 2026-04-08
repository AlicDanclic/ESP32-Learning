/**
 * 项目七：HTTP客户端与JSON解析 —— 获取天气数据
 * 
 * 目标：掌握HTTP GET请求、响应处理、JSON数据解析，从开放API获取实时天气信息。
 * 
 * 硬件：
 * - ESP32开发板
 * - 2.4GHz WiFi网络
 * 
 * 准备工作：
 * 1. 注册 OpenWeatherMap 账号: https://openweathermap.org/
 * 2. 获取免费API密钥
 * 3. 将API密钥填入下方 API_KEY
 */

#include <Arduino.h>
#include "ESP32WiFiManager.h"

void printWeather();

// ========== 配置信息 (请修改) ==========
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_openweathermap_api_key";  // 替换为你的API密钥
const char* CITY = "Beijing";  // 城市名称
// =====================================

// 创建WiFi管理器
ESP32WiFiManager wifi;

// 天气数据
WeatherData weather;

// 更新间隔 (10分钟)
const unsigned long UPDATE_INTERVAL = 600000;
unsigned long lastUpdateTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目七：HTTP客户端与JSON解析");
  Serial.println("        —— 获取天气数据");
  Serial.println("=================================");
  
  // 检查API密钥
  if (String(API_KEY) == "your_openweathermap_api_key") {
    Serial.println("\n[错误] 请先配置API密钥!");
    Serial.println("1. 访问 https://openweathermap.org/ 注册账号");
    Serial.println("2. 获取免费API密钥");
    Serial.println("3. 修改代码中的 API_KEY\n");
    while (true) {
      delay(1000);
    }
  }
  
  // 初始化WiFi
  wifi.begin();
  
  // 连接WiFi
  Serial.printf("正在连接WiFi: %s\n", WIFI_SSID);
  if (wifi.connect(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("WiFi连接成功!");
    
    // 配置NTP
    wifi.configNTP("pool.ntp.org", nullptr, nullptr, 8 * 3600, 0);
    wifi.syncTime();
    
    // 立即获取天气
    Serial.println("\n正在获取天气数据...");
    if (wifi.getWeather(API_KEY, CITY, weather)) {
      printWeather();
    } else {
      Serial.println("[错误] 获取天气失败!");
    }
    
    lastUpdateTime = millis();
    
  } else {
    Serial.println("WiFi连接失败!");
  }
  
  Serial.println("=================================\n");
}

void loop() {
  // 保持WiFi连接
  if (!wifi.isConnected()) {
    Serial.println("[警告] WiFi断开, 重新连接...");
    wifi.connect(WIFI_SSID, WIFI_PASSWORD);
  }
  
  // 定时更新天气
  if (millis() - lastUpdateTime >= UPDATE_INTERVAL) {
    Serial.println("\n[更新] 正在更新天气数据...");
    
    if (wifi.getWeather(API_KEY, CITY, weather)) {
      printWeather();
    } else {
      Serial.println("[错误] 天气更新失败!");
    }
    
    lastUpdateTime = millis();
  }
  
  // 显示当前时间
  static unsigned long lastTimePrint = 0;
  if (millis() - lastTimePrint >= 60000) { // 每分钟显示一次
    Serial.printf("\n[时间] %s\n", wifi.getFormattedTime().c_str());
    Serial.printf("[提示] 下次更新: %d秒后\n", 
      (UPDATE_INTERVAL - (millis() - lastUpdateTime)) / 1000);
    lastTimePrint = millis();
  }
}

// 打印天气信息
void printWeather() {
  if (!weather.valid) {
    Serial.println("[错误] 天气数据无效!");
    return;
  }
  
  Serial.println("\n========== 天气信息 ==========");
  Serial.printf("城市: %s\n", weather.city.c_str());
  Serial.printf("温度: %.1f°C\n", weather.temperature);
  Serial.printf("天气: %s\n", weather.description.c_str());
  Serial.printf("湿度: %d%%\n", weather.humidity);
  Serial.printf("风速: %.1f m/s\n", weather.windSpeed);
  Serial.printf("图标: %s\n", weather.icon.c_str());
  Serial.printf("数据时间: %s\n", wifi.getFormattedTime().c_str());
  Serial.println("==============================\n");
}

/**
 * 扩展示例: 使用HTTP GET请求其他API
 */
void exampleHttpRequest() {
  String response;
  
  // GET请求示例
  int httpCode = wifi.httpGet("http://httpbin.org/get", response);
  
  if (httpCode == 200) {
    Serial.println("请求成功!");
    Serial.println("响应: " + response);
    
    // 解析JSON
    StaticJsonDocument<1024> doc;
    if (wifi.parseJSON(response, doc)) {
      const char* origin = doc["origin"];
      const char* url = doc["url"];
      Serial.printf("来源IP: %s\n", origin);
      Serial.printf("请求URL: %s\n", url);
    }
  } else {
    Serial.printf("请求失败, 状态码: %d\n", httpCode);
  }
}

/**
 * 扩展示例: 使用HTTP POST请求
 */
void exampleHttpPost() {
  String response;
  
  // 构建JSON请求体
  StaticJsonDocument<256> doc;
  doc["name"] = "ESP32";
  doc["value"] = 123;
  
  String payload;
  serializeJson(doc, payload);
  
  // POST请求
  int httpCode = wifi.httpPost(
    "http://httpbin.org/post",
    payload.c_str(),
    "application/json",
    response
  );
  
  if (httpCode == 200) {
    Serial.println("POST请求成功!");
  }
}
