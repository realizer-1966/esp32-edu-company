/*
 * Heltec V3 LoRa 기본 통신 (송신기)
 * 
 * 하드웨어:
 * - Meshtastic Heltec V3 ESP32-S3 LoRa
 * 
 * 기능:
 * - LoRa 장거리 통신 (1-5km)
 * - OLED 디스플레이 메시지 표시
 * - 5 초마다 데이터 전송
 * 
 * 주파수: 915MHz (한국)
 * 
 * 작성일: 2026-08-18
 * 라이선스: MIT
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

// Heltec V3 LoRa 핀 정의
#define SS 8
#define RST 12
#define DIO0 13

// OLED 디스플레이
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 카운터
int counter = 0;

void setup() {
  Serial.begin(115200);
  
  // OLED 초기화
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "Heltec V3 LoRa");
  u8g2.drawStr(0, 30, "TX Mode");
  u8g2.sendBuffer();
  
  delay(1000);
  
  // LoRa 초기화
  LoRa.setPins(SS, RST, DIO0);
  
  Serial.println("LoRa 초기화중...");
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "LoRa Init...");
  u8g2.sendBuffer();
  
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa 초기화 실패!");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 15, "LoRa Failed!");
    u8g2.sendBuffer();
    while (1);
  }
  
  Serial.println("LoRa 초기화 성공!");
  Serial.println("주파수: 915MHz");
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "LoRa OK!");
  u8g2.drawStr(0, 30, "Freq: 915MHz");
  u8g2.sendBuffer();
  
  delay(1000);
}

void loop() {
  // 전송할 데이터
  String message = "Hello LoRa #" + String(counter);
  
  Serial.println("송신: " + message);
  
  // OLED 업데이트
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "TX: " + message);
  u8g2.drawStr(0, 45, "Count: " + String(counter));
  u8g2.sendBuffer();
  
  // LoRa 패킷 전송
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  
  counter++;
  
  // 5 초 대기
  delay(5000);
}
