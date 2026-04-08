#ifndef ESP32_DAC_H
#define ESP32_DAC_H

#include <Arduino.h>

// DAC通道枚举
enum DACChannel {
  DAC_CHANNEL_1 = 0, // GPIO25
  DAC_CHANNEL_2 = 1  // GPIO26
};

// 波形类型枚举
enum WaveformType {
  WAVE_DC = 0,      // 直流
  WAVE_SINE = 1,    // 正弦波
  WAVE_TRIANGLE = 2, // 三角波
  WAVE_SAWTOOTH = 3, // 锯齿波
  WAVE_SQUARE = 4   // 方波
};

class ESP32DAC {
public:
  /**
   * @brief 构造函数
   * @param channel DAC通道 (DAC_CHANNEL_1=GPIO25, DAC_CHANNEL_2=GPIO26)
   */
  ESP32DAC(DACChannel channel);

  /**
   * @brief 初始化DAC
   */
  void begin();

  /**
   * @brief 设置输出电压 (原始值)
   * @param value 输出值 (0-255, 8位分辨率)
   */
  void setValue(uint8_t value);

  /**
   * @brief 设置输出电压 (百分比)
   * @param percent 百分比 (0.0 - 100.0)
   */
  void setPercent(float percent);

  /**
   * @brief 设置输出电压 (毫伏)
   * @param voltage 电压值 (0-3300mV)
   */
  void setVoltage(uint16_t voltage);

  /**
   * @brief 设置输出电压 (伏特)
   * @param voltage 电压值 (0.0 - 3.3V)
   */
  void setVoltageFloat(float voltage);

  /**
   * @brief 开始生成波形
   * @param type 波形类型
   * @param frequency 频率 (Hz)
   * @param amplitude 幅度 (0-100%)
   * @param offset 直流偏移 (0-100%)
   */
  void startWaveform(WaveformType type, float frequency, float amplitude = 100.0, float offset = 50.0);

  /**
   * @brief 停止波形生成
   */
  void stopWaveform();

  /**
   * @brief 波形生成处理函数, 需要在主循环中调用
   */
  void loop();

  /**
   * @brief 设置波形参数
   * @param frequency 频率 (Hz)
   * @param amplitude 幅度 (0-100%)
   * @param offset 直流偏移 (0-100%)
   */
  void setWaveformParams(float frequency, float amplitude, float offset);

  /**
   * @brief 获取当前输出值
   * @return 当前输出值 (0-255)
   */
  uint8_t getCurrentValue();

  /**
   * @brief 关闭DAC输出
   */
  void off();

private:
  DACChannel _channel;
  uint8_t _pin;
  uint8_t _currentValue;

  // 波形生成相关
  WaveformType _waveType;
  float _waveFrequency;
  float _waveAmplitude;
  float _waveOffset;
  bool _waveRunning;
  unsigned long _waveStartTime;

  // 内部辅助函数
  void _writeDAC(uint8_t value);
  uint8_t _calculateSine(float phase);
  uint8_t _calculateTriangle(float phase);
  uint8_t _calculateSawtooth(float phase);
  uint8_t _calculateSquare(float phase);
};

#endif
