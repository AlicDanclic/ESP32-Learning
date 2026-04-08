#include <Arduino.h>
#include "ESP32DAC.h"

// ============================================
// 配置区域 - 根据需求修改
// ============================================

// 定义DAC对象
// DAC_CHANNEL_1 -> GPIO25
// DAC_CHANNEL_2 -> GPIO26
ESP32DAC dac1(DAC_CHANNEL_1);
ESP32DAC dac2(DAC_CHANNEL_2);

// 演示模式选择 (取消注释你想测试的模式)
#define DEMO_MODE_VOLTAGE    // 直流电压扫描
// #define DEMO_MODE_WAVEFORM   // 波形生成演示
// #define DEMO_MODE_DUAL       // 双通道独立输出
// #define DEMO_MODE_MUSIC      // 简单音阶播放

// ============================================
// 全局变量
// ============================================

#ifdef DEMO_MODE_VOLTAGE
int voltage = 0;
int step = 100;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 50; // 50ms更新一次
#endif

#ifdef DEMO_MODE_WAVEFORM
WaveformType waveTypes[] = {WAVE_SINE, WAVE_TRIANGLE, WAVE_SAWTOOTH, WAVE_SQUARE};
const char* waveNames[] = {"正弦波", "三角波", "锯齿波", "方波"};
int currentWave = 0;
unsigned long waveStartTime = 0;
const unsigned long WAVE_DURATION = 5000; // 每种波形持续5秒
#endif

#ifdef DEMO_MODE_MUSIC
// C大调音阶频率 (Hz)
float notes[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C4, D4, E4, F4, G4, A4, B4, C5
const char* noteNames[] = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si", "Do"};
int currentNote = 0;
unsigned long noteStartTime = 0;
const unsigned long NOTE_DURATION = 500; // 每个音符500ms
#endif

// ============================================
// 初始化函数
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("    ESP32 DAC 演示程序");
  Serial.println("=================================");
  
  // 初始化DAC
  dac1.begin();
  dac2.begin();
  
  Serial.println("DAC初始化完成");
  Serial.println("通道1: GPIO25");
  Serial.println("通道2: GPIO26");
  Serial.println("---------------------------------");

  // 根据选择的模式进行初始化
  #ifdef DEMO_MODE_VOLTAGE
    Serial.println("模式: 直流电压扫描");
    Serial.println("范围: 0V -> 3.3V -> 0V");
    Serial.println("=================================");
  #endif

  #ifdef DEMO_MODE_WAVEFORM
    Serial.println("模式: 波形生成演示");
    Serial.printf("起始波形: %s (1Hz)\n", waveNames[currentWave]);
    dac1.startWaveform(waveTypes[currentWave], 1.0, 100.0, 50.0);
    waveStartTime = millis();
    Serial.println("=================================");
  #endif

  #ifdef DEMO_MODE_DUAL
    Serial.println("模式: 双通道独立输出");
    // 通道1: 2Hz正弦波
    dac1.startWaveform(WAVE_SINE, 2.0, 100.0, 50.0);
    Serial.println("通道1: 2Hz 正弦波");
    
    // 通道2: 1Hz三角波，80%幅度
    dac2.startWaveform(WAVE_TRIANGLE, 1.0, 80.0, 50.0);
    Serial.println("通道2: 1Hz 三角波 (80%幅度)");
    Serial.println("=================================");
  #endif

  #ifdef DEMO_MODE_MUSIC
    Serial.println("模式: 音阶播放");
    Serial.println("播放: C大调音阶");
    dac1.startWaveform(WAVE_SINE, notes[0], 100.0, 50.0);
    noteStartTime = millis();
    Serial.printf("当前音符: %s (%.0f Hz)\n", noteNames[0], notes[0]);
    Serial.println("=================================");
  #endif
}

// ============================================
// 主循环
// ============================================

void loop() {
  
  // ==================== 模式1: 直流电压扫描 ====================
  #ifdef DEMO_MODE_VOLTAGE
  
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
      lastUpdate = millis();
      
      // 更新电压
      voltage += step;
      
      // 到达边界时反转方向
      if (voltage >= 3300) {
        voltage = 3300;
        step = -100;
      } else if (voltage <= 0) {
        voltage = 0;
        step = 100;
      }
      
      // 设置电压
      dac1.setVoltage(voltage);
      
      // 打印当前状态 (每500ms打印一次)
      static unsigned long lastPrint = 0;
      if (millis() - lastPrint >= 500) {
        lastPrint = millis();
        float volt = voltage / 1000.0;
        uint8_t raw = dac1.getCurrentValue();
        Serial.printf("电压: %.2fV | 原始值: %d/255 | 百分比: %.1f%%\n", 
                      volt, raw, (voltage / 3300.0) * 100);
      }
    }
    
    // 可选：使用DAC2输出固定电压作为参考
    // dac2.setVoltageFloat(1.65); // 1.65V参考电压
    
  #endif

  // ==================== 模式2: 波形生成演示 ====================
  #ifdef DEMO_MODE_WAVEFORM
  
    // 驱动波形生成 (必须在loop中调用)
    dac1.loop();
    
    // 每5秒切换一次波形
    if (millis() - waveStartTime >= WAVE_DURATION) {
      waveStartTime = millis();
      currentWave = (currentWave + 1) % 4;
      
      dac1.startWaveform(waveTypes[currentWave], 1.0, 100.0, 50.0);
      
      Serial.printf("切换波形: %s\n", waveNames[currentWave]);
    }
    
  #endif

  // ==================== 模式3: 双通道独立输出 ====================
  #ifdef DEMO_MODE_DUAL
  
    // 同时驱动两个通道的波形
    dac1.loop();
    dac2.loop();
    
    // 每秒打印一次状态
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 1000) {
      lastPrint = millis();
      Serial.printf("CH1值: %d/255 | CH2值: %d/255\n", 
                    dac1.getCurrentValue(), dac2.getCurrentValue());
    }
    
  #endif

  // ==================== 模式4: 音阶播放 ====================
  #ifdef DEMO_MODE_MUSIC
  
    // 驱动波形生成
    dac1.loop();
    
    // 切换音符
    if (millis() - noteStartTime >= NOTE_DURATION) {
      noteStartTime = millis();
      currentNote = (currentNote + 1) % 8;
      
      // 更新频率
      dac1.setWaveformParams(notes[currentNote], 100.0, 50.0);
      
      Serial.printf("音符: %s (%.0f Hz)\n", noteNames[currentNote], notes[currentNote]);
    }
    
  #endif

  // ==================== 串口命令处理 (通用) ====================
  // 可以通过串口发送命令控制DAC
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "off") {
      dac1.off();
      dac2.off();
      Serial.println("DAC已关闭");
    }
    else if (cmd == "status") {
      Serial.printf("DAC1: %d/255\n", dac1.getCurrentValue());
      Serial.printf("DAC2: %d/255\n", dac2.getCurrentValue());
    }
    else if (cmd.startsWith("v1 ")) {
      int v = cmd.substring(3).toInt();
      dac1.setVoltage(v);
      Serial.printf("DAC1设置为: %dmV\n", v);
    }
    else if (cmd.startsWith("v2 ")) {
      int v = cmd.substring(3).toInt();
      dac2.setVoltage(v);
      Serial.printf("DAC2设置为: %dmV\n", v);
    }
    else if (cmd == "help") {
      Serial.println("可用命令:");
      Serial.println("  off     - 关闭DAC输出");
      Serial.println("  status  - 查看当前状态");
      Serial.println("  v1 xxx  - 设置DAC1电压(mV)");
      Serial.println("  v2 xxx  - 设置DAC2电压(mV)");
      Serial.println("  help    - 显示帮助");
    }
  }
}