#include "ESP32Button.h"

// 构造函数
ESP32Button::ESP32Button(uint8_t pin, bool activeLow) {
  _pin = pin;
  _activeLow = activeLow;
  
  // 初始化默认参数
  _debounceTime = 50;
  _longPressTime = 1000;
  _doubleClickTime = 300;
  
  // 初始化状态变量
  _lastChangeTime = 0;
  _lastClickTime = 0;
  _isPressed = false;
  _waitingForDoubleClick = false;
  _longPressTriggered = false;
  _lastState = _activeLow ? HIGH : LOW; // 假设初始状态为未按下
}

void ESP32Button::begin() {
  if (_activeLow) {
    pinMode(_pin, INPUT_PULLUP);
    _lastState = HIGH;
  } else {
    pinMode(_pin, INPUT_PULLDOWN);
    _lastState = LOW;
  }
}

void ESP32Button::setDebounceTime(unsigned long ms) {
  _debounceTime = ms;
}

void ESP32Button::setLongPressTime(unsigned long ms) {
  _longPressTime = ms;
}

void ESP32Button::setDoubleClickTime(unsigned long ms) {
  _doubleClickTime = ms;
}

bool ESP32Button::isPressed() {
  return _isPressed;
}

ButtonEvent ESP32Button::loop() {
  int currentState = digitalRead(_pin);
  unsigned long now = millis();
  ButtonEvent event = EVENT_NONE;

  // 1. 处理物理信号消抖
  if (currentState != _lastState) {
    _lastChangeTime = now;
    _lastState = currentState;
  }

  // 如果电平稳定时间超过消抖时间
  if ((now - _lastChangeTime) > _debounceTime) {
    // 计算逻辑状态 (activeLow时, LOW代表按下true)
    bool logicalState = _activeLow ? (currentState == LOW) : (currentState == HIGH);

    // 如果状态发生了改变
    if (logicalState != _isPressed) {
      _isPressed = logicalState;

      if (_isPressed) {
        // ---> 按下时刻 <---
        _longPressTriggered = false; // 重置长按标志
        
        // 如果正在等待双击，且这次按下距离上次松开很近 -> 可能是双击的第二次按下
        if (_waitingForDoubleClick && (now - _lastClickTime <= _doubleClickTime)) {
           // 这里不做操作，等松开时确认双击
        } else {
          // 第一次按下，或者超时了，视为新的开始
          _waitingForDoubleClick = false; 
        }
      } 
      else {
        // ---> 松开时刻 <---
        if (!_longPressTriggered) {
          // 如果没有触发过长按，才检测短按/双击逻辑
          
          if (_waitingForDoubleClick) {
            // 如果这是第二次松开，且时间符合 -> 触发双击
            event = EVENT_DOUBLE_CLICK;
            _waitingForDoubleClick = false; // 双击完成，重置
          } else {
            // 这是第一次松开，标记为等待双击，并记录时间
            _waitingForDoubleClick = true;
            _lastClickTime = now;
          }
        }
      }
    }
  }

  // 2. 处理长按逻辑 (按住期间)
  if (_isPressed && !_longPressTriggered) {
    // 如果按住的时间超过设定值
    if (now - _lastChangeTime > _longPressTime) {
      event = EVENT_LONG_PRESS;
      _longPressTriggered = true; // 锁定，防止一直触发
      _waitingForDoubleClick = false; // 长按后不可能再接双击
    }
  }

  // 3. 处理单击延迟判定
  // 如果处于等待双击状态，但时间已经超时了，说明没有第二次点击 -> 确认为单击
  if (_waitingForDoubleClick && !_isPressed && (now - _lastClickTime > _doubleClickTime)) {
    event = EVENT_SHORT_PRESS;
    _waitingForDoubleClick = false;
  }

  return event;
}