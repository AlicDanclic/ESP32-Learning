# U8g2 图形库使用指南

## 简介

U8g2 是一个为单色 OLED/LCD 屏幕设计的强大图形库，支持多种屏幕驱动（如 SSD1306、SH1106 等）。相比 Adafruit 库，U8g2 提供了更丰富的字体选择（包括 6x8 等宽小字体）和更灵活的绘图功能。

## 安装

### PlatformIO

在 `platformio.ini` 中添加依赖：
```ini
lib_deps = 
    olikraus/U8g2@^2.35.15
```

### Arduino IDE

在库管理器中搜索 `U8g2` 并安装（作者 olikraus）。

---

## 快速开始

### 1. 包含头文件并创建对象
根据你的屏幕分辨率和接口选择正确的构造函数。

**常用构造函数（I2C接口）：**
| 屏幕尺寸 | 构造函数 |
|---------|----------|
| 128x64 | `U8G2_SSD1306_128X64_NONAME_F_SW_I2C` |
| 128x32 | `U8G2_SSD1306_128X32_NONAME_F_SW_I2C` |

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// 参数：旋转方向，SCL引脚，SDA引脚，复位引脚（U8X8_PIN_NONE表示不用）
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SCL*/ 22, /*SDA*/ 21, /*复位*/ U8X8_PIN_NONE);
```

### 2. 初始化
在 `setup()` 中：
```cpp
void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);   // 设置字体
  u8g2.setFontPosTop();              // 设置坐标从字符左上角开始
}
```

### 3. 显示内容
U8g2 采用双缓冲机制，显示必须放在 `firstPage()` / `nextPage()` 循环中：
```cpp
void loop() {
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print("Hello, ESP32!");
    u8g2.setCursor(0, 12);
    u8g2.print("U8g2 Library");
  } while (u8g2.nextPage());
  delay(1000);
}
```

---

## 常用函数详解

### 初始化与页面管理

| 函数 | 说明 |
|------|------|
| `u8g2.begin()` | 初始化屏幕，必须首先调用 |
| `u8g2.firstPage()` | 开始一帧的绘制，清空缓冲区并准备绘制 |
| `u8g2.nextPage()` | 结束当前页绘制并发送到屏幕，返回 `true` 表示还有下一页（一般循环内调用） |
| `u8g2.clearBuffer()` | 清空缓冲区（配合 `sendBuffer()` 使用） |
| `u8g2.sendBuffer()` | 将缓冲区内容发送到屏幕（配合 `clearBuffer()` 使用） |

> **标准用法**：`firstPage()`/`nextPage()` 循环是最简单可靠的方式，内部自动管理缓冲区，无需手动调用 `clearBuffer()` 和 `sendBuffer()`。

### 文本显示

| 函数 | 说明 |
|------|------|
| `u8g2.setFont(const uint8_t *font)` | 设置字体，参数为预定义字体变量 |
| `u8g2.setFontPosTop()` | 设置光标位置为字符左上角（默认是基线） |
| `u8g2.setFontPosCenter()` | 设置光标位置为字符垂直居中 |
| `u8g2.setCursor(x, y)` | 设置文本起始坐标 |
| `u8g2.print(str)` | 输出字符串，支持 `String`、`char*`、数字等 |
| `u8g2.println(str)` | 输出字符串并换行（换行需手动调整 y 坐标） |
| `u8g2.setFontDirection(uint8_t dir)` | 设置文字方向（0=正常，1=90度，2=180度，3=270度） |
| `u8g2.enableUTF8Print()` | 启用 UTF-8 编码支持（用于中文等） |

### 图形绘制

所有绘图函数在 `firstPage()`/`nextPage()` 循环内使用，或在 `clearBuffer()`/`sendBuffer()` 模式下使用。

| 函数 | 说明 |
|------|------|
| `u8g2.drawPixel(x, y)` | 画一个像素点 |
| `u8g2.drawLine(x0, y0, x1, y1)` | 画线段 |
| `u8g2.drawHLine(x, y, w)` | 画水平线（宽度 w） |
| `u8g2.drawVLine(x, y, h)` | 画垂直线（高度 h） |
| `u8g2.drawFrame(x, y, w, h)` | 画矩形边框 |
| `u8g2.drawBox(x, y, w, h)` | 画实心矩形 |
| `u8g2.drawRFrame(x, y, w, h, r)` | 画圆角矩形边框 |
| `u8g2.drawRBox(x, y, w, h, r)` | 画实心圆角矩形 |
| `u8g2.drawCircle(x, y, rad)` | 画圆（空心） |
| `u8g2.drawDisc(x, y, rad)` | 画实心圆 |
| `u8g2.drawEllipse(x, y, rx, ry)` | 画椭圆（空心） |
| `u8g2.drawFilledEllipse(x, y, rx, ry)` | 画实心椭圆 |
| `u8g2.drawTriangle(x0,y0, x1,y1, x2,y2)` | 画三角形（空心） |
| `u8g2.drawFilledTriangle(x0,y0, x1,y1, x2,y2)` | 画实心三角形 |

### 位图与图像

| 函数 | 说明 |
|------|------|
| `u8g2.drawXBMP(x, y, w, h, const uint8_t *bitmap)` | 绘制 XBM 格式位图（宽 w，高 h） |
| `u8g2.drawBitmap(x, y, cnt, h, const uint8_t *bitmap)` | 通用位图绘制，`cnt` 为字节数 |
| `u8g2.drawXBM(x, y, w, h, const uint8_t *bitmap)` | 旧版 XBM 绘制（已弃用） |

> **XBM 格式**：可以通过在线工具将图片转换为 C 数组，例如 [image2cpp](https://javl.github.io/image2cpp/)。

### 高级功能

| 函数 | 说明 |
|------|------|
| `u8g2.setDrawColor(color)` | 设置绘制颜色（1=前景色，0=背景色） |
| `u8g2.setBitmapMode(mode)` | 设置位图模式（0=透明，1=覆盖） |
| `u8g2.setContrast(value)` | 设置对比度（0-255，部分屏幕支持） |
| `u8g2.setPowerSave(mode)` | 设置省电模式（1=关屏，0=开屏） |
| `u8g2.getWidth()` | 获取屏幕宽度（像素） |
| `u8g2.getHeight()` | 获取屏幕高度（像素） |

---

## 如何使用：从简单到复杂

### 1. 显示静态文本

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontPosTop();
}

void loop() {
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print("Hello, ESP32!");
    u8g2.setCursor(0, 12);
    u8g2.print("U8g2 is great!");
  } while (u8g2.nextPage());
  delay(1000);
}
```

### 2. 显示变量值（动态刷新）

```cpp
int counter = 0;

void loop() {
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print("Count: ");
    u8g2.print(counter);
  } while (u8g2.nextPage());
  counter++;
  delay(100);
}
```

### 3. 绘制图形

```cpp
void loop() {
  u8g2.firstPage();
  do {
    u8g2.drawFrame(10, 10, 50, 30);        // 矩形边框
    u8g2.drawBox(70, 10, 40, 40);          // 实心正方形
    u8g2.drawCircle(100, 50, 10);          // 空心圆
    u8g2.drawDisc(30, 50, 12);             // 实心圆
    u8g2.drawLine(0, 0, 127, 63);          // 对角线
  } while (u8g2.nextPage());
  delay(2000);
}
```

### 4. 绘制进度条

```cpp
int progress = 0;   // 0-100

void drawProgressBar(int percent) {
  int barWidth = map(percent, 0, 100, 0, 100);  // 进度条宽度最大100像素
  u8g2.drawFrame(14, 30, 100, 12);              // 边框
  u8g2.drawBox(15, 31, barWidth, 10);           // 填充
}

void loop() {
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 20);
    u8g2.print("Progress:");
    drawProgressBar(progress);
    u8g2.setCursor(50, 50);
    u8g2.print(progress);
    u8g2.print("%");
  } while (u8g2.nextPage());
  
  progress = (progress + 1) % 101;
  delay(100);
}
```

### 5. 显示位图（XBM）

首先，用 [image2cpp](https://javl.github.io/image2cpp/) 将图片转换为 C 数组，例如一个 32x32 的心形：

```cpp
// 心形位图数据（32x32）
const uint8_t heart_bitmap[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  // ... 省略中间数据 ...
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void loop() {
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(48, 16, 32, 32, heart_bitmap);
  } while (u8g2.nextPage());
  delay(5000);
}
```

### 6. 多页面显示（翻页）

```cpp
int page = 0;

void showPage1() {
  u8g2.setCursor(0, 10);
  u8g2.print("Page 1");
  u8g2.setCursor(0, 30);
  u8g2.print("Press button");
}

void showPage2() {
  u8g2.setCursor(0, 10);
  u8g2.print("Page 2");
  u8g2.drawBox(10, 30, 50, 20);
}

void loop() {
  u8g2.firstPage();
  do {
    if (page == 0) showPage1();
    else showPage2();
  } while (u8g2.nextPage());
  
  // 模拟按键切换页面
  delay(2000);
  page = (page + 1) % 2;
}
```

### 7. 使用 UTF-8 显示中文

注意：中文字体较大，占用 Flash 较多，建议仅在使用时才包含。

```cpp
void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print();                      // 启用 UTF-8 支持
  u8g2.setFont(u8g2_font_wqy12_t_chinese3);    // 文泉驿 12px 中文字体
  u8g2.setFontPosTop();
}

void loop() {
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print("你好，世界！");
    u8g2.setCursor(0, 16);
    u8g2.print("U8g2 支持中文");
  } while (u8g2.nextPage());
  delay(3000);
}
```

### 8. 两种缓冲区模式对比

U8g2 支持两种工作模式：

- **页面循环模式**（推荐）：
  ```cpp
  u8g2.firstPage();
  do {
    // 所有绘图命令放在这里
  } while (u8g2.nextPage());
  ```
  每次 `nextPage()` 将当前缓冲区内容发送到屏幕，适用于大多数场景。

- **手动缓冲区模式**：
  ```cpp
  u8g2.clearBuffer();
  // 绘图命令
  u8g2.sendBuffer();
  ```
  这种模式允许你随时更新屏幕，适合需要频繁局部更新的场合（如动画），但需要自己管理缓冲区。

---

## 常用字体列表

U8g2 内置大量字体，命名规则：`u8g2_font_[宽]x[高]_tf`（tf = 全字符集，tr = 精简版，tn = 数字版）。

| 字体名 | 尺寸 (宽×高) | 说明 |
|--------|-------------|------|
| `u8g2_font_5x7_tf` | 5×7 | 极紧凑，类似 Arduino 默认 |
| `u8g2_font_6x10_tf` | 6×10 | 清晰，行距合理 |
| `u8g2_font_6x12_tf` | 6×12 | 更清晰 |
| `u8g2_font_7x13_tf` | 7×13 | 标准大小 |
| `u8g2_font_8x13_tf` | 8×13 | 常用中等字体 |
| `u8g2_font_10x20_tf` | 10×20 | 大字 |
| `u8g2_font_logisoso16_tf` | 可变宽度 | 艺术字体 |
| `u8g2_font_wqy12_t_chinese3` | 12px | 文泉驿 12px 中文字体 |

> **查看完整字体列表**：U8g2 官方 Wiki [字体列表](https://github.com/olikraus/u8g2/wiki/fntlistall)

---

## 完整示例：综合应用

下面是一个结合文本、图形、进度条和动态刷新的示例：

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);

int counter = 0;

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontPosTop();
}

void loop() {
  u8g2.firstPage();
  do {
    // 标题
    u8g2.setCursor(0, 0);
    u8g2.print("U8g2 Demo");
    u8g2.drawHLine(0, 10, 128);
    
    // 计数器
    u8g2.setCursor(0, 20);
    u8g2.print("Count: ");
    u8g2.print(counter);
    
    // 进度条
    int percent = counter % 101;
    int barWidth = map(percent, 0, 100, 0, 100);
    u8g2.drawFrame(14, 35, 100, 12);
    u8g2.drawBox(15, 36, barWidth, 10);
    u8g2.setCursor(60, 52);
    u8g2.print(percent);
    u8g2.print("%");
    
    // 画一个圆
    u8g2.drawCircle(110, 55, 8);
  } while (u8g2.nextPage());
  
  counter++;
  delay(100);
}
```

---

## 更多资源

- [U8g2 官方 GitHub](https://github.com/olikraus/u8g2)
- [U8g2 完整函数列表](https://github.com/olikraus/u8g2/wiki/u8g2reference)
- [字体列表与示例](https://github.com/olikraus/u8g2/wiki/fntlistall)
- [在线图片转 XBM 工具](https://javl.github.io/image2cpp/)