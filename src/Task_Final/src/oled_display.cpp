#include "oled_display.h"
#include <Arduino.h>

// 请确保将您的GIF数据头文件放在这里，并确认其中的宏定义与下面一致
#include "my_gif_gif.h"   // 包含 MY_GIF_WIDTH, MY_GIF_HEIGHT, MY_GIF_FRAME_COUNT,
                          // MY_GIF_DELAY_MS, MY_GIF_FRAME_OFFSET, MY_GIF_FRAMES_DATA
                          // 以及 MY_GIF_FRAME_BYTES（通常为 (128*64+7)/8 = 1024）

// ========== 全局对象 ==========
static U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SCL=*/22, /*SDA=*/21, /*reset=*/U8X8_PIN_NONE);

// ========== 状态机变量 ==========
static State currentState = STATE_LOADING;

// 进度条
static int progress = 0;
static unsigned long lastProgressUpdate = 0;
const unsigned long PROGRESS_INTERVAL = 10;   // 10ms 增加1%
const int MAX_PROGRESS = 30;

// 动画计时
static unsigned long stateEnterTime = 0;
const unsigned long E_CENTER_DURATION = 1000; // 中央E停留1秒
const unsigned long E_MOVE_DURATION = 800;    // 移动动画800ms

// 移动动画参数
static int eX = 64, eY = 32;
const int eTargetX = 20, eTargetY = 32;
static int eStartX, eStartY;
static unsigned long moveStartTime = 0;

// 字体
static const uint8_t* bigFont = u8g2_font_logisoso20_tf;
static const uint8_t* smallFont = u8g2_font_6x10_tf;

// 主界面数据
static const char* dateStr = "2025-04-02";
static char timeBuffer[9] = "14:30:00";
static const char* weatherStr = "Sunny  25C";
static unsigned long lastTimeUpdate = 0;

// ========== GIF 播放相关变量 ==========
static uint8_t gifFrameBuffer[MY_GIF_FRAME_BYTES];  // 存储当前要显示的帧
static int gifCurrentFrame = 0;                     // 当前帧索引
static unsigned long gifLastFrameTime = 0;          // 上一帧显示的时刻
static bool gifPlaying = false;                     // 是否正在播放

// ========== 内部函数 ==========
static void updateSimulatedTime() {
  static int hour = 14, minute = 30, second = 0;
  unsigned long now = millis();
  if (now - lastTimeUpdate >= 1000) {
    second++;
    if (second >= 60) {
      second = 0;
      minute++;
      if (minute >= 60) {
        minute = 0;
        hour++;
        if (hour >= 24) hour = 0;
      }
    }
    sprintf(timeBuffer, "%02d:%02d:%02d", hour, minute, second);
    lastTimeUpdate = now;
  }
}

static void drawLoading() {
  u8g2.setFont(smallFont);
  u8g2.setCursor(0, 20);
  u8g2.print("Loading...");

  u8g2.drawFrame(14, 35, 100, 12);
  int barWidth = map(progress, 0, MAX_PROGRESS, 0, 100);
  if (barWidth > 0) {
    u8g2.drawBox(15, 36, barWidth, 10);
  }
}

static void drawECenter() {
  u8g2.setFont(bigFont);
  int w = u8g2.getStrWidth("E");
  int x = (128 - w) / 2;
  int y = 32 + 10;   // 垂直居中
  u8g2.setCursor(x, y);
  u8g2.print("E");
}

static void drawEMove() {
  u8g2.setFont(bigFont);
  u8g2.setCursor(eX, eY + 10);
  u8g2.print("E");

  u8g2.setFont(smallFont);
  u8g2.setCursor(80, eY + 12);
  u8g2.print("ESP32");
}

static void drawMain() {
  u8g2.setFont(smallFont);
  u8g2.setCursor(0, 10);
  u8g2.print("Date: ");
  u8g2.print(dateStr);

  u8g2.setCursor(0, 25);
  u8g2.print("Time: ");
  u8g2.print(timeBuffer);

  u8g2.setCursor(0, 40);
  u8g2.print("Weather: ");
  u8g2.print(weatherStr);
}

// ========== GIF 播放核心函数 ==========
// 从GIF数据中加载指定帧到 gifFrameBuffer
static void loadGifFrame(int frameIndex) {
  uint32_t offset = MY_GIF_FRAME_OFFSET[frameIndex];
  const uint8_t* src = &MY_GIF_FRAMES_DATA[offset];
  memcpy(gifFrameBuffer, src, MY_GIF_FRAME_BYTES);
}

// 开始播放GIF
static void startGifPlayback() {
  gifPlaying = true;
  gifCurrentFrame = 0;
  loadGifFrame(0);                     // 预加载第一帧
  gifLastFrameTime = millis();
}

// 更新GIF播放（在状态机中调用，非阻塞）
static void updateGifPlayback() {
  if (!gifPlaying) return;

  unsigned long now = millis();
  // 判断是否需要切换到下一帧
  if (now - gifLastFrameTime >= MY_GIF_DELAY_MS[gifCurrentFrame]) {
    gifCurrentFrame++;
    if (gifCurrentFrame >= MY_GIF_FRAME_COUNT) {
      // 播放完成，切换到主界面
      gifPlaying = false;
      currentState = STATE_MAIN;
    } else {
      // 加载下一帧
      loadGifFrame(gifCurrentFrame);
      gifLastFrameTime = now;
    }
  }
}

// 绘制当前GIF帧（在 firstPage/nextPage 循环内调用）
static void drawGifFrame() {
  if (gifPlaying) {
    // 参数: (x, y, 每行字节数, 总行数, 数据指针)
    u8g2.drawBitmap(0, 0, MY_GIF_WIDTH / 8, MY_GIF_HEIGHT, gifFrameBuffer);
  }
}

// ========== 对外接口实现 ==========
void initOLED() {
  u8g2.begin();
  u8g2.setFontPosTop();
  u8g2.setBitmapMode(1);   // 白色显示模式

  progress = 0;
  lastProgressUpdate = millis();
  stateEnterTime = millis();
}

void updateStateMachine() {
  unsigned long now = millis();

  switch (currentState) {
    case STATE_LOADING:
      if (now - lastProgressUpdate >= PROGRESS_INTERVAL) {
        lastProgressUpdate = now;
        progress++;
        if (progress >= MAX_PROGRESS) {
          progress = MAX_PROGRESS;
          currentState = STATE_PLAY_GIF;
          stateEnterTime = now;
        }
      }
      break;

    case STATE_E_CENTER:
      if (now - stateEnterTime >= E_CENTER_DURATION) {
        currentState = STATE_E_MOVE;
        moveStartTime = now;
        eStartX = 64;
        eStartY = 32;
        eX = eStartX;
        eY = eStartY;
      }
      break;

    case STATE_E_MOVE:
      {
        float t = (float)(now - moveStartTime) / E_MOVE_DURATION;
        if (t >= 1.0f) {
          // 移动动画完成，开始播放GIF
          currentState = STATE_PLAY_GIF;
          startGifPlayback();
        } else {
          eX = eStartX + (int)((eTargetX - eStartX) * t);
          eY = eStartY + (int)((eTargetY - eStartY) * t);
        }
      }
      break;

    case STATE_PLAY_GIF:
      updateGifPlayback();   // 更新帧索引和缓冲区
      break;

    case STATE_MAIN:
      updateSimulatedTime();
      break;
  }
}

void drawCurrentState() {
  switch (currentState) {
    case STATE_LOADING:
      drawLoading();
      break;
    case STATE_E_CENTER:
      drawECenter();
      break;
    case STATE_E_MOVE:
      drawEMove();
      break;
    case STATE_PLAY_GIF:
      drawGifFrame();       // 绘制当前GIF帧
      break;
    case STATE_MAIN:
      drawMain();
      break;
  }
}

U8G2_SSD1306_128X64_NONAME_F_SW_I2C& getU8g2() {
  return u8g2;
}