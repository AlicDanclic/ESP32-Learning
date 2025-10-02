#include <Arduino.h>

void setup() {
  // 这里原本是 LED_BUILTIN，由于没有定义 LED_BUILTIN, 我们这里把它改成 2 就行
  // 或者在代码最前面定义 #define LED_BUILTIN 2
  pinMode(2, OUTPUT); // 初始化 GPIO2 
}

void loop() {
  digitalWrite(2, HIGH);   // 点亮LED
  delay(1000);
  digitalWrite(2, LOW);    // 熄灭LED
  delay(1000);
}
