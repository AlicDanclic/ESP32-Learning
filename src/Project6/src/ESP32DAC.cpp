#include "ESP32DAC.h"
#include <math.h>

// 构造函数
ESP32DAC::ESP32DAC(DACChannel channel) {
  _channel = channel;
  _pin = (channel == DAC_CHANNEL_1) ? 25 : 26;
  _currentValue = 0;
  _waveType = WAVE_DC;
  _waveFrequency = 1.0;
  _waveAmplitude = 100.0;
  _waveOffset = 50.0;
  _waveRunning = false;
  _waveStartTime = 0;
}

void ESP32DAC::begin() {
  // DAC引脚不需要特殊初始化, analogWrite会自动配置
  // 但我们可以先输出0
  setValue(0);
}

void ESP32DAC::setValue(uint8_t value) {
  _currentValue = value;
  _writeDAC(value);
}

void ESP32DAC::setPercent(float percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  uint8_t value = (uint8_t)((percent / 100.0) * 255);
  setValue(value);
}

void ESP32DAC::setVoltage(uint16_t voltage) {
  if (voltage > 3300) voltage = 3300;

  float percent = (voltage / 3300.0) * 100.0;
  setPercent(percent);
}

void ESP32DAC::setVoltageFloat(float voltage) {
  if (voltage < 0) voltage = 0;
  if (voltage > 3.3) voltage = 3.3;

  setVoltage((uint16_t)(voltage * 1000));
}

void ESP32DAC::startWaveform(WaveformType type, float frequency, float amplitude, float offset) {
  _waveType = type;
  _waveFrequency = frequency;
  _waveAmplitude = amplitude;
  _waveOffset = offset;
  _waveRunning = true;
  _waveStartTime = millis();
}

void ESP32DAC::stopWaveform() {
  _waveRunning = false;
}

void ESP32DAC::loop() {
  if (!_waveRunning) return;

  unsigned long now = millis();
  float elapsed = (now - _waveStartTime) / 1000.0; // 转换为秒

  // 计算当前相位 (0-1)
  float phase = fmod(elapsed * _waveFrequency, 1.0);

  uint8_t value = 0;
  switch (_waveType) {
    case WAVE_SINE:
      value = _calculateSine(phase);
      break;
    case WAVE_TRIANGLE:
      value = _calculateTriangle(phase);
      break;
    case WAVE_SAWTOOTH:
      value = _calculateSawtooth(phase);
      break;
    case WAVE_SQUARE:
      value = _calculateSquare(phase);
      break;
    case WAVE_DC:
    default:
      value = (uint8_t)((_waveOffset / 100.0) * 255);
      break;
  }

  _writeDAC(value);
}

void ESP32DAC::setWaveformParams(float frequency, float amplitude, float offset) {
  _waveFrequency = frequency;
  _waveAmplitude = amplitude;
  _waveOffset = offset;
}

uint8_t ESP32DAC::getCurrentValue() {
  return _currentValue;
}

void ESP32DAC::off() {
  stopWaveform();
  setValue(0);
}

// 私有辅助函数
void ESP32DAC::_writeDAC(uint8_t value) {
  _currentValue = value;
  dacWrite(_pin, value);
}

uint8_t ESP32DAC::_calculateSine(float phase) {
  // 正弦波: sin(2*PI*phase), 范围 -1 到 1
  float sine = sin(2 * PI * phase);
  // 映射到 0-1 范围
  float normalized = (sine + 1) / 2;
  // 应用幅度和偏移
  float output = _waveOffset / 100.0 + (normalized - 0.5) * (_waveAmplitude / 100.0);
  // 限制范围
  if (output < 0) output = 0;
  if (output > 1) output = 1;

  return (uint8_t)(output * 255);
}

uint8_t ESP32DAC::_calculateTriangle(float phase) {
  // 三角波: 先上升后下降
  float triangle;
  if (phase < 0.5) {
    triangle = phase * 2; // 0 -> 1
  } else {
    triangle = 2 - phase * 2; // 1 -> 0
  }
  // 应用幅度和偏移
  float output = _waveOffset / 100.0 + (triangle - 0.5) * (_waveAmplitude / 100.0);
  if (output < 0) output = 0;
  if (output > 1) output = 1;

  return (uint8_t)(output * 255);
}

uint8_t ESP32DAC::_calculateSawtooth(float phase) {
  // 锯齿波: 线性上升, 然后跳变
  // 应用幅度和偏移
  float output = _waveOffset / 100.0 + (phase - 0.5) * (_waveAmplitude / 100.0);
  if (output < 0) output = 0;
  if (output > 1) output = 1;

  return (uint8_t)(output * 255);
}

uint8_t ESP32DAC::_calculateSquare(float phase) {
  // 方波: 50%占空比
  float square = (phase < 0.5) ? 1.0 : 0.0;
  // 应用幅度和偏移
  float output = _waveOffset / 100.0 + (square - 0.5) * (_waveAmplitude / 100.0);
  if (output < 0) output = 0;
  if (output > 1) output = 1;

  return (uint8_t)(output * 255);
}
