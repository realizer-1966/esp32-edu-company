# Heltec V3 ESP32-S3 LoRa 개발환경 설정

Meshtastic Heltec V3 ESP32-S3 LoRa 보드의 개발환경 설정 가이드입니다.

---

## 1. Heltec V3 하드웨어 스펙

### 주요 스펙
| 항목 | 사양 |
|------|------|
| **MCU** | ESP32-S3 (듀얼코어, 240MHz) |
| **Flash** | 8MB PSRAM |
| **WiFi** | 2.4GHz 802.11 b/g/n |
| **블루투스** | BLE 5.0 |
| **LoRa** | SX1262 (868/915MHz) |
| **디스플레이** | 0.96 인치 OLED (128x64, I2C) |
| **배터리** | 리튬 이온 충전 회로 내장 (18650/21700) |
| **USB** | USB-C (CH9102 시리얼) |
| **안테나** | LoRa (U.FL) + WiFi (온보드) |
| **GPIO** | 22 개 (5V 허용 핀 일부) |
| **크기** | 60 × 27 mm |

### 핀아웃 (주요 핀)
```
LoRa:
├─ SS: GPIO 8
├─ RST: GPIO 12
├─ DIO0: GPIO 13
├─ BUSY: GPIO 14
└─ TXEN: GPIO 21 / RXEN: GPIO 22

OLED:
├─ SDA: GPIO 41
├─ SCL: GPIO 42
└─ RST: GPIO 21

USB:
└─ CH9102 (자동 리셋 회로)

배터리:
├─ BAT_ADC: GPIO 1 (전압 측정)
└─ 충전: TP4056 내장
```

---

## 2. 필수 소프트웨어

### 2.1 Arduino IDE 설정 (초보자 추천) ⭐

#### 1. Arduino IDE 설치
```
다운로드: https://www.arduino.cc/en/software

Windows: https://www.arduino.cc/download.php
macOS: App Store 또는 공식 사이트
Linux: sudo apt install arduino-ide (또는 공식 사이트)
```

#### 2. ESP32 보드 매니저 추가
```
1. Arduino IDE 실행
2. 파일 → 설정 (Ctrl+,)
3. "추가 보드 관리자 URL" 입력:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
4. 확인 클릭
```

#### 3. ESP32 보드 설치
```
1. 도구 → 보드 → 보드 매니저
2. "esp32" 검색
3. "ESP32 by Espressif Systems" 설치 (버전 3.0.0 이상)
```

#### 4. Heltec V3 선택
```
1. 도구 → 보드 → ESP32S3 Dev Module
2. 보드 선택: "Heltec WiFi LoRa 32(V3)"
```

#### 5. 라이브러리 설치
```
스케치 → 라이브러리 포함 → 라이브러리 관리

검색 및 설치:
- LoRa by Sandeep Mistry (버전 0.8.0 이상)
- U8g2 by olikraus (버전 2.35.0 이상)
- RadioLib by Jan Gromes (선택, 고급용)
```

---

### 2.2 PlatformIO IDE (고급 추천) ⭐⭐⭐

#### 1. VS Code + PlatformIO 설치
```
1. VS Code 다운로드: https://code.visualstudio.com/
2. VS Code 실행
3. 확장 프로그램 (Ctrl+Shift+X)
4. "PlatformIO IDE" 검색 → 설치
5. 재시작
```

#### 2. 새 프로젝트 생성
```bash
# CLI 에서
pio project create --board heltec_wifi_lora_32_V3 --name heltec-lora-test

# 또는 VS Code 에서:
# PlatformIO Home → New Project → Heltec WiFi LoRa 32 V3
```

#### 3. platformio.ini 설정
```ini
[env:heltec-v3]
platform = espressif32
board = heltec_wifi_lora_32_V3
framework = arduino

; LoRa 주파수 (한국 915MHz)
build_flags = 
  -DLO_RA_FREQUENCY=915000000
  -DREGION_KR=1
  -DCORE_DEBUG_LEVEL=3
  -DBOARD_HAS_PSRAM
  
; 라이브러리
lib_deps = 
  sandeepmistry/LoRa@^0.8.0
  olikraus/U8g2@^2.35.0
  
; 업로드 설정
upload_speed = 921600
monitor_speed = 115200
monitor_filters = esp32_exception_decoder, time
```

#### 4. 빌드 및 업로드
```bash
# 빌드
pio run

# 업로드
pio run --target upload

# 시리얼 모니터
pio device monitor

# 빌드 + 업로드 + 모니터
pio run --target upload && pio device monitor
```

---

### 2.3 Meshtastic 펌웨어 (바로 사용) ⭐⭐

Meshtastic 은 오픈소스 LoRa 메시 네트워크 펌웨어입니다.

#### 웹 플래시 (가장 쉬움)
```
1. 크롬/엣지 브라우저 접속
   https://meshtastic.org/docs/software/android/installation/

2. Heltec V3 선택

3. USB 로 보드 연결

4. "Flash" 버튼 클릭 (1-2 분)

5. Android/iOS 앱으로 연결 (블루투스)
```

#### Android 앱
```
1. Google Play 에서 "Meshtastic" 설치
   https://play.google.com/store/apps/details?id=com.geeksville.mesh

2. 블루투스 켜기

3. 앱 실행 → Heltec V3 연결

4. 채팅/센서/지도 사용
```

#### iOS 앱
```
1. App Store 에서 "Meshtastic" 설치
   https://apps.apple.com/app/meshtastic/id6448847978

2. 블루투스 켜기

3. 앱 실행 → Heltec V3 연결
```

---

## 3. USB 드라이버 설치

### Windows

#### CH9102 드라이버
```
1. 드라이버 다운로드:
   http://www.wch.cn/downloads/CH9102DRV_ZIP.html

2. ZIP 압축 풀기

3. setup.exe 실행 (관리자 권한)

4. 설치 완료 후 재부팅

5. 장치 관리자 확인:
   - 포트 (COM & LPT) → USB-SERIAL CH9102
```

#### 자동 리셋 문제
Heltec V3 는 자동 리셋 회로가 내장되어 있어 추가 작업이 필요 없습니다.

### macOS

```bash
# 최신 macOS 는 기본 지원
# 구버전은 CP210x 드라이버 설치
# https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

# 포트 확인
ls /dev/cu.usbserial*
```

### Linux

```bash
# 기본 지원됨
ls /dev/ttyUSB*

# 권한 설정
sudo usermod -a -G dialout $USER

# 재부팅 후 확인
ls -l /dev/ttyUSB0
```

---

## 4. 첫 코드 업로드

### 4.1 LED 점멸 예제

```cpp
// Heltec V3 에는 내장 LED 가 없음 → OLED 사용
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
}

void loop() {
  u8g2.clearBuffer();
  u8g2.drawStr(0, 30, "Hello Heltec V3!");
  u8g2.sendBuffer();
  delay(1000);
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 30, "LoRa Test");
  u8g2.sendBuffer();
  delay(1000);
}
```

### 4.2 LoRa 송신기 예제

```cpp
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

#define SS 8
#define RST 12
#define DIO0 13

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int counter = 0;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(915E6)) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 30, "LoRa Failed!");
    u8g2.sendBuffer();
    while (1);
  }
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 30, "LoRa OK 915MHz");
  u8g2.sendBuffer();
}

void loop() {
  String msg = "Hello LoRa #" + String(counter);
  
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "TX: " + msg);
  u8g2.drawStr(0, 45, "Count: " + String(counter));
  u8g2.sendBuffer();
  
  counter++;
  delay(5000);
}
```

### 4.3 LoRa 수신기 예제

```cpp
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

#define SS 8
#define RST 12
#define DIO0 13

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int receiveCount = 0;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(915E6)) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 30, "LoRa Failed!");
    u8g2.sendBuffer();
    while (1);
  }
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 30, "Listening...");
  u8g2.sendBuffer();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    int rssi = LoRa.packetRssi();
    
    Serial.println("수신: " + received);
    Serial.println("RSSI: " + String(rssi) + " dBm");
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 15, "RX: " + received);
    u8g2.drawStr(0, 35, "RSSI: " + String(rssi) + "dBm");
    u8g2.drawStr(0, 55, "Count: " + String(++receiveCount));
    u8g2.sendBuffer();
  }
  
  delay(100);
}
```

---

## 5. 문제 해결

### ❌ "Failed to connect to ESP32-S3"

**해결 1: 부트모드 진입**
```
1. BOOT 버튼 누르기
2. RESET 버튼 누르기
3. BOOT 버튼 놓기
4. RESET 버튼 놓기
5. 업로드 재시도
```

**해결 2: USB 케이블 확인**
- 데이터 지원 USB-C 케이블 사용 (충전 전용 X)

**해결 3: 드라이버 재설치**
```
1. 장치 관리자에서 CH9102 제거
2. 드라이버 재설치
3. 재부팅
```

---

### ❌ "LoRa 초기화 실패"

**원인 1: 안테나 미연결**
```
해결: LoRa 안테나를 U.FL 커넥터에 단단히 연결
```

**원인 2: 핀 설정 오류**
```cpp
// Heltec V3 핀 확인
#define SS 8
#define RST 12
#define DIO0 13
```

**원인 3: 주파수 불일치**
```cpp
// 한국 915MHz
LoRa.begin(915E6);

// 유럽 868MHz
// LoRa.begin(868E6);
```

---

### ❌ "OLED 출력 안 됨"

**해결 1: I2C 주소 확인**
```cpp
// Heltec V3 OLED 는 기본 I2C 주소 사용
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

**해결 2: 핀아웃 확인**
```cpp
// Heltec V3 OLED 핀
// SDA: GPIO 41
// SCL: GPIO 42
// RST: GPIO 21
```

**해결 3: 라이브러리 버전**
```
U8g2 라이브러리 2.35.0 이상 설치
```

---

### ❌ "배터리 충전 안 됨"

**해결:**
```
1. 18650/21700 배터리 방향 확인 (+/-)
2. USB-C 로 전원 연결 (충전 우선)
3. 충전 LED 확인 (빨강: 충전중, 초록: 완충)
4. 과방전 배터리는 복원 필요
```

---

### ❌ "RSSI 너무 낮음"

**해결:**
```
1. 안테나 방향 수직으로
2. 시야 확보 (장애물 제거)
3. 거리 단축 (100m → 500m → 1km)
4. 출력 조정 (LoRa.setTxPower)
```

```cpp
// 출력 조정 (14-22dBm)
LoRa.setTxPower(20, PA_BOOST);
```

---

## 6. 최적화 설정

### 6.1 저전력 모드

```cpp
#include <esp_sleep.h>

void setup() {
  // 10 초마다 웨이크업
  esp_sleep_enable_timer_wakeup(10 * 1000000);
}

void loop() {
  // 데이터 측정/전송
  sendData();
  
  // 딥슬립
  esp_deep_sleep_start();
}
```

### 6.2 배터리 전압 측정

```cpp
#define BAT_ADC 1

void setup() {
  analogReadResolution(12);
}

float readBatteryVoltage() {
  int adc = analogRead(BAT_ADC);
  float voltage = adc * (3.3 / 4095.0) * 2.0;  // 전압 분할
  return voltage;
}

void loop() {
  float batVoltage = readBatteryVoltage();
  Serial.printf("배터리: %.2fV\n", batVoltage);
  delay(1000);
}
```

### 6.3 LoRa 출력 조정

```cpp
void setup() {
  LoRa.begin(915E6);
  
  // 출력 조정 (14-22dBm)
  LoRa.setTxPower(20, PA_BOOST);
  
  // 스프레딩 팩터 (7-12)
  // 높을수록 거리↑, 속도↓
  LoRa.setSpreadingFactor(10);
  
  // 대역폭 (7.8kHz - 500kHz)
  LoRa.setSignalBandwidth(125E3);
  
  // 코딩 레이트 (5-8)
  LoRa.setCodingRate4(5);
}
```

---

## 7. Meshtastic 커스터마이징

### 7.1 펌웨어 빌드

```bash
# Meshtastic 소스 클론
git clone https://github.com/meshtastic/firmware.git
cd firmware

# PlatformIO 로 빌드
pio run -e heltec-v3

# 업로드
pio run -e heltec-v3 --target upload
```

### 7.2 설정 변경

```cpp
// meshtastic_config.h
#define LORA_FREQUENCY 915.0f  // 한국 주파수
#define LORA_TX_POWER 20       // 출력 20dBm
#define DEVICE_NAME "Heltec-V3-01"
```

### 7.3 센서 추가

```cpp
// meshtastic_sensor.cpp
#include <MAX30105.h>

MAX30105 particleSensor;

void setupSensor() {
  particleSensor.begin(Wire, I2C_SPEED_FAST);
}

float readHeartRate() {
  return particleSensor.getHeartRate();
}
```

---

## 8. 체크리스트

### 설치 완료
- [ ] Arduino IDE 또는 PlatformIO 설치
- [ ] ESP32 보드 매니저 추가
- [ ] Heltec V3 보드 선택
- [ ] LoRa/U8g2 라이브러리 설치
- [ ] CH9102 드라이버 설치 (Windows)

### 테스트
- [ ] LED 점멸 예제 업로드
- [ ] OLED 디스플레이 테스트
- [ ] LoRa 기본 통신 (100m)
- [ ] LoRa 장거리 테스트 (1km+)
- [ ] 배터리 충전 확인

### Meshtastic
- [ ] 웹 플래시로 펌웨어 설치
- [ ] Android/iOS 앱 연결
- [ ] 채팅 테스트
- [ ] GPS 위치 공유 (선택)

---

## 9. 추천 개발 워크플로우

### 일상 개발
```bash
# 1. 코드 수정
code src/main.cpp

# 2. 빌드
pio run

# 3. 업로드
pio run --target upload

# 4. 시리얼 모니터
pio device monitor

# 5. Git 커밋
git add .
git commit -m "feat: LoRa 출력 최적화"
git push
```

### Meshtastic 커스터마이징
```bash
# 1. Meshtastic 소스 클론
git clone https://github.com/meshtastic/firmware.git

# 2. 설정 수정
vim meshtastic_config.h

# 3. 빌드
pio run -e heltec-v3

# 4. 업로드
pio run -e heltec-v3 --target upload
```

---

## 10. 추가 리소스

### 공식 문서
- [Heltec V3 문서](https://docs.heltec.org/en/node/esp32/wifi_lora_32_V3/intro.html)
- [Meshtastic 프로젝트](https://meshtastic.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)

### 커뮤니티
- [Meshtastic Discord](https://discord.gg/meshtastic)
- [Heltec Forum](https://forum.heltec.cn/)
- [ESP32 Forum](https://esp32.com/)

### 튜토리얼
- [LoRa 통신 기초](https://randomnerdtutorials.com/projects-esp32/)
- [Meshtastic 시작하기](https://meshtastic.org/docs/getting-started/)

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**검증**: Windows 11, macOS Sonoma, Ubuntu 22.04
