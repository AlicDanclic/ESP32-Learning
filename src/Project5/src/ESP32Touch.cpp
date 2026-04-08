#include "ESP32Touch.h"

// 构造函数
ESP32Touch::ESP32Touch(uint8_t pin, uint16_t threshold) {
  _pin = pin;
  _threshold = threshold;
  _baseline = 0;
  _debounceTime = 50;
  _holdTime = 500;
  _lastChangeTime = 0;
  _touchStartTime = 0;
  _lastState = false;
  _currentState = false;
  _holdTriggered = false;
}

void ESP32Touch::begin() {
  // 初始化触摸模块
  touch_pad_init();

  // 配置触摸通道
  touch_pad_t channel = _gpioToTouchChannel(_pin);
  if (channel != TOUCH_PAD_MAX) {
    touch_pad_config(channel, _threshold);
  }

  // 自动校准基准值
  calibrate(0.8);
}

TouchEvent ESP32Touch::loop() {
  unsigned long now = millis();
  bool rawState = isTouched();
  TouchEvent event = TOUCH_EVENT_NONE;

  // 消抖处理
  if (rawState != _lastState) {
    _lastChangeTime = now;
    _lastState = rawState;
  }

  // 状态稳定后处理
  if ((now - _lastChangeTime) > _debounceTime) {
    if (rawState != _currentState) {
      _currentState = rawState;

      if (_currentState) {
        // 触摸按下
        _touchStartTime = now;
        _holdTriggered = false;
        event = TOUCH_EVENT_PRESS;
      } else {
        // 触摸释放
        event = TOUCH_EVENT_RELEASE;
      }
    }
  }

  // 长按检测
  if (_currentState && !_holdTriggered) {
    if ((now - _touchStartTime) > _holdTime) {
      event = TOUCH_EVENT_HOLD;
      _holdTriggered = true;
    }
  }

  return event;
}

uint16_t ESP32Touch::readRaw() {
  touch_pad_t channel = _gpioToTouchChannel(_pin);
  if (channel != TOUCH_PAD_MAX) {
    return touchRead(_pin);
  }
  return 0;
}

bool ESP32Touch::isTouched() {
  uint16_t value = readRaw();
  // 值小于阈值表示被触摸
  return (value < _threshold);
}

bool ESP32Touch::getState() {
  return _currentState;
}

void ESP32Touch::setThreshold(uint16_t threshold) {
  _threshold = threshold;
  touch_pad_t channel = _gpioToTouchChannel(_pin);
  if (channel != TOUCH_PAD_MAX) {
    touch_pad_config(channel, _threshold);
  }
}

uint16_t ESP32Touch::getThreshold() {
  return _threshold;
}

void ESP32Touch::calibrate(float sensitivity) {
  // 读取多次取平均作为基准值
  uint32_t sum = 0;
  const int samples = 50;

  for (int i = 0; i < samples; i++) {
    sum += readRaw();
    delay(10);
  }

  _baseline = sum / samples;
  // 根据灵敏度设置阈值
  // 基准值 * (1 - 灵敏度) 作为阈值
  _threshold = (uint16_t)(_baseline * (1.0 - sensitivity));

  // 应用新阈值
  touch_pad_t channel = _gpioToTouchChannel(_pin);
  if (channel != TOUCH_PAD_MAX) {
    touch_pad_config(channel, _threshold);
  }
}

void ESP32Touch::setDebounceTime(unsigned long ms) {
  _debounceTime = ms;
}

void ESP32Touch::setHoldTime(unsigned long ms) {
  _holdTime = ms;
}

uint16_t ESP32Touch::getBaseline() {
  return _baseline;
}

// 私有辅助函数: GPIO到触摸通道映射
touch_pad_t ESP32Touch::_gpioToTouchChannel(uint8_t pin) {
  switch (pin) {
    case 4:  return TOUCH_PAD_NUM0;  // T0
    case 0:  return TOUCH_PAD_NUM1;  // T1
    case 2:  return TOUCH_PAD_NUM2;  // T2
    case 15: return TOUCH_PAD_NUM3;  // T3
    case 13: return TOUCH_PAD_NUM4;  // T4
    case 12: return TOUCH_PAD_NUM5;  // T5
    case 14: return TOUCH_PAD_NUM6;  // T6
    case 27: return TOUCH_PAD_NUM7;  // T7
    case 33: return TOUCH_PAD_NUM8;  // T8
    case 32: return TOUCH_PAD_NUM9;  // T9
    default: return TOUCH_PAD_MAX;   // 无效引脚
  }
}
