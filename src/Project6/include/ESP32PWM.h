#ifndef ESP32PWM_H
#define ESP32PWM_H

#include <Arduino.h>

class ESP32PWM {
public:
  // 构造函数
  // pin: 输出引脚
  // channel: PWM通道 (0-15), ESP32 Core v3.0+ 自动管理，但在 v2.x 中必须指定
  // freq: 频率 (Hz), 默认 5000Hz
  // resolution: 分辨率 (bits), 默认 8位 (0-255)
  ESP32PWM(uint8_t pin, uint8_t channel = 0, double freq = 5000, uint8_t resolution = 8);

  // 初始化硬件
  void begin();

  // 设置频率
  void setFrequency(double freq);

  // 设置占空比 (原始数值)
  // 例如 8位分辨率: 0-255
  void setDuty(uint32_t duty);

  // 设置占空比 (百分比)
  // 0.0 - 100.0
  void setPercent(float percent);

  // 设置目标占空比并开始渐变 (非阻塞)
  // targetPercent: 目标百分比
  // durationMs: 渐变过程总耗时 (毫秒)
  void fadeTo(float targetPercent, unsigned long durationMs);

  // 必须在主循环中调用，用于处理渐变动画
  void loop();

  // 获取当前最大占空比数值 (基于分辨率)
  uint32_t getMaxDuty();

  // 检查当前是否正在进行渐变 (用于呼吸灯逻辑判断)
  bool isFading();

private:
  uint8_t _pin;
  uint8_t _channel;
  double _freq;
  uint8_t _resolution;
  uint32_t _maxDutyVal; // 当前分辨率下的最大值 (如 255)

  // 渐变控制变量
  bool _isFading;
  uint32_t _startDuty;
  uint32_t _currentDuty;
  uint32_t _targetDuty;
  unsigned long _fadeStartTime;
  unsigned long _fadeDuration;
  
  // 内部辅助函数：应用 PWM 值到硬件
  void _writeToHardware(uint32_t duty);
};

#endif