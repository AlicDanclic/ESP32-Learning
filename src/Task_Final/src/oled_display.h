#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <U8g2lib.h>

// 状态枚举（新增 STATE_PLAY_GIF）
enum State {
  STATE_LOADING,
  STATE_E_CENTER,
  STATE_E_MOVE,
  STATE_PLAY_GIF,   // 播放GIF动画
  STATE_MAIN
};

// 初始化 OLED 及相关变量
void initOLED();

// 更新状态机（进度、动画、模拟时间、GIF播放）
void updateStateMachine();

// 根据当前状态绘制屏幕内容（在 firstPage/nextPage 循环内调用）
void drawCurrentState();

// 获取 U8g2 对象引用（用于 main 中的页面循环）
U8G2_SSD1306_128X64_NONAME_F_SW_I2C& getU8g2();

#endif