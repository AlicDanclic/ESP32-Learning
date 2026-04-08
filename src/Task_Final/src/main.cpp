#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

// ------------------------- 用户配置区域 -------------------------
// Wi-Fi 配置
const char* ssid = "";          // 修改为你的WiFi名称
const char* password = "";      // 修改为你的WiFi密码

// OpenWeatherMap API 配置
const char* apiKey = "";         // 从 openweathermap.org 获取
const char* city = "Beijing";               // 城市名（英文），例如 "Beijing" 或 "London"
const char* countryCode = "CN";             // 国家代码，例如 "CN" 中国，"US" 美国

// 更新间隔配置
const unsigned long WEATHER_UPDATE_INTERVAL = 1 * 60 * 1000;  // 天气刷新间隔：15分钟
const unsigned long NTP_SYNC_INTERVAL = 60 * 60 * 1000;        // NTP 同步间隔：1小时

// OLED 引脚配置（I2C）
const int SCL_PIN = 22;
const int SDA_PIN = 21;
// ------------------------- 用户配置区域结束 -------------------------

// 创建 OLED 对象：软件 I2C 模式，128x64 分辨率
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SCL=*/SCL_PIN, /*SDA=*/SDA_PIN, /*复位=*/U8X8_PIN_NONE);

// NTP 客户端对象
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 8 * 3600, 60000);  // UTC+8 时区，更新间隔 60 秒

// 全局变量：天气数据
String weatherDesc = "Unknown";
int temperature = 0;
bool weatherValid = false;          // 是否成功获取过天气
unsigned long lastWeatherUpdate = 0;

// 全局变量：连接状态
bool wifiConnected = false;

// 函数声明
void connectWiFi();
bool fetchWeatherData();
void drawLoading(int percent);
void drawMainScreen();
void drawErrorMessage(const char* line1, const char* line2 = nullptr);

void setup() {
  Serial.begin(115200);
  
  // 初始化 OLED
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontPosTop();
  u8g2.clearBuffer();
  u8g2.setCursor(0, 0);
  u8g2.print("Initializing...");
  u8g2.sendBuffer();
  
  // 连接 Wi-Fi
  connectWiFi();
  
  // 初始化 NTP 客户端并等待第一次时间同步
  if (wifiConnected) {
    timeClient.begin();
    // 等待 NTP 获取有效时间（最多 10 秒）
    int attempts = 0;
    while (!timeClient.update() && attempts < 20) {
      delay(500);
      attempts++;
      Serial.print(".");
    }
    if (attempts >= 20) {
      Serial.println("\nNTP 同步失败，将使用内部时钟（不准确）");
    } else {
      Serial.println("\nNTP 同步成功");
    }
  }
  
  // 首次获取天气数据
  if (wifiConnected) {
    fetchWeatherData();
  }
  
  // 载入动画（模拟启动进度）
  for (int percent = 0; percent <= 100; percent += 2) {
    drawLoading(percent);
    delay(20);
  }
  delay(500);  // 停留半秒，准备进入主界面
}

void loop() {
  // 1. 保持 Wi-Fi 连接（若断开则尝试重连）
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    connectWiFi();               // 阻塞式重连，直到成功
    if (wifiConnected) {
      // 重新连接后，重新初始化 NTP 客户端
      timeClient.begin();
      timeClient.update();
    }
  }
  
  // 2. 更新 NTP 时间（自动按内部间隔同步）
  if (wifiConnected) {
    timeClient.update();
  }
  
  // 3. 周期性更新天气数据（不阻塞显示）
  if (wifiConnected && (millis() - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)) {
    if (fetchWeatherData()) {
      lastWeatherUpdate = millis();
    } else {
      // 如果获取失败，保留旧数据，并稍后重试（下次循环再次尝试）
      Serial.println("天气获取失败，下次再试");
    }
  }
  
  // 4. 绘制主界面（时间、日期、天气、温度）
  drawMainScreen();
  
  // 控制刷新频率（约 100ms，避免闪烁且保证秒数更新平滑）
  delay(100);
}

/**
 * 连接 Wi-Fi（带 OLED 提示）
 */
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(0, 0);
  u8g2.print("Connecting WiFi...");
  u8g2.setCursor(0, 12);
  u8g2.print(ssid);
  u8g2.sendBuffer();
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {  // 最多等待15秒
    delay(500);
    Serial.print(".");
    attempts++;
    
    // 动态显示连接进度点
    u8g2.setCursor(0, 24);
    for (int i = 0; i < (attempts % 10); i++) u8g2.print(".");
    u8g2.sendBuffer();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi 已连接");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
    
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("WiFi Connected!");
    u8g2.setCursor(0, 12);
    u8g2.print(WiFi.localIP().toString());
    u8g2.sendBuffer();
    delay(1500);
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi 连接失败！");
    drawErrorMessage("WiFi Failed!", "Check SSID/PWD");
    delay(3000);
  }
}

/**
 * 从 OpenWeatherMap 获取天气数据（阻塞式，但设置了超时）
 * @return true 获取成功，false 失败
 */
bool fetchWeatherData() {
  if (!wifiConnected) return false;
  
  // 构建 API URL（使用 metric 单位，返回摄氏度）
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) +
               "," + String(countryCode) + "&units=metric&appid=" + String(apiKey);
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(5000);  // 5秒超时
  
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET 失败，错误码: %d\n", httpCode);
    http.end();
    return false;
  }
  
  String payload = http.getString();
  http.end();
  
  // 解析 JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("JSON 解析失败: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // 提取天气描述和温度
  if (doc.containsKey("weather") && doc["weather"][0].containsKey("description")) {
    weatherDesc = doc["weather"][0]["description"].as<String>();
    // 将描述首字母大写（可选）
    if (weatherDesc.length() > 0) {
      weatherDesc[0] = toupper(weatherDesc[0]);
    }
  } else {
    weatherDesc = "N/A";
  }
  
  if (doc.containsKey("main") && doc["main"].containsKey("temp")) {
    temperature = (int)round(doc["main"]["temp"].as<float>());  // 取整显示
  } else {
    temperature = 0;
  }
  
  weatherValid = true;
  Serial.print("天气更新: ");
  Serial.print(weatherDesc);
  Serial.print(", 温度: ");
  Serial.println(temperature);
  
  return true;
}

/**
 * 绘制启动加载动画（进度条 + 百分比）
 */
void drawLoading(int percent) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(0, 0);
  u8g2.print("Starting System...");
  
  // 进度条边框
  int barX = 14;
  int barY = 30;
  int barWidth = 100;
  int barHeight = 12;
  u8g2.drawFrame(barX, barY, barWidth, barHeight);
  
  // 进度条填充
  int fillWidth = map(percent, 0, 100, 0, barWidth);
  if (fillWidth > 0) {
    u8g2.drawBox(barX + 1, barY + 1, fillWidth, barHeight - 2);
  }
  
  // 百分比文字
  char percentStr[8];
  sprintf(percentStr, "%d%%", percent);
  u8g2.setCursor(barX + barWidth / 2 - 10, barY + barHeight + 5);
  u8g2.print(percentStr);
  
  // 如果 Wi-Fi 已连接，显示一个小提示
  if (wifiConnected) {
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(0, 58);
    u8g2.print("WiFi OK");
  }
  
  u8g2.sendBuffer();
}

/**
 * 显示错误信息（两行）
 */
void drawErrorMessage(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(0, 20);
  u8g2.print(line1);
  if (line2 != nullptr) {
    u8g2.setCursor(0, 35);
    u8g2.print(line2);
  }
  u8g2.sendBuffer();
}

/**
 * 绘制主界面：时间、日期、天气、温度
 */
void drawMainScreen() {
  u8g2.clearBuffer();
  
  // 获取当前时间（从 NTP 客户端）
  time_t epochTime = 0;
  struct tm *timeInfo = nullptr;
  if (wifiConnected && timeClient.isTimeSet()) {
    epochTime = timeClient.getEpochTime();
    timeInfo = localtime(&epochTime);
  } else {
    // 如果 NTP 未就绪，显示占位符
    static unsigned long fakeSeconds = 0;
    fakeSeconds++;
    static struct tm fakeTm;
    fakeTm.tm_hour = (fakeSeconds / 3600) % 24;
    fakeTm.tm_min = (fakeSeconds / 60) % 60;
    fakeTm.tm_sec = fakeSeconds % 60;
    fakeTm.tm_year = 125;  // 2025-1900
    fakeTm.tm_mon = 3;     // 4月
    fakeTm.tm_mday = 7;
    timeInfo = &fakeTm;
  }
  
  // 1. 时间（大字体，居中）
  u8g2.setFont(u8g2_font_10x20_tf);
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  int timeWidth = u8g2.getStrWidth(timeStr);
  u8g2.setCursor((128 - timeWidth) / 2, 5);
  u8g2.print(timeStr);
  
  // 2. 日期（较小字体，居中）
  u8g2.setFont(u8g2_font_6x10_tf);
  char dateStr[15];
  sprintf(dateStr, "%04d-%02d-%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday);
  int dateWidth = u8g2.getStrWidth(dateStr);
  u8g2.setCursor((128 - dateWidth) / 2, 28);
  u8g2.print(dateStr);
  
  // 分隔线
  u8g2.drawHLine(0, 38, 128);
  
  // 3. 天气和温度信息
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(10, 45);
  u8g2.print("Weather:");
  u8g2.setCursor(70, 45);
  if (weatherValid) {
    // 限制天气描述长度，避免超出屏幕
    String shortDesc = weatherDesc;
    if (shortDesc.length() > 12) shortDesc = shortDesc.substring(0, 10) + "..";
    u8g2.print(shortDesc);
  } else {
    u8g2.print("--");
  }
  
  u8g2.setCursor(10, 55);
  u8g2.print("Temp:");
  u8g2.setCursor(70, 55);
  if (weatherValid) {
    u8g2.print(temperature);
    u8g2.print(" C");
  } else {
    u8g2.print("-- C");
  }
  
  // 可选：显示 Wi-Fi 信号标识（小圆点）
  if (wifiConnected) {
    u8g2.drawDisc(120, 60, 2, U8G2_DRAW_ALL);
  } else {
    u8g2.drawFrame(118, 58, 5, 5);
  }
  
  u8g2.sendBuffer();
}