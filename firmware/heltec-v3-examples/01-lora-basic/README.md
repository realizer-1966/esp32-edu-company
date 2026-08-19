# Heltec V3 LoRa 기본 통신

Heltec V3 ESP32-S3 LoRa 보드를 사용한 장거리 무선 통신 예제입니다.

## 원리

```
송신기 (TX)               수신기 (RX)
    ↓                       ↓
LoRa 915MHz              LoRa 915MHz
    ↓                       ↓
수 km 전송               수신 + OLED 표시
```

## 하드웨어 요구사항

- **보드**: Meshtastic Heltec V3 ESP32-S3 LoRa × 2
- **안테나**: LoRa 안테나 (868/915MHz) × 2
- **배터리**: 18650 리튬 이온 (선택)
- **USB 케이블**: USB-C × 2

## 설치

### 1. 보드 매니저 설정 (Arduino IDE)

```
1. Arduino IDE 실행
2. 파일 → 설정
3. 추가 보드 관리자 URL:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
4. 도구 → 보드 → 보드 매니저
5. "ESP32 by Espressif Systems" 설치
```

### 2. Heltec V3 선택

```
도구 → 보드 → ESP32S3 Dev Module
도구 → Board: "Heltec WiFi LoRa 32(V3)"
```

### 3. 라이브러리 설치

```
스케치 → 라이브러리 포함 → 라이브러리 관리

검색 및 설치:
- LoRa by Sandeep Mistry
- U8g2 by olikraus
```

### 4. PlatformIO 사용 (추천)

```bash
cd 01-lora-basic
pio lib install
pio run
```

## 사용법

### 1. 안테나 연결

**중요: 안테나 없이 LoRa 동작 금지! (보드 손상)**

```
1. LoRa 안테나를 U.FL 커넥터에 연결
2. 가볍게 눌러서 "딸깍" 소리 확인
3. 절대로 안테나 없이 전원 인가 금지
```

### 2. 드라이버 설치

#### Windows
- **CH9102 드라이버**: http://www.wch.cn/downloads/CH9102DRV_ZIP.html
- 설치 후 장치 관리자에서 COM 포트 확인

#### Linux/macOS
- 기본 지원
- 권한 설정: `sudo usermod -a -G dialout $USER`

### 3. 코드 업로드

#### 송신기 (TX)
```bash
# 01-lora-sender.ino 열기
# 업로드
pio run --target upload
```

#### 수신기 (RX)
```bash
# 01-lora-receiver.ino 열기
# 업로드
pio run --target upload
```

### 4. 테스트

```
1. 송신기와 수신기를 10m 이상 떨어뜨림
2. 송신기 전원 인가
3. 수신기 OLED 에 메시지 표시 확인
4. 시리얼 모니터로 RSSI 확인
```

## 화면 구성

### 송신기 OLED
```
┌─────────────────┐
│ Heltec V3 LoRa  │
│ TX Mode         │
├─────────────────┤
│ TX: Hello LoRa  │
│      #42        │
│ Count: 42       │
└─────────────────┘
```

### 수신기 OLED
```
┌─────────────────┐
│ Heltec V3 LoRa  │
│ RX Mode         │
├─────────────────┤
│ RX: Hello LoRa  │
│ RSSI: -85dBm    │
│ Count: 15       │
└─────────────────┘
```

## 성능

| 환경 | 거리 | RSSI | SNR |
|------|------|------|-----|
| **개방 (시야)** | 3-5km | -90dBm | 8-10dB |
| **도시 (건물)** | 1-2km | -85dBm | 6-8dB |
| **실내 (벽)** | 100-300m | -75dBm | 5-7dB |

## 주파수 설정

### 한국 (915MHz)
```cpp
LoRa.begin(915E6);  // 915MHz
```

### 유럽 (868MHz)
```cpp
LoRa.begin(868E6);  // 868MHz
```

### 미국 (915MHz)
```cpp
LoRa.begin(915E6);  // 915MHz
```

## 확장 아이디어

### 1. 심박수 전송
```cpp
#include <MAX30105.h>

int heartRate = particleSensor.getHeartRate();
LoRa.beginPacket();
LoRa.print("HR:");
LoRa.print(heartRate);
LoRa.endPacket();
```

### 2. GPS 위치 전송
```cpp
#include <TinyGPS++.h>

float lat = gps.location.lat();
float lng = gps.location.lng();

LoRa.beginPacket();
LoRa.print("GPS:");
LoRa.print(lat, 6);
LoRa.print(",");
LoRa.print(lng, 6);
LoRa.endPacket();
```

### 3. 낙상 감지
```cpp
#include <MPU6050.h>

if (acceleration > 5.0) {  // 5G 이상
  LoRa.beginPacket();
  LoRa.print("FALL! GPS:35.1234,129.5678");
  LoRa.endPacket();
}
```

### 4. 메시 네트워크
```cpp
// 중계 기능 추가
if (received.toNode != MY_NODE_ID) {
  // 다른 노드꺼면 재전송
  LoRa.beginPacket();
  LoRa.write(packet, size);
  LoRa.endPacket();
}
```

## 문제 해결

### LoRa 초기화 실패
- **원인**: 안테나 미연결, 핀 설정 오류
- **해결**: 안테나 연결, `#define SS/RST/DIO0` 확인

### 수신 안 됨
- **원인**: 주파수 불일치, 거리太远
- **해결**: 양쪽 915MHz 동일, 안테나 재연결

### RSSI 너무 낮음
- **원인**: 안테나 방향, 장애물
- **해결**: 안테나 수직, 시야 확보

### OLED 출력 안 됨
- **원인**: I2C 주소, 라이브러리 버전
- **해결**: `U8G2_SSD1306_128X64_NONAME_F_HW_I2C` 확인

## 법적 고려사항

### 전파법 (한국)
- **LoRa 915MHz**: 아마추어무선 또는 무선국 허가 필요
- **출력**: 10mW 이하 (Heltec V3 준수)
- **용도**: 실험/교육 목적은 허가 면제

### 주의사항
- **상업용**: 무선국 허가 필요
- **아마추어**: 아마추어무선 기사 자격 필요
- **교육**: 학교/기관 실험은 면제

## 라이선스

MIT License

## 참고

- [Heltec V3 문서](https://docs.heltec.org/en/node/esp32/wifi_lora_32_V3/intro.html)
- [Meshtastic 프로젝트](https://meshtastic.org/)
- [LoRa Alliance](https://lora-alliance.org/)
- [RadioLib 라이브러리](https://github.com/jgromes/RadioLib)
