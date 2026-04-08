#include "PWMChannel.h"

PWMChannel::PWMChannel(uint8_t channel, uint8_t pin) 
    : _channel(channel), _pin(pin), _freq_hz(1000.0), 
      _resolution(8), _initialized(false), _max_duty(255) {
}

void PWMChannel::_calculateMaxDuty() {
    _max_duty = (1UL << _resolution) - 1;
}

bool PWMChannel::init(double freq_hz, uint8_t resolution_bits) {
    if (_channel > 15) {
        Serial.printf("错误: 通道号%d无效 (0-15)\n", _channel);
        return false;
    }
    
    if (freq_hz <= 0) {
        Serial.printf("错误: 频率%.2fHz无效\n", freq_hz);
        return false;
    }
    
    if (resolution_bits < 1 || resolution_bits > 16) {
        Serial.printf("错误: 分辨率%d位无效 (1-16)\n", resolution_bits);
        return false;
    }

    _freq_hz = freq_hz;
    _resolution = resolution_bits;
    _calculateMaxDuty();

    double actual_freq = ledcSetup(_channel, _freq_hz, _resolution);
    ledcAttachPin(_pin, _channel);
    
    _initialized = true;

    Serial.printf("[PWM通道%d] 初始化成功:\n", _channel);
    Serial.printf("  引脚: GPIO%d\n", _pin);
    Serial.printf("  频率: 请求%.2fHz, 实际%.2fHz\n", _freq_hz, actual_freq);
    Serial.printf("  分辨率: %d位 (占空比范围: 0-%lu)\n", _resolution, _max_duty);
    Serial.printf("  周期: %.2fμs\n", getPeriodUs());
    
    return true;
}

bool PWMChannel::writePercent(float percent) {
    if (!_initialized) {
        Serial.printf("错误: 通道%d未初始化，请先调用init()\n", _channel);
        return false;
    }

    percent = constrain(percent, 0.0, 100.0);
    uint32_t duty = static_cast<uint32_t>((percent / 100.0) * _max_duty);
    ledcWrite(_channel, duty);
    
    Serial.printf("[通道%d] 占空比: %.1f%% -> 值: %lu/%lu, 脉宽: %.2fμs\n",
                 _channel, percent, duty, _max_duty, getPulseWidthUs(percent));
    
    return true;
}

bool PWMChannel::writeMicroseconds(float pulse_width_us) {
    if (!_initialized) return false;
    
    float period_us = getPeriodUs();
    pulse_width_us = constrain(pulse_width_us, 0.0, period_us);
    float percent = (pulse_width_us / period_us) * 100.0;
    
    return writePercent(percent);
}

bool PWMChannel::writeDuty(uint32_t duty_value) {
    if (!_initialized) return false;
    
    duty_value = constrain(duty_value, 0UL, _max_duty);
    float percent = (duty_value * 100.0) / _max_duty;
    
    ledcWrite(_channel, duty_value);
    Serial.printf("[通道%d] 原始值: %lu -> 占空比: %.1f%%, 脉宽: %.2fμs\n",
                 _channel, duty_value, percent, getPulseWidthUs(percent));
    
    return true;
}

void PWMChannel::breathe(unsigned long duration_ms, int cycles) {
    if (!_initialized) return;
    
    for (int i = 0; i < cycles; i++) {
        for (float p = 0; p <= 100.0; p += 1.0) {
            writePercent(p);
            delay(duration_ms / 200);
        }
        for (float p = 100.0; p >= 0.0; p -= 1.0) {
            writePercent(p);
            delay(duration_ms / 200);
        }
    }
}

float PWMChannel::getPeriodUs() const {
    return 1000000.0 / _freq_hz;
}

float PWMChannel::getPulseWidthUs(float percent) const {
    return (percent / 100.0) * getPeriodUs();
}

uint32_t PWMChannel::getMaxDuty() const {
    return _max_duty;
}

double PWMChannel::getFrequency() const {
    return _freq_hz;
}

uint8_t PWMChannel::getResolution() const {
    return _resolution;
}

uint8_t PWMChannel::getPin() const {
    return _pin;
}

bool PWMChannel::isInitialized() const {
    return _initialized;
}

void PWMChannel::printInfo() const {
    Serial.printf("\n=== PWM通道%d 配置信息 ===\n", _channel);
    Serial.printf("引脚: GPIO%d\n", _pin);
    Serial.printf("频率: %.2fHz\n", _freq_hz);
    Serial.printf("分辨率: %d位\n", _resolution);
    Serial.printf("占空比范围: 0-%lu\n", _max_duty);
    Serial.printf("周期: %.2fμs\n", getPeriodUs());
    Serial.printf("状态: %s\n", _initialized ? "已初始化" : "未初始化");
    Serial.println("==========================\n");
}