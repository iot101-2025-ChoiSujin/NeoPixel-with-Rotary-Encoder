#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>                  // 조도센서
#include <Adafruit_NeoPixel.h>       // LED 제어

// 핀 설정
#define SDA_PIN 17
#define SCL_PIN 18
#define pulseA 23
#define pulseB 22

// LED 설정
#define ledPin 15 
#define ledNum 8

// 디스플레이, 조도센서 객체
BH1750 lightMeter;

// 네오픽셀 객체
Adafruit_NeoPixel pixels(ledNum, ledPin, NEO_GRB + NEO_KHZ800);

// 로터리 인코더 변수
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
char rotary_buf[20];

// 로터리 인코더 인터럽트 핸들러
IRAM_ATTR void handleRotary() {
    int MSB = digitalRead(pulseA);
    int LSB = digitalRead(pulseB);

    int encoded = (MSB << 1) | LSB;
    int sum = (lastEncoded << 2) | encoded;

    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;
    lastEncoded = encoded;

    if (encoderValue > 255) {
      encoderValue = 255;
    } else if (encoderValue < 0) {
      encoderValue = 0;
    }
}

// 물방울 효과 (재귀 사용)
void waterDrop(int i, int R, int G, int B, int maxTail = 4, int tail = 1) {
    int divider = exp(tail);
    pixels.setPixelColor(i, pixels.Color(R/divider, G/divider, B/divider));
    if (i < ledNum && tail < maxTail) {
        waterDrop(++i, R, G, B, maxTail, ++tail);
    }
}

void setup() {
    // 기본 설정
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    lightMeter.begin();
    
    // 로터리 인코더 핀모드 및 인터럽트
    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE);

    // 네오픽셀 초기화
    pixels.begin();
}

void loop() {
    // 인코더 값으로 delay 시간 조절
    int delayValue = map(encoderValue, 0, 255, 50, 1000);  
    sprintf(rotary_buf, "%d ms", delayValue);
    
    // 랜덤 색상 생성
    int R = random(0, 255);
    int G = random(0, 255);
    int B = random(0, 255);

    // LED 물방울 효과
    for (int i = ledNum - 1; i >= 0 ; i--) {
        pixels.clear();
        waterDrop(i, R, G, B);
        pixels.show();
        delay(delayValue);  
    }
}
