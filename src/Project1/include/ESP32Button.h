#ifndef ESP32_BUTTON_H
#define ESP32_BUTTON_H

#include <Arduino.h>

// 定义按键事件枚举
enum ButtonEvent {
  EVENT_NONE = 0,
  EVENT_SHORT_PRESS,  // 短按/单击
  EVENT_DOUBLE_CLICK, // 双击
  EVENT_LONG_PRESS    // 长按
};

class ESP32Button {
  private:
    uint8_t _pin;
    bool _activeLow;           // 是否低电平触发 (INPUT_PULLUP 通常为 true)
    
    // 时间参数 (单位: ms)
    unsigned long _debounceTime;      // 消抖时间
    unsigned long _longPressTime;     // 长按判定时间
    unsigned long _doubleClickTime;   // 双击间隔判定时间

    // 状态机变量
    unsigned long _lastChangeTime;     // 上次电平跳变时间
    unsigned long _lastClickTime;      // 上次点击松开的时间
    bool _lastState;                   // 上次读取的物理电平
    bool _isPressed;                   // 当前稳定的按键逻辑状态
    bool _waitingForDoubleClick;       // 是否正在等待第二次点击
    bool _longPressTriggered;          // 是否已经触发过长按

  public:
    /**
     * @brief 构造函数
     * @param pin 按键连接的 GPIO 引脚
     * @param activeLow 是否低电平触发 (默认 true，适用于 INPUT_PULLUP)
     */
    ESP32Button(uint8_t pin, bool activeLow = true);

    /**
     * @brief 初始化引脚模式
     */
    void begin();

    /**
     * @brief 核心处理函数，需要在主循环中不断调用
     * @return ButtonEvent 当前触发的事件
     */
    ButtonEvent loop();

    /**
     * @brief 获取当前按键是否按下的实时状态（消抖后）
     */
    bool isPressed();

    // 参数设置方法
    void setDebounceTime(unsigned long ms);
    void setLongPressTime(unsigned long ms);
    void setDoubleClickTime(unsigned long ms);
};

#endif