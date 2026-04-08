#include "ESP32PWM.h"

// 构造函数
ESP32PWM::ESP32PWM(uint8_t pin, uint8_t channel, double freq, uint8_t resolution) {
  _pin = pin;
  _channel = channel;
  _freq = freq;
  _resolution = resolution;
  
  // 计算当前分辨率下的最大值 (例如 8bit = 255)
  _maxDutyVal = (uint32_t)pow(2, _resolution) - 1;
  
  _currentDuty = 0;
  _isFading = false;
}

void ESP32PWM::begin() {
  // ESP32 Arduino Core 3.0.0 更改了 LEDC API
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  // v3.0+ 写法: 自动管理通道
  ledcAttach(_pin, _freq, _resolution);
#else
  // v2.x 写法: 手动绑定通道
  ledcSetup(_channel, _freq, _resolution);
  ledcAttachPin(_pin, _channel);
#endif

  // 初始设为 0
  setDuty(0);
}

void ESP32PWM::setFrequency(double freq) {
  _freq = freq;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcChangeFrequency(_pin, _freq, _resolution);
#else
  ledcSetup(_channel, _freq, _resolution);
#endif
}

void ESP32PWM::setDuty(uint32_t duty) {
  // 停止任何正在进行的渐变
  _isFading = false;
  
  // 限制范围
  if (duty > _maxDutyVal) duty = _maxDutyVal;
  
  _currentDuty = duty;
  _writeToHardware(_currentDuty);
}

void ESP32PWM::setPercent(float percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  uint32_t duty = (uint32_t)((percent / 100.0) * _maxDutyVal);
  setDuty(duty);
}

void ESP32PWM::fadeTo(float targetPercent, unsigned long durationMs) {
  if (targetPercent < 0) targetPercent = 0;
  if (targetPercent > 100) targetPercent = 100;

  _targetDuty = (uint32_t)((targetPercent / 100.0) * _maxDutyVal);
  
  // 如果当前已经在目标值，直接退出
  if (_targetDuty == _currentDuty) {
    _isFading = false;
    return;
  }

  _startDuty = _currentDuty;
  _fadeStartTime = millis();
  _fadeDuration = durationMs;
  _isFading = true;
}

void ESP32PWM::loop() {
  // 如果没有在渐变，直接返回
  if (!_isFading) return;

  unsigned long now = millis();
  unsigned long elapsed = now - _fadeStartTime;

  if (elapsed >= _fadeDuration) {
    // 渐变结束
    _currentDuty = _targetDuty;
    _writeToHardware(_currentDuty);
    _isFading = false;
  } else {
    // 计算当前步骤的 Duty (线性插值)
    // 公式: Start + (Target - Start) * (Elapsed / Total)
    float progress = (float)elapsed / (float)_fadeDuration;
    int32_t diff = (int32_t)_targetDuty - (int32_t)_startDuty;
    _currentDuty = _startDuty + (diff * progress);
    
    _writeToHardware(_currentDuty);
  }
}

bool ESP32PWM::isFading() {
  return _isFading;
}

void ESP32PWM::_writeToHardware(uint32_t duty) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(_pin, duty);
#else
  ledcWrite(_channel, duty);
#endif
}

uint32_t ESP32PWM::getMaxDuty() {
  return _maxDutyVal;
}