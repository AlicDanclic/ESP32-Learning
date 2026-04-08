# ArduinoJson 库使用指南

## 简介

ArduinoJson 是一个为 Arduino 和 ESP32 等嵌入式平台设计的轻量级 JSON 解析与生成库。它具有以下特点：

- **高效**：专为资源受限的微控制器优化，内存占用极小。
- **易用**：类似 JavaScript 的对象访问语法，直观便捷。
- **稳定**：经过大量项目验证，文档完善，社区活跃。

支持从字符串、流（如 `WiFiClient`）解析 JSON，也能轻松生成 JSON 数据。当前稳定版本为 **V7**，本指南基于 V7 编写。

---

## 安装

### PlatformIO

在 `platformio.ini` 中添加依赖：

```ini
lib_deps = 
    bblanchon/ArduinoJson @ ^7.0.0
```

### Arduino IDE

在库管理器中搜索 `ArduinoJson` 并安装（作者 bblanchon）。

---

## 快速开始

### 1. 包含头文件

```cpp
#include <ArduinoJson.h>
```

### 2. 解析 JSON 字符串

```cpp
// JSON 数据
const char* json = "{\"sensor\":\"DHT22\",\"temperature\":23.5}";

// 创建文档对象（大小根据 JSON 内容估算）
StaticJsonDocument<128> doc;

// 解析
DeserializationError error = deserializeJson(doc, json);
if (error) {
  Serial.print("解析失败: ");
  Serial.println(error.c_str());
  return;
}

// 提取数据
const char* sensor = doc["sensor"];
float temperature = doc["temperature"];

Serial.print("传感器: ");
Serial.println(sensor);
Serial.print("温度: ");
Serial.println(temperature);
```

### 3. 生成 JSON 字符串

```cpp
StaticJsonDocument<128> doc;

doc["name"] = "ESP32";
doc["value"] = 42;

String output;
serializeJson(doc, output);
Serial.println(output);   // 输出: {"name":"ESP32","value":42}
```

---

## 常用函数详解

### 文档对象

| 类型 | 说明 |
|------|------|
| `StaticJsonDocument<N>` | 栈上分配的固定大小文档（推荐，无内存碎片） |
| `DynamicJsonDocument` | 堆上分配的可变大小文档（V7 中已弃用，统一使用 `JsonDocument`） |
| `JsonDocument` | V7 中统一类型，可自动选择分配方式（通过 `JsonDocument::to<...>()`） |

> **V7 变化**：V7 中不再区分 `StaticJsonDocument` 和 `DynamicJsonDocument`，统一使用 `JsonDocument`，并可通过 `doc.to<JsonArray>()` 等指定类型。

### 解析与序列化

| 函数 | 说明 |
|------|------|
| `deserializeJson(doc, input)` | 解析 JSON，`input` 可为 `String`、`char*`、`Stream` 等，返回 `DeserializationError` |
| `serializeJson(doc, output)` | 将文档序列化为 JSON 字符串，`output` 可为 `String`、`Print` 对象等 |
| `serializeJsonPretty(doc, output)` | 格式化输出（带缩进） |

### 访问数据

| 操作 | 说明 |
|------|------|
| `doc["key"]` | 获取键对应的值，返回 `JsonVariant` |
| `doc["key"].as<T>()` | 将值转换为指定类型 `T`（如 `as<float>()`） |
| `doc["key"] \| defaultValue` | 若键不存在，返回默认值（V7 新增） |
| `doc.size()` | 获取对象或数组的元素个数 |
| `doc.containsKey("key")` | 检查对象中是否包含某个键 |

### 数组操作

```cpp
JsonArray arr = doc.to<JsonArray>();
arr.add(10);
arr.add(20);
int first = arr[0];          // 10

for (JsonVariant v : arr) {
  Serial.println(v.as<int>());
}
```

### 对象操作

```cpp
JsonObject obj = doc.to<JsonObject>();
obj["x"] = 1;
obj["y"] = 2;
int x = obj["x"];            // 1

for (JsonPair kv : obj) {
  Serial.print(kv.key().c_str());
  Serial.print(": ");
  Serial.println(kv.value().as<int>());
}
```

---

## 使用示例

### 1. 从串口解析 JSON

```cpp
void setup() {
  Serial.begin(115200);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, input);
    if (!error) {
      float temp = doc["temp"];
      Serial.print("温度: ");
      Serial.println(temp);
    }
  }
}
```

### 2. 从 WiFiClient 解析（HTTP 响应）

```cpp
WiFiClient client;
// ... 建立连接并发送 HTTP 请求 ...

JsonDocument doc;
DeserializationError error = deserializeJson(doc, client);
if (!error) {
  const char* status = doc["status"];
  // ...
}
```

### 3. 处理嵌套对象

```json
{
  "device": {
    "id": 123,
    "name": "sensor"
  },
  "values": [10, 20, 30]
}
```

```cpp
int id = doc["device"]["id"];                    // 123
const char* name = doc["device"]["name"];        // "sensor"
JsonArray arr = doc["values"];
for (int v : arr) {
  Serial.println(v);
}
```

### 4. 处理数组对象

```json
{
  "data": [
    {"x": 1, "y": 2},
    {"x": 3, "y": 4}
  ]
}
```

```cpp
JsonArray data = doc["data"];
for (JsonObject obj : data) {
  int x = obj["x"];
  int y = obj["y"];
  Serial.printf("(%d, %d)\n", x, y);
}
```

### 5. 生成 JSON 并发送到串口

```cpp
JsonDocument doc;
doc["time"] = millis();
doc["sensor"] = "DHT22";
doc["temperature"] = 23.5;

serializeJson(doc, Serial);
Serial.println();   // 换行
```

### 6. 处理大 JSON（流式解析）

当 JSON 数据很大时，使用 `deserializeJson` 直接传入 `Stream` 对象（如 `WiFiClient`），避免先读入大字符串。

```cpp
// 假设 client 是 WiFiClient，已经连接到服务器
DeserializationError error = deserializeJson(doc, client);
if (error) {
  // 处理错误
}
```

### 7. 错误处理与内存估算

使用 `DeserializationError` 检查解析结果：

```cpp
DeserializationError error = deserializeJson(doc, input);
switch (error.code()) {
  case DeserializationError::Ok:
    // 成功
    break;
  case DeserializationError::InvalidInput:
    Serial.println("输入不是有效 JSON");
    break;
  case DeserializationError::NoMemory:
    Serial.println("文档内存不足");
    break;
  // ... 其他错误类型
}
```

**内存估算**：使用 [ArduinoJson 助手](https://arduinojson.org/v7/assistant/) 输入 JSON 结构，获得推荐的文档大小。

---

## 注意事项

- **内存管理**：V7 中 `JsonDocument` 自动选择分配方式，但需要预估最大内存。过大或过小都会导致问题。
- **字符串拷贝**：默认情况下，解析后的字符串是原 JSON 的引用，如果原数据被修改或释放，会导致悬空指针。如需独立副本，使用 `.as<String>()` 或 `strdup()`。
- **浮点数精度**：JSON 中的浮点数按双精度存储，但打印时可能丢失精度，建议使用 `serializeJson(doc, Serial)` 直接输出。
- **性能**：`deserializeJson` 对于流式输入（如 `WiFiClient`）效率最高，避免使用 `String` 拼接。
- **V6 与 V7 差异**：V7 统一了文档类型，不再区分 `StaticJsonDocument` 和 `DynamicJsonDocument`，但 API 基本兼容。旧项目如需升级，可参考[官方迁移指南](https://arduinojson.org/v7/how-to/upgrade-from-v6-to-v7/)。

---

## 更多资源

- [ArduinoJson 官方文档](https://arduinojson.org/)（详细教程、API 参考）
- [在线助手](https://arduinojson.org/v7/assistant/)（计算文档大小）
- [GitHub 仓库](https://github.com/bblanchon/ArduinoJson)
- [常见问题解答](https://arduinojson.org/v7/faq/)

---

通过 ArduinoJson，您可以轻松地在 ESP32 等设备上处理 JSON 数据，无论是解析传感器上报、配置信息，还是生成 API 响应，都能高效完成。