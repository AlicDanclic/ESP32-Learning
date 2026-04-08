/**
 * 项目八：综合实战 —— 智能桌面气象站
 * 
 * 目标：融合WiFi、NTP、HTTP、JSON、中断、I2C显示等模块，构建完整的物联网应用系统。
 * 
 * 硬件连接：
 * - OLED显示屏: SDA -> GPIO21, SCL -> GPIO22 (I2C)
 * - 按键1: GPIO18 (模式切换)
 * - 按键2: GPIO19 (刷新)
 * - 电位器: GPIO34 (调节亮度, 可选)
 * 
 * 依赖库:
 * - Adafruit SSD1306
 * - Adafruit GFX
 * - ArduinoJson
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP32WiFiManager.h"
#include "ESP32Button.h"
#include "ESP32ADC.h"

// ========== 配置信息 (请修改) ==========
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_openweathermap_api_key";
const char* CITY = "Beijing";
// =====================================

// OLED配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

// 引脚定义
const int BUTTON_MODE_PIN = 18;   // 模式切换按键
const int BUTTON_REFRESH_PIN = 19; // 刷新按键
const int POT_PIN = 34;           // 电位器 (亮度调节)

// 创建对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP32WiFiManager wifi;
ESP32Button btnMode(BUTTON_MODE_PIN, true);
ESP32Button btnRefresh(BUTTON_REFRESH_PIN, true);
ESP32ADC potentiometer(POT_PIN);

// 天气数据
WeatherData weather;
bool weatherValid = false;

// 显示模式
enum DisplayMode {
  MODE_TIME = 0,      // 时间模式
  MODE_WEATHER = 1,   // 天气模式
  MODE_SYSTEM = 2,    // 系统信息模式
  MODE_COUNT = 3
};
DisplayMode currentMode = MODE_TIME;

// 更新间隔
const unsigned long WEATHER_UPDATE_INTERVAL = 600000; // 10分钟
const unsigned long TIME_UPDATE_INTERVAL = 1000;      // 1秒
unsigned long lastWeatherUpdate = 0;
unsigned long lastTimeUpdate = 0;

// 屏幕亮度 (0-255)
int screenBrightness = 128;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("项目八：智能桌面气象站");
  Serial.println("=================================");
  
  // 初始化OLED
  if (!initOLED()) {
    Serial.println("[错误] OLED初始化失败!");
    while (true) delay(1000);
  }
  
  // 初始化按键
  btnMode.begin();
  btnRefresh.begin();
  
  // 初始化电位器
  potentiometer.begin();
  potentiometer.setFilterMode(FILTER_AVERAGE, 10);
  
  // 显示启动画面
  showBootScreen();
  
  // 初始化WiFi
  wifi.begin();
  
  // 连接WiFi
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.printf("SSID: %s\n", WIFI_SSID);
  display.display();
  
  if (wifi.connect(WIFI_SSID, WIFI_PASSWORD)) {
    display.println("WiFi Connected!");
    display.printf("IP: %s\n", wifi.getLocalIP().c_str());
    display.display();
    
    // 配置NTP
    wifi.configNTP("pool.ntp.org", nullptr, nullptr, 8 * 3600, 0);
    wifi.syncTime();
    
    // 获取天气
    display.println("Getting weather...");
    display.display();
    
    if (String(API_KEY) != "your_openweathermap_api_key") {
      if (wifi.getWeather(API_KEY, CITY, weather)) {
        weatherValid = true;
        display.println("Weather OK!");
      } else {
        display.println("Weather Failed!");
      }
    } else {
      display.println("No API Key!");
    }
    display.display();
    
    delay(1000);
  } else {
    display.println("WiFi Failed!");
    display.display();
    delay(2000);
  }
  
  Serial.println("系统启动完成!");
  Serial.println("=================================\n");
}

void loop() {
  // 处理按键
  handleButtons();
  
  // 调节亮度
  handleBrightness();
  
  // 保持WiFi连接
  if (!wifi.isConnected()) {
    wifi.connect(WIFI_SSID, WIFI_PASSWORD);
  }
  
  // 定时更新天气
  if (millis() - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    updateWeather();
  }
  
  // 更新显示
  if (millis() - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    updateDisplay();
    lastTimeUpdate = millis();
  }
}

// 初始化OLED
bool initOLED() {
  Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    return false;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  return true;
}

// 显示启动画面
void showBootScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println("Weather");
  display.setCursor(25, 40);
  display.println("Station");
  display.display();
  delay(1500);
}

// 处理按键
void handleButtons() {
  // 模式切换按键
  ButtonEvent modeEvent = btnMode.loop();
  if (modeEvent == EVENT_SHORT_PRESS) {
    currentMode = (DisplayMode)((currentMode + 1) % MODE_COUNT);
    Serial.printf("[按键] 切换到模式: %d\n", currentMode);
    updateDisplay();
  }
  
  // 刷新按键
  ButtonEvent refreshEvent = btnRefresh.loop();
  if (refreshEvent == EVENT_SHORT_PRESS) {
    Serial.println("[按键] 手动刷新");
    if (currentMode == MODE_WEATHER) {
      updateWeather();
    }
    updateDisplay();
  }
}

// 处理亮度调节
void handleBrightness() {
  static int lastBrightness = -1;
  
  // 读取电位器值 (0-100%)
  float percent = potentiometer.readPercent();
  screenBrightness = (int)(percent * 2.55); // 转换为0-255
  
  // 限制最小亮度
  if (screenBrightness < 10) screenBrightness = 10;
  
  // 只有亮度变化时才更新
  if (abs(screenBrightness - lastBrightness) > 5) {
    // SSD1306不支持硬件亮度调节, 这里只是记录值
    // 实际项目中可以使用PWM控制OLED的VCC或背光
    lastBrightness = screenBrightness;
    Serial.printf("[亮度] %d%%\n", (int)percent);
  }
}

// 更新天气数据
void updateWeather() {
  if (!wifi.isConnected()) return;
  if (String(API_KEY) == "your_openweathermap_api_key") return;
  
  Serial.println("[更新] 正在获取天气数据...");
  
  if (wifi.getWeather(API_KEY, CITY, weather)) {
    weatherValid = true;
    Serial.println("[更新] 天气数据更新成功!");
  } else {
    weatherValid = false;
    Serial.println("[更新] 天气数据更新失败!");
  }
  
  lastWeatherUpdate = millis();
}

// 更新显示
void updateDisplay() {
  display.clearDisplay();
  
  switch (currentMode) {
    case MODE_TIME:
      showTimeMode();
      break;
    case MODE_WEATHER:
      showWeatherMode();
      break;
    case MODE_SYSTEM:
      showSystemMode();
      break;
  }
  
  // 显示模式指示器
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.printf("Mode:%d/3  ", currentMode + 1);
  
  // 显示WiFi状态
  if (wifi.isConnected()) {
    display.printf("WiFi:%ddBm", wifi.getRSSI());
  } else {
    display.print("WiFi:--");
  }
  
  display.display();
}

// 时间模式
void showTimeMode() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== TIME MODE ===");
  
  // 显示大字体时间
  int year, month, day, hour, minute, second;
  wifi.getTime(year, month, day, hour, minute, second);
  
  display.setTextSize(2);
  display.setCursor(15, 18);
  display.printf("%02d:%02d:%02d", hour, minute, second);
  
  display.setTextSize(1);
  display.setCursor(25, 40);
  display.printf("%04d-%02d-%02d", year, month, day);
  
  // 显示星期
  const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  time_t now = wifi.getTimestamp();
  struct tm* timeinfo = localtime(&now);
  display.setCursor(50, 50);
  display.printf("%s", weekdays[timeinfo->tm_wday]);
}

// 天气模式
void showWeatherMode() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== WEATHER MODE ===");
  
  if (weatherValid) {
    display.setCursor(0, 14);
    display.printf("City: %s\n", weather.city.c_str());
    
    display.setTextSize(2);
    display.setCursor(0, 26);
    display.printf("%.1fC", weather.temperature);
    
    display.setTextSize(1);
    display.setCursor(70, 28);
    display.printf("%s", weather.description.c_str());
    
    display.setCursor(0, 46);
    display.printf("Hum:%d%%  Wind:%.1fm/s", weather.humidity, weather.windSpeed);
  } else {
    display.setCursor(10, 25);
    display.println("No weather data");
    display.setCursor(5, 40);
    display.println("Press R to refresh");
  }
}

// 系统信息模式
void showSystemMode() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== SYSTEM MODE ===");
  
  display.setCursor(0, 14);
  display.printf("IP: %s\n", wifi.getLocalIP().c_str());
  
  display.setCursor(0, 24);
  display.printf("MAC: %s\n", wifi.getMACAddress().c_str());
  
  display.setCursor(0, 34);
  if (wifi.isConnected()) {
    display.printf("RSSI: %d dBm\n", wifi.getRSSI());
  } else {
    display.println("WiFi: Disconnected");
  }
  
  display.setCursor(0, 44);
  display.printf("Heap: %d KB\n", ESP.getFreeHeap() / 1024);
  
  display.setCursor(0, 54);
  display.printf("Uptime: %lu min", millis() / 60000);
}
