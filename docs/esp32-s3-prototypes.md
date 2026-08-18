# ESP32-S3 헬스케어 디바이스 프로토타입

이 문서에서는 3 종 ESP32-S3 보드를 사용하여 실제로 제작할 수 있는 헬스케어 디바이스 프로토타입을 상세히 설명합니다.

---

## 1. 스마트 워치 (1.28 인치 원형 보드)

### 개요
- **난이도**: 중급
- **제작 시간**: 6-8 시간
- **비용**: 70,000 원 (보드 + 센서)
- **완제품 목표가**: 150,000 원

### 하드웨어 구성

| 부품 | 수량 | 단가 | 비고 |
|------|------|------|------|
| ESP32-S3 원형 1.28" 보드 | 1 개 | 25,000 원 | 메인 컨트롤러 |
| MAX30102 센서 | 1 개 | 8,000 원 | 심박/혈중산소 |
| Li-Po 배터리 (300mAh) | 1 개 | 10,000 원 | 3.7V, 충전 내장 |
| 실리콘 워치 밴드 (20mm) | 1 개 | 5,000 원 | 표준 20mm |
| 3D 프린팅 케이스 | 1 개 | 10,000 원 | PLA, 방수 |
| USB-C 케이블 | 1 개 | 5,000 원 | 충전/디버깅 |
| 나사/스탠드오프 | 1 세트 | 2,000 원 | M2x6mm |
| **총계** | | **65,000 원** | |

### 회로 연결

```
MAX30102 → ESP32-S3
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8 (I2C0)
└─ SCL → GPIO 9 (I2C0)

배터리 → JST PH 2.0 커넥터 (보드 내장 충전 회로)
```

### 3D 케이스 디자인

```
[정면]
┌─────────────────┐
│   ┌───────┐     │
│   │       │     │  ← 1.28" 원형 디스플레이
│   │  LCD  │     │     노출부 (Φ35mm)
│   │       │     │
│   └───────┘     │
│  [터치 영역]    │
└─────────────────┘
    │         │
    └────┬────┘   ← 밴드 연결부 (20mm)
         │

[측면]
┌─────────────────┐
│  ┌───────────┐  │
│  │  ESP32    │  │  ← 두께 12mm
│  │   PCB     │  │
│  └───────────┘  │
│   ┌─────────┐   │
│   │ 배터리  │   │  ← 두께 5mm
│   └─────────┘   │
└─────────────────┘
```

### 펌웨어 구조

```cpp
#include <TFT_eSPI.h>
#include <MAX30102.h>
#include <TouchLibrary.h>

TFT_eSPI tft;
MAX30102 sensor;
Touch touch;

// 상태 머신
enum WatchState {
  STATE_WATCH_FACE,      // 시계 화면
  STATE_HEART_RATE,      // 심박 측정
  STATE_SPO2,           // 혈중산소
  STATE_ACTIVITY,       // 활동량
  STATE_SLEEP,          // 수면 분석
  STATE_SETTINGS        // 설정
};

WatchState currentState = STATE_WATCH_FACE;

void setup() {
  tft.init();
  sensor.begin();
  touch.begin();
  
  // 터치 이벤트 등록
  touch.onTap(handleTap);
  touch.onSwipe(handleSwipe);
}

void loop() {
  switch (currentState) {
    case STATE_WATCH_FACE:
      drawWatchFace();
      break;
    case STATE_HEART_RATE:
      drawHeartRate();
      break;
    case STATE_SPO2:
      drawSpO2();
      break;
    // ...
  }
  
  touch.poll(); // 터치 처리
  delay(100);
}

// 원형 UI: 심박수 표시
void drawHeartRate() {
  tft.fillCircle(120, 120, 110, TFT_BLACK);
  
  // 하트 아이콘
  tft.drawIcon(heart_icon, 100, 80, 40, 40, TFT_RED);
  
  // 심박수 (중앙)
  tft.setCursor(80, 130);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(5);
  tft.print(sensor.readHeartRate());
  
  // BPM 텍스트
  tft.setTextSize(2);
  tft.setCursor(140, 150);
  tft.print("BPM");
  
  // 그래프 (하단)
  drawHeartRateGraph();
}

// 터치 제스처
void handleTap(int x, int y) {
  // 화면 전환
  currentState = (WatchState)((currentState + 1) % 6);
}

void handleSwipe(int direction) {
  // 스와이프 방향에 따른 기능
  if (direction == SWIPE_UP) {
    // 알림 확인
  } else if (direction == SWIPE_DOWN) {
    // 퀵 설정
  }
}
```

### 기능

#### 1. 심박수 측정
- **샘플링**: 1 Hz (연속), 0.1 Hz (절전)
- **정확도**: ±2 BPM (안정 시)
- **알림**: 고심박 (>100), 저심박 (<50)

#### 2. 혈중산소포화도 (SpO2)
- **측정**: 30 초 평균
- **정확도**: ±2%
- **알림**: 저산소 (<90%)

#### 3. 활동량 트래킹
- **걸음수**: 내장 IMU (가속도계)
- **거리**: 보폭 기반 추정
- **칼로리**: 심박 + 활동량

#### 4. 수면 분석
- **단계**: 깨어있음/얕은수면/깊은수면/REM
- **알고리즘**: 심박 변이도 (HRV) + 모션
- **리포트**: 아침에 점수 표시

#### 5. 알림
- **블루투스 연동**: 스마트폰 알림 미러링
- **진동**: 착신, 메시지, 알람
- **표시**: 발신자, 메시지 미리보기

### 배터리 수명

| 모드 | 소비전류 | 수명 |
|------|---------|------|
| **상시 표시** | 25mA | 12 시간 |
| **절전 (제스처)** | 5mA | 60 시간 |
| **Deep Sleep** | 0.5mA | 25 일 |

### 상업화 전략

#### 타겟
- 피트니스 애호가
- 건강 관리 관심 20-40 대
- 스마트워치 첫 구매자

#### 가격
- **얼리버드**: 120,000 원 (30% 할인)
- **정가**: 150,000 원
- **구독**: 월 5,000 원 (프리미엄 분석)

#### 차별화
- **오픈소스**: 펌웨어 커스터마이징
- **교육용**: 개발 문서 공개
- **커뮤니티**: 사용자 펌웨어 공유

---

## 2. 음성 건강 비서 (4 인치 보드)

### 개요
- **난이도**: 고급
- **제작 시간**: 10-12 시간
- **비용**: 120,000 원
- **완제품 목표가**: 250,000 원

### 하드웨어 구성

| 부품 | 수량 | 단가 | 비고 |
|------|------|------|------|
| ESP32-S3 4 인치 보드 | 1 개 | 45,000 원 | 메인 컨트롤러 |
| MAX30102 센서 | 1 개 | 8,000 원 | 심박/혈중산소 |
| BME680 센서 | 1 개 | 12,000 원 | 온도/습도/기압/가스 |
| 마이크 어레이 (4 개) | 1 개 | 25,000 원 | 빔포밍, 노이즈 캔슬링 |
| 스피커 (3W) | 1 개 | 10,000 원 | 8Ω, 클래스 D |
| Li-Po 배터리 (2000mAh) | 1 개 | 15,000 원 | 8 시간 구동 |
| AC 어댑터 (5V 2A) | 1 개 | 8,000 원 | 상시 전원 |
| 스탠드 (3D 프린팅) | 1 개 | 5,000 원 | 각도 조절 |
| microSD (32GB) | 1 개 | 10,000 원 | 데이터 저장 |
| **총계** | | **138,000 원** | |

### 회로 연결

```
MAX30102 → ESP32-S3 (I2C)
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8
└─ SCL → GPIO 9

BME680 → ESP32-S3 (I2C)
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8 (공유)
└─ SCL → GPIO 9 (공유)

마이크 어레이 → I2S
├─ WS → GPIO 5
├─ BCK → GPIO 6
├─ DATA → GPIO 7
└─ VCC → 3.3V

스피커 → I2S 앰프
├─ WS → GPIO 5 (공유)
├─ BCK → GPIO 6 (공유)
├─ DATA → GPIO 4
└─ VCC → 5V
```

### 펌웨어 구조

```cpp
#include <LovyanGFX.h>
#include <I2S.h>
#include <ESPSpeech.h>
#include <MAX30102.h>
#include <BME680.h>

LGFX lcd;
ESPSpeech speech;
MAX30102 heartSensor;
BME680 envSensor;

// 음성 명령
const char* commands[] = {
  "심박수 보여줘",
  "혈압 알려줘",
  "오늘 날씨 어때",
  "약 먹을 시간",
  "건강 점수 알려줘",
  " emergency",
  "안녕"
};

void setup() {
  lcd.init();
  speech.begin(2); // 듀얼 마이크
  heartSensor.begin();
  envSensor.begin();
  
  // 음성 명령 등록
  for (const char* cmd : commands) {
    speech.addCommand(cmd);
  }
  
  // 홈 화면 표시
  drawHomeScreen();
}

void loop() {
  // 음성 인식 (대기)
  String command = speech.listen();
  
  if (command != "") {
    processCommand(command);
  }
  
  // 주기적 건강 데이터 수집
  collectHealthData();
  
  delay(100);
}

void processCommand(String cmd) {
  lcd.clear();
  
  if (cmd == "심박수 보여줘") {
    int bpm = heartSensor.readHeartRate();
    int spo2 = heartSensor.readSpO2();
    
    lcd.printf("심박수: %d BPM\n", bpm);
    lcd.printf("혈중산소: %d%%\n", spo2);
    
    // 그래프 표시
    drawHeartRateGraph();
    
  } else if (cmd == "건강 점수 알려줘") {
    int score = calculateHealthScore();
    
    lcd.printf("건강 점수: %d/100\n", score);
    
    if (score >= 80) {
      lcd.print("좋음 😊");
    } else if (score >= 60) {
      lcd.print("보통 😐");
    } else {
      lcd.print("주의 😟");
    }
    
  } else if (cmd == "emergency") {
    // 긴급 호출
    callEmergency();
  }
  
  // 5 초 후 홈 화면
  delay(5000);
  drawHomeScreen();
}

int calculateHealthScore() {
  int score = 100;
  
  // 심박수 (60-100 정상)
  int bpm = heartSensor.readHeartRate();
  if (bpm < 60 || bpm > 100) score -= 20;
  
  // 혈중산소 (95% 이상 정상)
  int spo2 = heartSensor.readSpO2();
  if (spo2 < 95) score -= 20;
  
  // 환경 (온도 20-26 도)
  float temp = envSensor.readTemperature();
  if (temp < 20 || temp > 26) score -= 10;
  
  // 습도 (40-60%)
  float humidity = envSensor.readHumidity();
  if (humidity < 40 || humidity > 60) score -= 10;
  
  return score;
}

void callEmergency() {
  lcd.fillScreen(TFT_RED);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  lcd.print("긴급 호출 중...");
  
  // 블루투스로 스마트폰 연동
  // 또는 WiFi 로 VoIP 호출
  // 또는 GSM 모듈로 119 전화
}

void drawHomeScreen() {
  lcd.clear();
  
  // 시계
  time_t now = time(NULL);
  lcd.printf("%02d:%02d\n", hour(now), minute(now));
  
  // 건강 요약
  lcd.printf("심박: %d BPM\n", heartSensor.readHeartRate());
  lcd.printf("산소: %d%%\n", heartSensor.readSpO2());
  lcd.printf("온도: %.1f°C\n", envSensor.readTemperature());
  
  // 음성 인식 아이콘
  lcd.drawIcon(mic_icon, 200, 200, 60, 60, TFT_BLUE);
  lcd.print("말씀하세요...");
}
```

### 음성 인식 기능

#### 1. 빔포밍
```cpp
float beamform(int16_t* mic1, int16_t* mic2, int len) {
  // 두 마이크 신호의 시간차 보정
  // 방향성 향상 (정면 음성 강조)
  
  float output = 0;
  for (int i = 0; i < len; i++) {
    output += (mic1[i] + mic2[i]) / 2;
  }
  return output / len;
}
```

#### 2. 노이즈 캔슬링
```cpp
void noiseCancel(int16_t* primary, int16_t* reference, int len) {
  // LMS (Least Mean Square) 알고리즘
  // 주변 소음 제거
  
  float mu = 0.01; // 학습률
  float w = 0;     // 가중치
  
  for (int i = 0; i < len; i++) {
    float error = primary[i] - w * reference[i];
    w += mu * error * reference[i];
    output[i] = error;
  }
}
```

#### 3. 음성 명령 인식
- **오프라인**: 20 개 명령어 (ESP-NOW)
- **온라인**: 무제한 (WiFi → 클라우드 STT)
- **정확도**: 95% (조용한 환경), 85% (소음 환경)

### 상업화 전략

#### 타겟
- 50 대 이상 중장년층
- 만성질환자 (당뇨, 고혈압)
- 1 인 가구 노인

#### 가격
- **정가**: 250,000 원
- **구독**: 월 10,000 원 (원격 모니터링, 알림)
- **보험**: 장기요양보험 적용 가능

#### 유통
- **온라인**: 쿠팡, 11 번가
- **오프라인**: 약국, 요양원
- **B2B**: 병원, 요양병원

---

## 3. AI 수면 코치 (AMOLED 보드)

### 개요
- **난이도**: 최상급
- **제작 시간**: 15-20 시간
- **비용**: 150,000 원
- **완제품 목표가**: 350,000 원

### 하드웨어 구성

| 부품 | 수량 | 단가 | 비고 |
|------|------|------|------|
| ESP32-S3 AMOLED 2.16" 보드 | 1 개 | 55,000 원 | 메인 컨트롤러 |
| MAX30102 센서 | 1 개 | 8,000 원 | 심박/혈중산소 |
| AD8232 ECG 모듈 | 1 개 | 25,000 원 | 심전도 (1 리드) |
| MPX5010 압력 센서 | 1 개 | 15,000 원 | 혈압 측정 |
| Li-Po 배터리 (500mAh) | 1 개 | 12,000 원 | 24 시간 구동 |
| 충전 독 (Pogo pin) | 1 개 | 8,000 원 | 자기부착 |
| 실리콘 밴드 (22mm) | 1 개 | 8,000 원 | 프리미엄 |
| 3D 프린팅 케이스 | 1 개 | 15,000 원 | 방수 IP67 |
| microSD (64GB) | 1 개 | 15,000 원 | 하룻밤 데이터 |
| **총계** | | **161,000 원** | |

### 회로 연결

```
MAX30102 → ESP32-S3 (I2C)
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8
└─ SCL → GPIO 9

AD8232 (ECG) → ESP32-S3 (아날로그)
├─ VCC → 3.3V
├─ GND → GND
├─ LO+ → GPIO 1 (ADC1)
└─ LO- → GPIO 2 (ADC1)

MPX5010 (혈압) → ESP32-S3 (아날로그)
├─ VCC → 5V
├─ GND → GND
└─ OUT → GPIO 3 (ADC1)

마이크 (듀얼) → I2S
├─ WS → GPIO 5
├─ BCK → GPIO 6
├─ DATA → GPIO 7
└─ VCC → 3.3V
```

### 펌웨어 구조

```cpp
#include <AMOLED_Display.h>
#include <TensorFlowLite.h>
#include <MAX30102.h>
#include <AD8232.h>

AMOLED display;
tflite::MicroInterpreter interpreter;
MAX30102 ppgSensor;
AD8232 ecgSensor;

// 수면 단계 열거형
enum SleepStage {
  STAGE_AWAKE,      // 깨어있음
  STAGE_LIGHT,      // 얕은수면 (N1, N2)
  STAGE_DEEP,       // 깊은수면 (N3)
  STAGE_REM         // 렘수면
};

// 수면 데이터
struct SleepData {
  time_t startTime;
  time_t endTime;
  SleepStage stages[360]; // 1 분 간격 6 시간
  int awakeCount;
  int apneaCount;
  float avgHeartRate;
  float avgSpO2;
};

void setup() {
  display.init();
  ppgSensor.begin();
  ecgSensor.begin();
  
  // TensorFlow Lite 모델 로드
  loadSleepModel();
  
  // 수면 모드 진입
  enterSleepMode();
}

void loop() {
  SleepData sleepData;
  
  // 하룻밤 데이터 수집
  collectSleepData(&sleepData);
  
  // 아침 리포트 생성
  generateSleepReport(&sleepData);
  
  // 충전 독으로 데이터 전송
  syncToPhone(&sleepData);
}

void collectSleepData(SleepData* data) {
  int minute = 0;
  
  while (minute < 360) { // 6 시간
    // 1 분간 데이터 수집
    uint32_t heartRates[60];
    uint32_t spo2Values[60];
    float ecgSamples[250]; // 250Hz 1 초
    
    for (int i = 0; i < 60; i++) {
      heartRates[i] = ppgSensor.readHeartRate();
      spo2Values[i] = ppgSensor.readSpO2();
      delay(1000);
    }
    
    // 심박 변이도 (HRV) 계산
    float hrv = calculateHRV(heartRates, 60);
    
    // 호흡 패턴 분석 (마이크)
    float breathRate = analyzeBreath();
    
    // 코골이 감지 (마이크)
    bool snoring = detectSnoring();
    
    // 무호흡 감지
    bool apnea = detectApnea(breathRate, spo2Values, 60);
    
    // 수면 단계 분류 (TensorFlow Lite)
    float inputFeatures[5] = {
      (float)minute / 360,    // 시간 (정규화)
      calculateAvg(heartRates, 60),  // 평균 심박
      hrv,                    // HRV
      breathRate,             // 호흡수
      calculateAvg(spo2Values, 60)   // 평균 SpO2
    };
    
    SleepStage stage = classifySleepStage(inputFeatures);
    data->stages[minute] = stage;
    
    if (apnea) data->apneaCount++;
    if (stage == STAGE_AWAKE) data->awakeCount++;
    
    minute++;
  }
}

SleepStage classifySleepStage(float* features) {
  // 입력 데이터 설정
  interpreter.input(0)->data.f = features;
  
  // 추론
  interpreter.Invoke();
  
  // 출력: [awake, light, deep, rem] 확률
  float* output = interpreter.output(0)->data.f;
  
  // 최대 확률 단계 반환
  return (SleepStage)argmax(output, 4);
}

void generateSleepReport(SleepData* data) {
  display.clear();
  
  // 수면 점수 (0-100)
  int score = calculateSleepScore(data);
  
  // 점수 표시 (AMOLED)
  drawSleepScore(score);
  
  // 수면 단계 그래프
  drawSleepHypnogram(data);
  
  // 통계
  display.printf("총 수면: %d 시간 %d 분\n", 
                 data->duration / 3600, 
                 (data->duration % 3600) / 60);
  display.printf("깊은수면: %d%%\n", 
                 calculateDeepSleepPercent(data));
  display.printf("무호흡: %d 회\n", data->apneaCount);
  
  // 코멘트
  if (score >= 80) {
    display.print("훌륭한 수면! 😊");
  } else if (score >= 60) {
    display.print("보통이에요 😐");
  } else {
    display.print("수면 개선 필요 😟");
  }
}

int calculateSleepScore(SleepData* data) {
  int score = 100;
  
  // 수면 효율 (잠든 시간 / 침대에 있는 시간)
  float efficiency = (360 - data->awakeCount) / 360.0;
  if (efficiency < 0.85) score -= 20;
  
  // 깊은수면 비율 (15-25% 정상)
  int deepPercent = calculateDeepSleepPercent(data);
  if (deepPercent < 15 || deepPercent > 25) score -= 20;
  
  // 무호흡 횟수 (5 회 미만 정상)
  if (data->apneaCount > 5) score -= 20;
  
  // 수면 시간 (7-9 시간 정상)
  if (data->duration < 7*3600 || data->duration > 9*3600) score -= 10;
  
  return score;
}

void enterSleepMode() {
  // AMOLED 끄기 (절전)
  display.sleep();
  
  // 센서 저전력 모드
  ppgSensor.setLowPower(true);
  
  // ESP32 Deep Sleep (1 분마다 웨이크)
  esp_sleep_enable_timer_wakeup(60 * 1000000);
  esp_deep_sleep_start();
}
```

### TensorFlow Lite 모델

#### 모델 아키텍처
```
입력: [5] (시간, 심박, HRV, 호흡수, SpO2)
├─ Dense (16 neurons, ReLU)
├─ Dense (8 neurons, ReLU)
└─ 출력: [4] (Softmax) → [Awake, Light, Deep, REM]

모델 크기: 2KB
RAM 사용: 4KB
추론 시간: 30ms (ESP32-S3)
```

#### 학습 데이터
- **데이터셋**: PhysioNet Sleep-EDF (200 명)
- **특징**: 심박, HRV, 호흡, SpO2, 모션
- **정확도**: 82% (4 단계 분류)

### 상업화 전략

#### 타겟
- 수면 장애 환자 (불면증, 수면 무호흡)
- 바이오해커 (수면 최적화)
- 시프트 워커 (교대 근무자)

#### 가격
- **정가**: 350,000 원
- **구독**: 월 15,000 원 (AI 코칭, 의료진 상담)
- **보험**: 수면클리닉 처방 시 일부 환급

#### 유통
- **온라인**: 와디즈 크라우드펀딩
- **오프라인**: 수면클리닉, 이비인후과
- **해외**: 미국 (FDA Class II), 유럽 (CE)

---

## 비교 표

| 항목 | 스마트 워치 | 음성 건강 비서 | AI 수면 코치 |
|------|-----------|--------------|-------------|
| **보드** | 1.28" 원형 | 4 인치 | 2.16" AMOLED |
| **난이도** | 중급 | 고급 | 최상급 |
| **제작시간** | 6-8h | 10-12h | 15-20h |
| **비용** | 7 만원 | 12 만원 | 15 만원 |
| **목표가** | 15 만원 | 25 만원 | 35 만원 |
| **배터리** | 12-60h | 8h (상시) | 24h |
| **방수** | IP54 | 없음 | IP67 |
| **AI** | 기본 | 음성 인식 | TensorFlow Lite |
| **인증** | 불필요 | Class I | Class II |

---

## 개발 로드맵

### Phase 1: 프로토타입 (2 개월)
- [ ] 하드웨어 설계 (회로도, PCB)
- [ ] 3D 케이스 디자인
- [ ] 펌웨어 MVP (기본 기능)
- [ ] 10 명 사용자 테스트

### Phase 2: 고도화 (2 개월)
- [ ] 펌웨어 완성 (모든 기능)
- [ ] 모바일 앱 개발 (BLE 연동)
- [ ] 클라우드 백엔드 (데이터 저장)
- [ ] 100 명 베타 테스트

### Phase 3: 인증 (3 개월)
- [ ] 의료기기 인증 (Class I/II)
- [ ] 전기안전 인증 (KC)
- [ ] 무선 인증 (KC, FCC, CE)
- [ ] 품질 관리 시스템 (ISO 13485)

### Phase 4: 출시 (1 개월)
- [ ] 크라우드펀딩
- [ ] 마케팅 (YouTube, 블로그)
- [ ] 유통 채널 구축
- [ ] 고객 지원 체계

---

## 다음 단계

1. **보드 선택**: 3 종 중 타겟에 맞는 보드 선택
2. **센서 소싱**: AliExpress, DigiKey, Mouser
3. **케이스 디자인**: Fusion 360, Onshape
4. **펌웨어 개발**: Arduino IDE, PlatformIO
5. **사용자 테스트**: 10 명 모집, 피드백
6. **고도화**: PCB 설계, 사출 금형

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**자문**: 웨어러블 개발자, 의료기기 엔지니어
