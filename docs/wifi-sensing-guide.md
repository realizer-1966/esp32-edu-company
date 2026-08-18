# WiFi 기반 동작감지 (WiFi Sensing) 가이드

## 개요

WiFi 신호의 변화 (CSI, Channel State Information) 를 분석하여 사람의 움직임, 호흡, 심박수 등을 감지하는 기술입니다.

---

## 1. ESP32-S3 의 WiFi 성능

### 현재 보유 하드웨어
| 항목 | WAVESHARE ESP32-S3-Touch-LCD-4 |
|------|-------------------------------|
| **WiFi 버전** | WiFi 4 (802.11 b/g/n) |
| **주파수** | 2.4GHz 만 지원 |
| **안테나** | 내장 PCB 안테나 |
| **CSI 지원** | 제한적 (Espressif SDK 필요) |

### WiFi 6/7 지원 칩
| 칩 | WiFi 버전 | 특징 |
|----|----------|------|
| **ESP32-C6** | WiFi 6 (802.11ax) | 2.4/5GHz, BLE 5, Zigbee |
| **ESP32-C5** | WiFi 6 (802.11ax) | 듀얼 밴드, 2024 년 출시 |
| **ESP32-C7** | WiFi 7 (802.11be) | 2.4/5/6GHz, 2025 년 출시예정 |

---

## 2. WiFi Sensing 원리

### CSI (Channel State Information)
```
WiFi 신호 = 송신기 → 공간 → 수신기

공간의 변화 (사람 움직임)
    ↓
WiFi 신호 위상/진폭 변화
    ↓
CSI 데이터 변화
    ↓
동작 감지 알고리즘
    ↓
움직임/호흡/심박수 감지
```

### 감지 가능한 신호
| 신호 | 주파수 | 감도 |
|------|--------|------|
| **대형 움직임** | 1-10Hz | 높음 (걸어다니기) |
| **손 움직임** | 5-20Hz | 중간 (제스처) |
| **호흡** | 0.2-0.5Hz | 높음 (가슴 확장) |
| **심박수** | 1-2Hz (60-120BPM) | 낮음 (미세 진동) |

---

## 3. ESP32-S3 로 가능한 것

### ✅ 가능
1. **대형 움직임 감지** (방 안 걸어다니기)
2. **존재 감지** (방에 사람 있음/없음)
3. **간단한 제스처** (손 흔들기)
4. **Fall Detection** (낙상 감지)

### ⚠️ 제한적
1. **호흡 감지** - 거리 1m 이내, 정밀도 낮음
2. **심박수** - 매우 어려움 (PPG 센서 권장)

### ❌ 불가능
1. **정밀한 생체신호** (ECG 수준)
2. **다중 사람 추적** (2 명 이상)
3. **WiFi 6/7 전용 기능** (하드웨어 제한)

---

## 4. 구현 방법

### 방법 1: ESP32 CSI Tool (연구용)

```cpp
#include <esp_wifi.h>
#include <esp_wifi_types.h>

// CSI 데이터 콜백
void wifi_csi_cb(void *ctx, wifi_csi_info_t *info) {
  // CSI 데이터 처리
  // info->buf: 128 바이트 CSI 데이터
  // info->rx_state: 신호 상태
  
  // 동작 감지 알고리즘
  detectMotion(info);
}

void setup() {
  // WiFi 스테이션 모드
  WiFi.begin("ssid", "password");
  
  // CSI 콜백 등록
  esp_wifi_set_csi(true);
  esp_wifi_set_csi_rx_cb(wifi_csi_cb, NULL);
}
```

**참고 프로젝트:**
- https://github.com/colinoflynn/esp32-csi-tool
- https://github.com/Abram-son/ESP32-CSIFallDetection

### 방법 2: RSSI 기반 감지 (간단)

```cpp
#include <WiFi.h>

int lastRSSI = 0;
int motionThreshold = 10; // dB 변화 임계값

void setup() {
  WiFi.begin("ssid", "password");
}

void loop() {
  int rssi = WiFi.RSSI();
  int delta = abs(rssi - lastRSSI);
  
  if (delta > motionThreshold) {
    Serial.println("동작 감지!");
  }
  
  lastRSSI = rssi;
  delay(100);
}
```

### 방법 3: ESP32-C6 (WiFi 6) 사용

```cpp
// ESP32-C6 는 WiFi 6 지원
// 더 정밀한 CSI 데이터 수집 가능
// 5GHz 대역 추가로 다중 경로 분석 향상

#include <esp_wifi.h>

// ESP32-C6 에서 더 정밀한 CSI 데이터 활용
// 호흡/심박수 감지 정확도 향상
```

---

## 5. 실제 프로젝트 예제

### 프로젝트 1: 존재 감지기 (RSSI 기반)

```cpp
#include <WiFi.h>
#include <LovyanGFX.h>

LGFX lcd;

const char* ssid = "your_wifi";
const char* password = "your_password";

int rssiHistory[60];
int rssiIndex = 0;
bool isOccupied = false;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  
  WiFi.begin(ssid, password);
  
  lcd.print("WiFi 존재 감지기");
  lcd.print("연결중...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  
  lcd.printf("\n연결됨!\nRSSI: %d dBm\n", WiFi.RSSI());
}

void loop() {
  int rssi = WiFi.RSSI();
  rssiHistory[rssiIndex++] = rssi;
  
  if (rssiIndex >= 60) {
    rssiIndex = 0;
    
    // 표준편차 계산 (변동성)
    float stdDev = calculateStdDev(rssiHistory, 60);
    
    // 변동성이 크면 동작 감지
    if (stdDev > 5.0) {
      isOccupied = true;
      lcd.fillScreen(TFT_GREEN);
      lcd.print("사람 있음");
    } else {
      isOccupied = false;
      lcd.fillScreen(TFT_RED);
      lcd.print("사람 없음");
    }
  }
  
  delay(100);
}

float calculateStdDev(int* data, int len) {
  float sum = 0;
  for (int i = 0; i < len; i++) {
    sum += data[i];
  }
  float mean = sum / len;
  
  float variance = 0;
  for (int i = 0; i < len; i++) {
    variance += (data[i] - mean) * (data[i] - mean);
  }
  
  return sqrt(variance / len);
}
```

### 프로젝트 2: 낙상 감지 (CSI 기반)

```cpp
#include <esp_wifi.h>
#include <Arduino.h>

// CSI 데이터 버퍼
wifi_csi_info_t csiBuffer[100];
int csiIndex = 0;

// 낙상 감지 임계값
const float FALL_THRESHOLD = 30.0;

void wifi_csi_cb(void *ctx, wifi_csi_info_t *info) {
  csiBuffer[csiIndex++] = *info;
  
  if (csiIndex >= 100) {
    csiIndex = 0;
    
    // CSI 진폭 변화 분석
    float amplitudeChange = analyzeCSI(csiBuffer, 100);
    
    // 급격한 변화 = 낙상
    if (amplitudeChange > FALL_THRESHOLD) {
      Serial.println("낙상 감지!");
      sendAlert();
    }
  }
}

float analyzeCSI(wifi_csi_info_t* buffer, int len) {
  // CSI 진폭 계산
  float amplitudes[100];
  
  for (int i = 0; i < len; i++) {
    int8_t* csi_data = (int8_t*)buffer[i].buf;
    float sum = 0;
    
    // 52 서브캐리어 평균
    for (int j = 0; j < 52; j++) {
      sum += abs(csi_data[j * 2]);
    }
    
    amplitudes[i] = sum / 52;
  }
  
  // 표준편차 (변동성)
  float mean = 0;
  for (int i = 0; i < len; i++) {
    mean += amplitudes[i];
  }
  mean /= len;
  
  float variance = 0;
  for (int i = 0; i < len; i++) {
    variance += (amplitudes[i] - mean) * (amplitudes[i] - mean);
  }
  
  return sqrt(variance);
}

void sendAlert() {
  // GPIO 로 부저/LED 작동
  // 또는 WiFi 로 서버에 알림
}

void setup() {
  Serial.begin(115200);
  
  // WiFi 스테이션 모드
  WiFi.begin("ssid", "password");
  
  // CSI 활성화
  esp_wifi_set_csi(true);
  esp_wifi_set_csi_rx_cb(wifi_csi_cb, NULL);
}

void loop() {
  delay(100);
}
```

### 프로젝트 3: 호흡 감지 (고급)

```cpp
#include <esp_wifi.h>
#include <FFT.h> // FFT 라이브러리

// CSI 데이터
float csiAmplitude[256];
int csiIndex = 0;

// 호흡 주파수 범위 (0.2-0.5Hz = 분당 12-30 회)
const float BREATH_MIN = 0.2;
const float BREATH_MAX = 0.5;

void wifi_csi_cb(void *ctx, wifi_csi_info_t *info) {
  // CSI 진폭 추출
  int8_t* csi_data = (int8_t*)info->buf;
  float amplitude = 0;
  
  for (int i = 0; i < 52; i++) {
    amplitude += abs(csi_data[i * 2]);
  }
  amplitude /= 52;
  
  csiAmplitude[csiIndex++] = amplitude;
  
  if (csiIndex >= 256) {
    csiIndex = 0;
    
    // FFT 로 주파수 분석
    float breathRate = analyzeBreath(csiAmplitude, 256);
    
    if (breathRate > 0) {
      Serial.printf("호흡수: %.1f 회/분\n", breathRate * 60);
    }
  }
}

float analyzeBreath(float* data, int len) {
  // FFT 적용
  // 0.2-0.5Hz 대역에서 최대 피크 찾기
  
  // 예시: 단순 제로크로싱
  int zeroCrossings = 0;
  float mean = 0;
  
  for (int i = 0; i < len; i++) {
    mean += data[i];
  }
  mean /= len;
  
  for (int i = 1; i < len; i++) {
    if ((data[i-1] - mean) * (data[i] - mean) < 0) {
      zeroCrossings++;
    }
  }
  
  // 초당 제로크로싱 → Hz → 분당 호흡수
  float freq = zeroCrossings / (2.0 * len / 10.0); // 10ms 샘플링 가정
  
  if (freq >= BREATH_MIN && freq <= BREATH_MAX) {
    return freq;
  }
  
  return 0;
}
```

---

## 6. WiFi 6/7 로 업그레이드

### ESP32-C6 보드

| 항목 | ESP32-C6 | ESP32-S3 |
|------|----------|----------|
| **WiFi** | WiFi 6 (802.11ax) | WiFi 4 (802.11n) |
| **주파수** | 2.4GHz + 5GHz | 2.4GHz 만 |
| **BLE** | 5.0 | 5.0 |
| **Zigbee** | ✅ | ❌ |
| **Thread** | ✅ | ❌ |
| **가격** | 15,000 원 | 10,000 원 |

### 추천 보드
1. **Seeed Studio XIAO ESP32C6**: 8,000 원, 소형
2. **ESP32-C6-DevKitC-1**: 15,000 원, 공식 개발보드
3. **ESP32-C6-WROOM-1**: 12,000 원, 모듈

### WiFi 6 장점
- **OFDMA**: 더 정밀한 CSI 데이터
- **5GHz 대역**: 다중 경로 분석 향상
- **TWT (Target Wake Time)**: 저전력 동작
- **호흡/심박수 감지 정확도 향상**

---

## 7. 비교 표

| 기술 | ESP32-S3 | ESP32-C6 | 전용 WiFi 센싱 |
|------|----------|----------|---------------|
| **동작 감지** | ✅ | ✅✅ | ✅✅✅ |
| **존재 감지** | ✅ | ✅✅ | ✅✅✅ |
| **호흡 감지** | ⚠️ | ✅ | ✅✅ |
| **심박수** | ❌ | ⚠️ | ✅ |
| **제스처** | ⚠️ | ✅ | ✅✅ |
| **가격** | 10,000 원 | 15,000 원 | 50,000 원+ |

### 전용 WiFi 센싱 칩
- **Google Soli**: 60GHz 레이더 (Pixel 4 제스처)
- **Vayyar**: 60GHz 라이다 (3D 이미징)
- **Origin Wireless**: WiFi 6 기반 홈 모니터링

---

## 8. 추천 접근법

### 단계 1: RSSI 기반 (오늘 시작)
- ESP32-S3 로 즉시 구현 가능
- 존재 감지, 대형 움직임
- 코드 복잡도 낮음

### 단계 2: CSI 기반 (1-2 주)
- ESP32 CSI Tool 학습
- 낙상 감지, 호흡 감지
- 신호 처리 지식 필요

### 단계 3: ESP32-C6 (1 개월)
- WiFi 6 로 업그레이드
- 정밀도 향상
- 5GHz 대역 활용

### 단계 4: 센서 퓨전 (2 개월)
- WiFi + MAX30102 (심박)
- WiFi + BME680 (환경)
- AI/머신러닝 통합

---

## 9. 한계 및 주의사항

### 기술적 한계
1. **ESP32-S3 는 WiFi 4 만 지원**
   - WiFi 6/7 기능 사용 불가
   - CSI 데이터 접근 제한적

2. **정밀도**
   - 호흡: 80-90% (1m 이내)
   - 심박수: 60-70% (매우 제한적)
   - 대형 움직임: 95%+

3. **환경 영향**
   - WiFi 라우터 거리/위치
   - 벽/가구 반사
   - 다른 전자기기 간섭

### 법적 제한
- **CSI 데이터**: 국가별 규제 확인
- **개인정보**: 존재 감지 데이터 저장 주의
- **의료기기**: 호흡/심박수는 의료기기로 분류될 수 있음

---

## 10. 다음 단계

### 즉시 시작
1. **RSSI 기반 존재 감지기** (1 시간)
   - 현재 ESP32-S3 로 가능
   - 코드 위 예제 참조

2. **ESP32 CSI Tool 설치** (2 시간)
   - https://github.com/colinoflynn/esp32-csi-tool
   - CSI 데이터 수집 테스트

### 중기 목표
3. **ESP32-C6 구매** (1 주)
   - Seeed Studio XIAO ESP32C6
   - WiFi 6 CSI 데이터 수집

4. **호흡 감지 구현** (2 주)
   - FFT 신호 처리
   - 0.2-0.5Hz 대역 필터링

### 장기 목표
5. **WiFi + PPG 퓨전** (1 개월)
   - WiFi (호흡) + MAX30102 (심박)
   - 수면 모니터링 시스템

6. **상업화** (3 개월)
   - 노인 케어 낙상 감지기
   - 의료기기 인증 (Class I)

---

**결론: ESP32-S3 로 WiFi 기반 동작감지는 가능하지만, WiFi 6/7 은 ESP32-C6/C7 이 필요합니다.**

**추천: 먼저 RSSI 기반 존재 감지기로 시작 → ESP32-C6 로 업그레이드 → CSI 기반 호흡 감지**

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**참고**: ESP32 CSI Tool, Google Soli, Origin Wireless
