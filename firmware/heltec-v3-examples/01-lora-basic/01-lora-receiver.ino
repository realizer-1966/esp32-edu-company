/*
 * Heltec V3 LoRa 기본 통신 (수신기)
 * 
 * 하드웨어:
 * - Meshtastic Heltec V3 ESP32-S3 LoRa
 * 
 * 기능:
 * - LoRa 장거리 수신 (1-5km)
 * - OLED 에 수신 메시지 표시
 * - RSSI (신호 강도) 표시
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

// 수신 카운터
int receiveCount = 0;

void setup() {
  Serial.begin(115200);
  
  // OLED 초기화
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "Heltec V3 LoRa");
  u8g2.drawStr(0, 30, "RX Mode");
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
  Serial.println("수신 대기중...");
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "LoRa OK!");
  u8g2.drawStr(0, 30, "Listening...");
  u8g2.sendBuffer();
}

void loop() {
  // 패킷 확인
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // 패킷 수신
    String received = "";
    
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    // RSSI (신호 강도)
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    
    // 시리얼 출력
    Serial.println("수신: " + received);
    Serial.println("RSSI: " + String(rssi) + " dBm");
    Serial.println("SNR: " + String(snr) + " dB");
    Serial.println("---");
    
    // 수신 카운트 증가
    receiveCount++;
    
    // OLED 업데이트
    u8g2.clearBuffer();
    
    // 1 행: 수신 메시지 (잘라서 표시)
    if (received.length() > 16) {
      received = received.substring(0, 16) + ".";
    }
    u8g2.drawStr(0, 15, "RX: " + received);
    
    // 2 행: RSSI
    String rssiStr = "RSSI: " + String(rssi) + "dBm";
    u8g2.drawStr(0, 35, rssiStr);
    
    // 3 행: 수신 카운트
    String countStr = "Count: " + String(receiveCount);
    u8g2.drawStr(0, 55, countStr);
    
    u8g2.sendBuffer();
    
    // LED 점멸 (수신 알림)
    // Heltec V3 에는 LED 가 없음 → OLED 플래시
    delay(500);
  }
  
  delay(100);
}
