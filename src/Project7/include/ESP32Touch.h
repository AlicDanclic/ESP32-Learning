#ifndef ESP32_TOUCH_H
#define ESP32_TOUCH_H

#include <Arduino.h>
#include <driver/touch_pad.h>

// 触摸事件枚举
typedef enum {
  TOUCH_EVENT_NONE = 0,
  TOUCH_EVENT_PRESS,    // 触摸按下
  TOUCH_EVENT_RELEASE,  // 触摸释放
  TOUCH_EVENT_HOLD      // 持续触摸
} TouchEvent;

class ESP32Touch {
public:
  /**
   * @brief 构造函数
   * @param pin 触摸引脚 (仅支持 T0-T9: GPIO4,0,2,15,13,12,14,27,33,32)
   * @param threshold 触摸阈值 (低于此值视为触摸)
   */
  ESP32Touch(uint8_t pin, uint16_t threshold = 40);

  /**
   * @brief 初始化触摸功能
   */
  void begin();

  /**
   * @brief 核心处理函数, 需要在主循环中不断调用
   * @return TouchEvent 当前触发的事件
   */
  TouchEvent loop();

  /**
   * @brief 获取原始触摸值
   * @return 原始触摸值 (值越小表示触摸越强)
   */
  uint16_t readRaw();

  /**
   * @brief 检查当前是否被触摸
   * @return true=被触摸, false=未触摸
   */
  bool isTouched();

  /**
   * @brief 获取触摸状态 (带消抖)
   * @return true=被触摸, false=未触摸
   */
  bool getState();

  /**
   * @brief 设置触摸阈值
   * @param threshold 阈值 (值越小越敏感)
   */
  void setThreshold(uint16_t threshold);

  /**
   * @brief 获取当前阈值
   * @return 当前阈值
   */
  uint16_t getThreshold();

  /**
   * @brief 自动校准阈值 (建议在无触摸时调用)
   * @param sensitivity 灵敏度系数 (0-1, 默认0.8, 越小越敏感)
   */
  void calibrate(float sensitivity = 0.8);

  /**
   * @brief 设置消抖时间
   * @param ms 消抖时间 (毫秒, 默认50ms)
   */
  void setDebounceTime(unsigned long ms);

  /**
   * @brief 设置长按判定时间
   * @param ms 长按时间 (毫秒, 默认500ms)
   */
  void setHoldTime(unsigned long ms);

  /**
   * @brief 获取基准值 (无触摸时的参考值)
   * @return 基准值
   */
  uint16_t getBaseline();

private:
  uint8_t _pin;
  uint16_t _threshold;
  uint16_t _baseline;
  unsigned long _debounceTime;
  unsigned long _holdTime;
  unsigned long _lastChangeTime;
  unsigned long _touchStartTime;
  bool _lastState;
  bool _currentState;
  bool _holdTriggered;

  // 将GPIO映射到触摸通道
  touch_pad_t _gpioToTouchChannel(uint8_t pin);
};

#endif
