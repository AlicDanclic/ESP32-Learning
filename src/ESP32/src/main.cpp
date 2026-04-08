#include "Arduino.h"
#include "PWMChannel.h"

PWMChannel ledPWM(0, 2);
PWMChannel servoPWM(1, 4);
PWMChannel motorPWM(2, 5);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("===== ESP32 PWM类演示 =====");
    
    ledPWM.init(5000.0, 8);
    servoPWM.init(50.0, 12);
    motorPWM.init(20000.0, 10);
    
    ledPWM.printInfo();
    servoPWM.printInfo();
    motorPWM.printInfo();
}

void loop() {
    Serial.println("实验1: LED呼吸灯");
    ledPWM.breathe(3000, 2);
    
    Serial.println("实验2: 舵机控制");
    servoPWM.writeMicroseconds(500.0);
    delay(1000);
    servoPWM.writeMicroseconds(1500.0);
    delay(1000);
    servoPWM.writeMicroseconds(2500.0);
    delay(1000);
    
    Serial.println("实验3: 电机控制");
    for (float speed = 0; speed <= 100; speed += 10) {
        motorPWM.writePercent(speed);
        Serial.printf("  速度: %.0f%%\n", speed);
        delay(300);
    }
    for (float speed = 100; speed >= 0; speed -= 10) {
        motorPWM.writePercent(speed);
        delay(300);
    }
    motorPWM.writePercent(0);
    
    delay(2000);
}