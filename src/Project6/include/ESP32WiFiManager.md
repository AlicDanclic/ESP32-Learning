# ESP32WiFiManager 类使用说明

这是一个功能完善的 ESP32 WiFi 管理类，集成了 WiFi 连接、NTP 时间同步、HTTP 请求和 JSON 解析功能。

## ✨ 核心功能

- **WiFi连接**: 支持连接到2.4GHz WiFi网络, 自动重连。
- **NTP时间同步**: 自动从NTP服务器获取时间, 支持时区设置。
- **HTTP请求**: 支持GET/POST/PUT/DELETE请求, 内置超时处理。
- **JSON解析**: 集成ArduinoJson库, 轻松解析API响应。
- **天气获取**: 内置OpenWeatherMap API支持, 一键获取天气数据。

## 🚀 快速上手

### 1. 引入文件

将 `ESP32WiFiManager.h` 和 `ESP32WiFiManager.cpp` 放入你的项目文件夹。

**platformio.ini 依赖:**
```ini
lib_deps =
    bblanchon/ArduinoJson @ ^6.21.0
```

### 2. 最小代码示例

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;

const char* SSID = "your_wifi_ssid";
const char* PASSWORD = "your_wifi_password";

void setup() {
    Serial.begin(115200);
    wifi.begin();
    
    // 连接WiFi
    if (wifi.connect(SSID, PASSWORD)) {
        Serial.println("WiFi connected!");
        Serial.println("IP: " + wifi.getLocalIP());
    }
}

void loop() {
    // 检查连接状态
    if (!wifi.isConnected()) {
        Serial.println("WiFi disconnected!");
    }
    delay(5000);
}
```

## ⚙️ API 参考

### 1. 初始化与连接

| **方法** | **说明** |
|----------|----------|
| `ESP32WiFiManager()` | **构造函数**。 |
| `void begin()` | **初始化WiFi**。必须在 `setup()` 中调用。 |
| `bool connect(ssid, password, timeout)` | **连接WiFi**。返回连接结果。 |
| `void disconnect()` | **断开连接**。 |
| `bool isConnected()` | **检查连接状态**。返回是否已连接。 |
| `WiFiConnectionState getState()` | **获取连接状态**。返回详细状态枚举。 |

### 2. 网络信息

| **方法** | **说明** |
|----------|----------|
| `String getLocalIP()` | **获取IP地址**。返回本地IP字符串。 |
| `String getMACAddress()` | **获取MAC地址**。返回MAC地址字符串。 |
| `int getRSSI()` | **获取信号强度**。返回信号强度(dBm)。 |

### 3. NTP时间功能

| **方法** | **说明** |
|----------|----------|
| `void configNTP(s1, s2, s3, offset, dst)` | **配置NTP服务器**。• `s1-s3`: NTP服务器 • `offset`: 时区偏移(秒) • `dst`: 夏令时偏移 |
| `bool syncTime(timeout)` | **同步时间**。阻塞等待时间同步。 |
| `time_t getTimestamp()` | **获取时间戳**。返回Unix时间戳。 |
| `String getFormattedTime(format)` | **获取格式化时间**。默认格式 "YYYY-MM-DD HH:MM:SS"。 |
| `void getTime(y, m, d, h, min, s)` | **获取时间分量**。获取年月日时分秒。 |

### 4. HTTP请求功能

| **方法** | **说明** |
|----------|----------|
| `int httpGet(url, response, timeout)` | **GET请求**。返回HTTP状态码。 |
| `int httpPost(url, payload, type, response, timeout)` | **POST请求**。返回HTTP状态码。 |
| `int httpRequest(method, url, payload, response, timeout)` | **通用请求**。支持GET/POST/PUT/DELETE。 |

### 5. JSON解析功能

| **方法** | **说明** |
|----------|----------|
| `bool parseJSON(json, doc)` | **解析JSON**。将字符串解析为JsonDocument。 |

### 6. 天气获取功能

| **方法** | **说明** |
|----------|----------|
| `bool getWeather(key, city, data, units, lang)` | **获取天气**。按城市名称获取天气。 |
| `bool getWeatherById(key, id, data, units, lang)` | **获取天气**。按城市ID获取天气。 |

### 7. 数据结构

#### WeatherData (天气数据)

| 字段 | 类型 | 说明 |
|------|------|------|
| `city` | String | 城市名称 |
| `temperature` | float | 温度 |
| `description` | String | 天气描述 |
| `humidity` | int | 湿度 (%) |
| `windSpeed` | float | 风速 (m/s) |
| `icon` | String | 天气图标代码 |
| `timestamp` | long | 数据时间戳 |
| `valid` | bool | 数据是否有效 |

### 8. 枚举定义

#### WiFiConnectionState (WiFi连接状态)

| 常量 | 说明 |
|------|------|
| `WIFI_STATE_DISCONNECTED` | 未连接 |
| `WIFI_STATE_CONNECTING` | 连接中 |
| `WIFI_STATE_CONNECTED` | 已连接 |
| `WIFI_STATE_CONNECTION_FAILED` | 连接失败 |

#### HTTPMethod (HTTP请求方法)

| 常量 | 说明 |
|------|------|
| `HTTP_GET` | GET请求 |
| `HTTP_POST` | POST请求 |
| `HTTP_PUT` | PUT请求 |
| `HTTP_DELETE` | DELETE请求 |

## 💡 常见应用场景

### 场景1: 获取NTP时间

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;

void setup() {
    Serial.begin(115200);
    wifi.begin();
    
    if (wifi.connect("SSID", "PASSWORD")) {
        // 配置NTP (东八区)
        wifi.configNTP("pool.ntp.org", "time.nist.gov", nullptr, 8 * 3600, 0);
        
        // 同步时间
        if (wifi.syncTime()) {
            Serial.println("Current time: " + wifi.getFormattedTime());
        }
    }
}

void loop() {
    // 每秒打印时间
    Serial.println(wifi.getFormattedTime());
    delay(1000);
}
```

### 场景2: HTTP GET请求

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;

void setup() {
    Serial.begin(115200);
    wifi.begin();
    
    if (wifi.connect("SSID", "PASSWORD")) {
        String response;
        int code = wifi.httpGet("http://httpbin.org/get", response);
        
        Serial.printf("HTTP Code: %d\n", code);
        Serial.println("Response: " + response);
    }
}

void loop() {}
```

### 场景3: 解析JSON数据

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;

void setup() {
    Serial.begin(115200);
    wifi.begin();
    
    if (wifi.connect("SSID", "PASSWORD")) {
        String response;
        wifi.httpGet("http://api.example.com/data", response);
        
        StaticJsonDocument<512> doc;
        if (wifi.parseJSON(response, doc)) {
            const char* name = doc["name"];
            float value = doc["value"];
            Serial.printf("Name: %s, Value: %.2f\n", name, value);
        }
    }
}

void loop() {}
```

### 场景4: 获取天气数据

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;
const char* API_KEY = "your_openweathermap_api_key";

void setup() {
    Serial.begin(115200);
    wifi.begin();
    
    if (wifi.connect("SSID", "PASSWORD")) {
        WeatherData weather;
        
        if (wifi.getWeather(API_KEY, "Beijing", weather)) {
            Serial.printf("City: %s\n", weather.city.c_str());
            Serial.printf("Temperature: %.1f°C\n", weather.temperature);
            Serial.printf("Description: %s\n", weather.description.c_str());
            Serial.printf("Humidity: %d%%\n", weather.humidity);
            Serial.printf("Wind Speed: %.1f m/s\n", weather.windSpeed);
        }
    }
}

void loop() {}
```

### 场景5: 定时获取天气

```cpp
#include "ESP32WiFiManager.h"

ESP32WiFiManager wifi;
const char* API_KEY = "your_api_key";
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 600000; // 10分钟

void setup() {
    Serial.begin(115200);
    wifi.begin();
    wifi.connect("SSID", "PASSWORD");
    wifi.configNTP("pool.ntp.org", nullptr, nullptr, 8 * 3600, 0);
    wifi.syncTime();
}

void loop() {
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        WeatherData weather;
        if (wifi.getWeather(API_KEY, "Shanghai", weather)) {
            Serial.printf("[%s] ", wifi.getFormattedTime().c_str());
            Serial.printf("%s: %.1f°C, %s\n", 
                weather.city.c_str(), 
                weather.temperature, 
                weather.description.c_str());
        }
        lastUpdate = millis();
    }
}
```

## ⚠️ 注意事项

1. **WiFi频段**: ESP32 仅支持 2.4GHz WiFi, 不支持 5GHz。
2. **API密钥**: 使用天气功能需要注册 OpenWeatherMap 账号获取免费 API 密钥。
3. **内存限制**: JSON 解析时请注意缓冲区大小, 大响应需要使用 DynamicJsonDocument。
4. **超时设置**: 网络请求默认超时 10 秒, 可根据网络环境调整。
5. **时区设置**: NTP 时区偏移单位为秒, 东八区为 `8 * 3600 = 28800`。
6. **重连机制**: 建议定期检查 `isConnected()`, 断线后自动重连。
