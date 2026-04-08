#include "ESP32ADC.h"

// 构造函数
ESP32ADC::ESP32ADC(uint8_t pin, ADCAttenuation attenuation, ADCResolution resolution) {
  _pin = pin;
  _attenuation = attenuation;
  _resolution = resolution;
  _filterMode = FILTER_NONE;
  _filterSamples = 10;
  _emaAlpha = 0.3;
  _emaValue = 0;
  _offset = 0;
  _updateMaxValue();
}

void ESP32ADC::begin() {
  // 设置ADC分辨率
  analogReadResolution(_resolution);

  // 设置ADC衰减
  switch (_attenuation) {
    case ADC_ATTEN_0DB:
      analogSetAttenuation(ADC_0db);
      break;
    case ADC_ATTEN_2_5DB:
      analogSetAttenuation(ADC_2_5db);
      break;
    case ADC_ATTEN_6DB:
      analogSetAttenuation(ADC_6db);
      break;
    case ADC_ATTEN_11DB:
      analogSetAttenuation(ADC_11db);
      break;
  }

  // 初始化EMA值
  _emaValue = analogRead(_pin);
}

uint16_t ESP32ADC::readRaw() {
  return analogRead(_pin);
}

uint32_t ESP32ADC::readVoltage() {
  return _rawToVoltage(readRaw());
}

float ESP32ADC::readVoltageFloat() {
  return readVoltage() / 1000.0;
}

float ESP32ADC::readPercent() {
  return (float)readRaw() / _maxValue * 100.0;
}

float ESP32ADC::mapToRange(float outMin, float outMax) {
  uint16_t raw = readRaw();
  return map(raw, 0, _maxValue, outMin * 1000, outMax * 1000) / 1000.0;
}

void ESP32ADC::setFilterMode(ADCFilterMode mode, uint8_t samples, float alpha) {
  _filterMode = mode;
  _filterSamples = samples;
  if (alpha >= 0 && alpha <= 1) {
    _emaAlpha = alpha;
  }
}

uint16_t ESP32ADC::readFiltered() {
  switch (_filterMode) {
    case FILTER_AVERAGE:
      return _readAverage();
    case FILTER_MEDIAN:
      return _readMedian();
    case FILTER_EMA:
      return _readEMA();
    case FILTER_NONE:
    default:
      return readRaw();
  }
}

uint32_t ESP32ADC::readFilteredVoltage() {
  return _rawToVoltage(readFiltered());
}

void ESP32ADC::setAttenuation(ADCAttenuation attenuation) {
  _attenuation = attenuation;
  begin(); // 重新初始化以应用新设置
}

void ESP32ADC::setResolution(ADCResolution resolution) {
  _resolution = resolution;
  _updateMaxValue();
  begin(); // 重新初始化以应用新设置
}

uint16_t ESP32ADC::getMaxValue() {
  return _maxValue;
}

void ESP32ADC::calibrate() {
  // 简单的校准: 读取多次取平均作为偏移量基准
  uint32_t sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(_pin);
    delay(1);
  }
  uint16_t avg = sum / 100;
  _offset = 0 - avg; // 假设接地时应该为0
}

void ESP32ADC::setOffset(int16_t offset) {
  _offset = offset;
}

// 私有辅助函数
void ESP32ADC::_updateMaxValue() {
  _maxValue = (1 << _resolution) - 1;
}

uint32_t ESP32ADC::_rawToVoltage(uint16_t raw) {
  // 根据衰减计算电压
  // 11dB衰减时, 量程约0-3.3V, 对应0-4095
  // 计算公式: V = raw / max * Vref
  float vref = 3.3;
  switch (_attenuation) {
    case ADC_ATTEN_0DB:
      vref = 1.1;
      break;
    case ADC_ATTEN_2_5DB:
      vref = 1.5;
      break;
    case ADC_ATTEN_6DB:
      vref = 2.2;
      break;
    case ADC_ATTEN_11DB:
      vref = 3.3;
      break;
  }

  int32_t adjusted = (int32_t)raw + _offset;
  if (adjusted < 0) adjusted = 0;
  if (adjusted > _maxValue) adjusted = _maxValue;

  return (uint32_t)((adjusted / (float)_maxValue) * vref * 1000);
}

uint16_t ESP32ADC::_readAverage() {
  uint32_t sum = 0;
  for (int i = 0; i < _filterSamples; i++) {
    sum += analogRead(_pin);
    delayMicroseconds(100);
  }
  return sum / _filterSamples;
}

uint16_t ESP32ADC::_readMedian() {
  uint16_t samples[_filterSamples];
  for (int i = 0; i < _filterSamples; i++) {
    samples[i] = analogRead(_pin);
    delayMicroseconds(100);
  }

  // 冒泡排序
  for (int i = 0; i < _filterSamples - 1; i++) {
    for (int j = 0; j < _filterSamples - i - 1; j++) {
      if (samples[j] > samples[j + 1]) {
        uint16_t temp = samples[j];
        samples[j] = samples[j + 1];
        samples[j + 1] = temp;
      }
    }
  }

  return samples[_filterSamples / 2];
}

uint16_t ESP32ADC::_readEMA() {
  uint16_t raw = analogRead(_pin);
  _emaValue = _emaAlpha * raw + (1 - _emaAlpha) * _emaValue;
  return (uint16_t)_emaValue;
}
