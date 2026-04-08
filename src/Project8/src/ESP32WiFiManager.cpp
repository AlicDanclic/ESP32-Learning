#include "ESP32WiFiManager.h"

// 构造函数
ESP32WiFiManager::ESP32WiFiManager() {
  _state = WIFI_STATE_DISCONNECTED;
  _ntpConfigured = false;
}

void ESP32WiFiManager::begin() {
  // 初始化WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
}

// ========== WiFi连接功能 ==========

bool ESP32WiFiManager::connect(const char* ssid, const char* password, unsigned long timeoutMs) {
  _ssid = String(ssid);
  _password = String(password);

  _setState(WIFI_STATE_CONNECTING);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > timeoutMs) {
      _setState(WIFI_STATE_CONNECTION_FAILED);
      Serial.println("\nConnection timeout!");
      return false;
    }
    delay(500);
    Serial.print(".");
  }

  _setState(WIFI_STATE_CONNECTED);
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  return true;
}

void ESP32WiFiManager::disconnect() {
  WiFi.disconnect(true);
  _setState(WIFI_STATE_DISCONNECTED);
}

bool ESP32WiFiManager::isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

WiFiConnectionState ESP32WiFiManager::getState() {
  // 实时更新状态
  if (WiFi.status() == WL_CONNECTED) {
    _state = WIFI_STATE_CONNECTED;
  } else if (_state == WIFI_STATE_CONNECTED) {
    _state = WIFI_STATE_DISCONNECTED;
  }
  return _state;
}

String ESP32WiFiManager::getLocalIP() {
  if (isConnected()) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}

String ESP32WiFiManager::getMACAddress() {
  return WiFi.macAddress();
}

int ESP32WiFiManager::getRSSI() {
  if (isConnected()) {
    return WiFi.RSSI();
  }
  return 0;
}

// ========== NTP时间功能 ==========

void ESP32WiFiManager::configNTP(const char* server1, const char* server2,
                                  const char* server3, long gmtOffset, int daylightOffset) {
  configTime(gmtOffset, daylightOffset, server1, server2, server3);
  _ntpConfigured = true;
}

bool ESP32WiFiManager::syncTime(unsigned long timeoutMs) {
  if (!_ntpConfigured) {
    // 使用默认配置
    configNTP("pool.ntp.org", "time.nist.gov", nullptr, 28800, 0);
  }

  Serial.print("Syncing time");
  unsigned long startTime = millis();

  while (time(nullptr) < 1000000000) { // 等待时间同步
    if (millis() - startTime > timeoutMs) {
      Serial.println("\nTime sync timeout!");
      return false;
    }
    delay(100);
    Serial.print(".");
  }

  Serial.println("\nTime synced!");
  return true;
}

time_t ESP32WiFiManager::getTimestamp() {
  return time(nullptr);
}

String ESP32WiFiManager::getFormattedTime(const char* format) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char buffer[64];
  strftime(buffer, sizeof(buffer), format, timeinfo);

  return String(buffer);
}

void ESP32WiFiManager::getTime(int& year, int& month, int& day, int& hour, int& minute, int& second) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  year = timeinfo->tm_year + 1900;
  month = timeinfo->tm_mon + 1;
  day = timeinfo->tm_mday;
  hour = timeinfo->tm_hour;
  minute = timeinfo->tm_min;
  second = timeinfo->tm_sec;
}

// ========== HTTP请求功能 ==========

int ESP32WiFiManager::httpGet(const char* url, String& response, unsigned long timeoutMs) {
  return httpRequest(HTTP_GET, url, nullptr, response, timeoutMs);
}

int ESP32WiFiManager::httpPost(const char* url, const char* payload, const char* contentType,
                                String& response, unsigned long timeoutMs) {
  if (!isConnected()) {
    response = "WiFi not connected";
    return -1;
  }

  HTTPClient http;
  http.setTimeout(timeoutMs);
  http.begin(url);
  http.addHeader("Content-Type", contentType);

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    response = http.getString();
  } else {
    response = "HTTP POST failed: " + String(http.errorToString(httpCode));
  }

  http.end();
  return httpCode;
}

int ESP32WiFiManager::httpRequest(HTTPMethod method, const char* url, const char* payload,
                                   String& response, unsigned long timeoutMs) {
  if (!isConnected()) {
    response = "WiFi not connected";
    return -1;
  }

  HTTPClient http;
  http.setTimeout(timeoutMs);
  http.begin(url);

  int httpCode = -1;

  switch (method) {
    case HTTP_GET:
      httpCode = http.GET();
      break;
    case HTTP_POST:
      if (payload) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.POST(payload);
      }
      break;
    case HTTP_PUT:
      if (payload) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.PUT(payload);
      }
      break;
    case HTTP_DELETE:
      httpCode = http.sendRequest("DELETE");
      break;
  }

  if (httpCode > 0) {
    response = http.getString();
  } else {
    response = "HTTP request failed: " + String(http.errorToString(httpCode));
  }

  http.end();
  return httpCode;
}

// ========== JSON解析功能 ==========

bool ESP32WiFiManager::parseJSON(const String& json, JsonDocument& doc) {
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

// ========== 天气获取功能 ==========

bool ESP32WiFiManager::getWeather(const char* apiKey, const char* city, WeatherData& weather,
                                   const char* units, const char* lang) {
  if (!isConnected()) {
    weather.valid = false;
    return false;
  }

  // 构建URL
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += String(city);
  url += "&appid=";
  url += String(apiKey);
  url += "&units=";
  url += String(units);
  url += "&lang=";
  url += String(lang);

  String response;
  int httpCode = httpGet(url.c_str(), response);

  if (httpCode != 200) {
    Serial.println("Weather API request failed!");
    weather.valid = false;
    return false;
  }

  // 解析JSON
  StaticJsonDocument<1024> doc;
  if (!parseJSON(response, doc)) {
    weather.valid = false;
    return false;
  }

  // 提取数据
  weather.city = doc["name"].as<String>();
  weather.temperature = doc["main"]["temp"].as<float>();
  weather.humidity = doc["main"]["humidity"].as<int>();
  weather.windSpeed = doc["wind"]["speed"].as<float>();
  weather.timestamp = doc["dt"].as<long>();

  // 天气描述
  JsonArray weatherArray = doc["weather"].as<JsonArray>();
  if (weatherArray.size() > 0) {
    weather.description = weatherArray[0]["description"].as<String>();
    weather.icon = weatherArray[0]["icon"].as<String>();
  }

  weather.valid = true;
  return true;
}

bool ESP32WiFiManager::getWeatherById(const char* apiKey, const char* cityId, WeatherData& weather,
                                       const char* units, const char* lang) {
  if (!isConnected()) {
    weather.valid = false;
    return false;
  }

  // 构建URL
  String url = "http://api.openweathermap.org/data/2.5/weather?id=";
  url += String(cityId);
  url += "&appid=";
  url += String(apiKey);
  url += "&units=";
  url += String(units);
  url += "&lang=";
  url += String(lang);

  String response;
  int httpCode = httpGet(url.c_str(), response);

  if (httpCode != 200) {
    weather.valid = false;
    return false;
  }

  StaticJsonDocument<1024> doc;
  if (!parseJSON(response, doc)) {
    weather.valid = false;
    return false;
  }

  weather.city = doc["name"].as<String>();
  weather.temperature = doc["main"]["temp"].as<float>();
  weather.humidity = doc["main"]["humidity"].as<int>();
  weather.windSpeed = doc["wind"]["speed"].as<float>();
  weather.timestamp = doc["dt"].as<long>();

  JsonArray weatherArray = doc["weather"].as<JsonArray>();
  if (weatherArray.size() > 0) {
    weather.description = weatherArray[0]["description"].as<String>();
    weather.icon = weatherArray[0]["icon"].as<String>();
  }

  weather.valid = true;
  return true;
}

// ========== 私有辅助函数 ==========

void ESP32WiFiManager::_setState(WiFiConnectionState state) {
  _state = state;
}

String ESP32WiFiManager::_methodToString(HTTPMethod method) {
  switch (method) {
    case HTTP_GET: return "GET";
    case HTTP_POST: return "POST";
    case HTTP_PUT: return "PUT";
    case HTTP_DELETE: return "DELETE";
    default: return "GET";
  }
}
