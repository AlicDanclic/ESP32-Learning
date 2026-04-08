#ifndef PWMCHANNEL_H
#define PWMCHANNEL_H

#include <Arduino.h>
#include <driver/ledc.h>

class PWMChannel {
private:
    uint8_t _channel;
    uint8_t _pin;
    double _freq_hz;
    uint8_t _resolution;
    bool _initialized;
    uint32_t _max_duty;
    
    void _calculateMaxDuty();

public:
    PWMChannel(uint8_t channel = 0, uint8_t pin = 2);
    
    bool init(double freq_hz = 1000.0, uint8_t resolution_bits = 8);
    bool writePercent(float percent);
    bool writeMicroseconds(float pulse_width_us);
    bool writeDuty(uint32_t duty_value);
    void breathe(unsigned long duration_ms = 3000, int cycles = 1);
    
    float getPeriodUs() const;
    float getPulseWidthUs(float percent) const;
    uint32_t getMaxDuty() const;
    double getFrequency() const;
    uint8_t getResolution() const;
    uint8_t getPin() const;
    bool isInitialized() const;
    void printInfo() const;
};

#endif