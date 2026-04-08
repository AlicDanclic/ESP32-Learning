#ifndef ESP32ADC_H
#define ESP32ADC_H

#include <Arduino.h>

// ADC衰减枚举
enum ADCAttenuation {
  ADC_ATTEN_0DB = 0,   // 0dB衰减, 量程约0-1.1V
  ADC_ATTEN_2_5DB = 1, // 2.5dB衰减, 量程约0-1.5V
  ADC_ATTEN_6DB = 2,   // 6dB衰减, 量程约0-2.2V
  ADC_ATTEN_11DB = 3   // 11dB衰减, 量程约0-3.3V (推荐)
};

// ADC分辨率枚举
enum ADCResolution {
  ADC_RES_9BIT = 9,   // 9位分辨率 (0-511)
  ADC_RES_10BIT = 10, // 10位分辨率 (0-1023)
  ADC_RES_11BIT = 11, // 11位分辨率 (0-2047)
  ADC_RES_12BIT = 12  // 12位分辨率 (0-4095) (默认)
};

// 滤波模式枚举
enum ADCFilterMode {
  FILTER_NONE = 0,    // 无滤波
  FILTER_AVERAGE = 1, // 平均值滤波
  FILTER_MEDIAN = 2,  // 中值滤波
  FILTER_EMA = 3      // 指数移动平均滤波
};

class ESP32ADC {
public:
  /**
   * @brief 构造函数
   * @param pin ADC输入引脚 (GPIO)
   * @param attenuation ADC衰减模式 (默认11dB, 量程0-3.3V)
   * @param resolution ADC分辨率 (默认12位)
   */
  ESP32ADC(uint8_t pin, ADCAttenuation attenuation = ADC_ATTEN_11DB, ADCResolution resolution = ADC_RES_12BIT);

  /**
   * @brief 初始化ADC
   */
  void begin();

  /**
   * @brief 读取原始ADC值
   * @return 原始ADC值 (0-4095, 取决于分辨率)
   */
  uint16_t readRaw();

  /**
   * @brief 读取电压值 (毫伏)
   * @return 电压值 (mV)
   */
  uint32_t readVoltage();

  /**
   * @brief 读取电压值 (伏特)
   * @return 电压值 (V)
   */
  float readVoltageFloat();

  /**
   * @brief 读取百分比 (0-100%)
   * @return 百分比值
   */
  float readPercent();

  /**
   * @brief 将ADC值映射到指定范围
   * @param outMin 输出范围最小值
   * @param outMax 输出范围最大值
   * @return 映射后的值
   */
  float mapToRange(float outMin, float outMax);

  /**
   * @brief 设置滤波模式
   * @param mode 滤波模式
   * @param samples 采样次数 (用于平均值滤波)
   * @param alpha EMA系数 (0-1, 用于EMA滤波)
   */
  void setFilterMode(ADCFilterMode mode, uint8_t samples = 10, float alpha = 0.3);

  /**
   * @brief 读取带滤波的ADC值
   * @return 滤波后的ADC值
   */
  uint16_t readFiltered();

  /**
   * @brief 读取带滤波的电压值
   * @return 滤波后的电压值 (mV)
   */
  uint32_t readFilteredVoltage();

  /**
   * @brief 设置衰减模式
   * @param attenuation 衰减模式
   */
  void setAttenuation(ADCAttenuation attenuation);

  /**
   * @brief 设置分辨率
   * @param resolution 分辨率
   */
  void setResolution(ADCResolution resolution);

  /**
   * @brief 获取ADC最大值 (基于当前分辨率)
   * @return 最大值
   */
  uint16_t getMaxValue();

  /**
   * @brief 校准ADC (使用内部参考电压)
   */
  void calibrate();

  /**
   * @brief 设置校准偏移量
   * @param offset 偏移量
   */
  void setOffset(int16_t offset);

private:
  uint8_t _pin;
  ADCAttenuation _attenuation;
  ADCResolution _resolution;
  ADCFilterMode _filterMode;
  uint8_t _filterSamples;
  float _emaAlpha;
  float _emaValue;
  int16_t _offset;
  uint16_t _maxValue;

  // 内部辅助函数
  uint16_t _readAverage();
  uint16_t _readMedian();
  uint16_t _readEMA();
  void _updateMaxValue();
  uint32_t _rawToVoltage(uint16_t raw);
};

#endif
