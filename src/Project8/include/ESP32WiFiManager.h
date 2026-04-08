#ifndef ESP32_WIFI_MANAGER_H
#define ESP32_WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi连接状态枚举
enum WiFiConnectionState {
  WIFI_STATE_DISCONNECTED = 0,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_CONNECTION_FAILED
};

// HTTP请求方法枚举
enum HTTPMethod {
  HTTP_GET = 0,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE
};

// 天气数据结构
struct WeatherData {
  String city;
  float temperature;
  String description;
  int humidity;
  float windSpeed;
  String icon;
  long timestamp;
  bool valid;
};

class ESP32WiFiManager {
public:
  /**
   * @brief 构造函数
   */
  ESP32WiFiManager();

  /**
   * @brief 初始化WiFi
   */
  void begin();

  // ========== WiFi连接功能 ==========

  /**
   * @brief 连接到WiFi网络
   * @param ssid WiFi名称
   * @param password WiFi密码
   * @param timeoutMs 超时时间 (毫秒, 默认30000ms)
   * @return true=连接成功, false=连接失败
   */
  bool connect(const char* ssid, const char* password, unsigned long timeoutMs = 30000);

  /**
   * @brief 断开WiFi连接
   */
  void disconnect();

  /**
   * @brief 检查是否已连接
   * @return true=已连接, false=未连接
   */
  bool isConnected();

  /**
   * @brief 获取连接状态
   * @return WiFi连接状态
   */
  WiFiConnectionState getState();

  /**
   * @brief 获取本地IP地址
   * @return IP地址字符串
   */
  String getLocalIP();

  /**
   * @brief 获取MAC地址
   * @return MAC地址字符串
   */
  String getMACAddress();

  /**
   * @brief 获取信号强度 (RSSI)
   * @return 信号强度 (dBm)
   */
  int getRSSI();

  // ========== NTP时间功能 ==========

  /**
   * @brief 配置NTP服务器
   * @param server1 主NTP服务器
   * @param server2 备用NTP服务器1 (可选)
   * @param server3 备用NTP服务器2 (可选)
   * @param gmtOffset 时区偏移 (秒, 默认东八区 8*3600)
   * @param daylightOffset 夏令时偏移 (秒, 默认0)
   */
  void configNTP(const char* server1, const char* server2 = nullptr,
                 const char* server3 = nullptr, long gmtOffset = 28800,
                 int daylightOffset = 0);

  /**
   * @brief 同步NTP时间
   * @param timeoutMs 超时时间 (毫秒, 默认10000ms)
   * @return true=同步成功, false=同步失败
   */
  bool syncTime(unsigned long timeoutMs = 10000);

  /**
   * @brief 获取当前时间戳
   * @return Unix时间戳 (秒)
   */
  time_t getTimestamp();

  /**
   * @brief 获取格式化时间字符串
   * @param format 时间格式 (strftime格式, 默认 "%Y-%m-%d %H:%M:%S")
   * @return 格式化时间字符串
   */
  String getFormattedTime(const char* format = "%Y-%m-%d %H:%M:%S");

  /**
   * @brief 获取年月日时分秒
   * @param year 年
   * @param month 月
   * @param day 日
   * @param hour 时
   * @param minute 分
   * @param second 秒
   */
  void getTime(int& year, int& month, int& day, int& hour, int& minute, int& second);

  // ========== HTTP请求功能 ==========

  /**
   * @brief 发送HTTP GET请求
   * @param url 请求URL
   * @param response 响应内容
   * @param timeoutMs 超时时间 (毫秒, 默认10000ms)
   * @return HTTP状态码 (-1表示请求失败)
   */
  int httpGet(const char* url, String& response, unsigned long timeoutMs = 10000);

  /**
   * @brief 发送HTTP POST请求
   * @param url 请求URL
   * @param payload 请求体内容
   * @param contentType Content-Type头
   * @param response 响应内容
   * @param timeoutMs 超时时间 (毫秒, 默认10000ms)
   * @return HTTP状态码 (-1表示请求失败)
   */
  int httpPost(const char* url, const char* payload, const char* contentType,
               String& response, unsigned long timeoutMs = 10000);

  /**
   * @brief 发送HTTP请求 (通用)
   * @param method 请求方法
   * @param url 请求URL
   * @param payload 请求体内容 (GET时可为空)
   * @param response 响应内容
   * @param timeoutMs 超时时间 (毫秒, 默认10000ms)
   * @return HTTP状态码 (-1表示请求失败)
   */
  int httpRequest(HTTPMethod method, const char* url, const char* payload,
                  String& response, unsigned long timeoutMs = 10000);

  // ========== JSON解析功能 ==========

  /**
   * @brief 解析JSON字符串
   * @param json JSON字符串
   * @param doc JsonDocument对象
   * @return true=解析成功, false=解析失败
   */
  bool parseJSON(const String& json, JsonDocument& doc);

  /**
   * @brief 解析JSON字符串 (静态缓冲区版本)
   * @param json JSON字符串
   * @param capacity 缓冲区大小
   * @return StaticJsonDocument对象
   */
  template<size_t N>
  StaticJsonDocument<N> parseJSON(const String& json) {
    StaticJsonDocument<N> doc;
    deserializeJson(doc, json);
    return doc;
  }

  // ========== 天气获取功能 ==========

  /**
   * @brief 获取天气数据 (使用OpenWeatherMap API)
   * @param apiKey API密钥
   * @param city 城市名称
   * @param weather 天气数据结构
   * @param units 单位 ("metric"=摄氏度, "imperial"=华氏度)
   * @param lang 语言 ("zh_cn"=中文)
   * @return true=获取成功, false=获取失败
   */
  bool getWeather(const char* apiKey, const char* city, WeatherData& weather,
                  const char* units = "metric", const char* lang = "zh_cn");

  /**
   * @brief 获取天气数据 (使用OpenWeatherMap API, 按城市ID)
   * @param apiKey API密钥
   * @param cityId 城市ID
   * @param weather 天气数据结构
   * @param units 单位
   * @param lang 语言
   * @return true=获取成功, false=获取失败
   */
  bool getWeatherById(const char* apiKey, const char* cityId, WeatherData& weather,
                      const char* units = "metric", const char* lang = "zh_cn");

private:
  WiFiConnectionState _state;
  String _ssid;
  String _password;
  bool _ntpConfigured;

  // 内部辅助函数
  void _setState(WiFiConnectionState state);
  String _methodToString(HTTPMethod method);
};

#endif
