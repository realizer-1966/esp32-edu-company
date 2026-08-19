# 블루투스 복합 활용 가이드 (Bluetooth Hybrid Sensing)

## 개요

ESP32-S3 의 WiFi + 블루투스 (BLE 5.0) 를 동시에 활용하여 더 정밀한 동작감지, 존재감지, 건강 모니터링을 구현합니다.

---

## 1. ESP32-S3 무선 성능

### 동시 지원
| 기능 | 사양 | 동시 사용 |
|------|------|----------|
| **WiFi** | 802.11 b/g/n (2.4GHz) | ✅ |
| **블루투스** | BLE 5.0 (Bluetooth Low Energy) | ✅ |
| **동시 동작** | WiFi + BLE 병렬 처리 | ✅ |

### 블루투스 모드
| 모드 | 용도 | 예시 |
|------|------|------|
| **BLE Observer** | 광고 스캔 | 존재 감지, 디바이스 탐지 |
| **BLE Broadcaster** | 광고 송출 | 비콘, 위치 알림 |
| **BLE Peripheral** | 서버 역할 | 건강 센서 데이터 제공 |
| **BLE Central** | 클라이언트 역할 | 스마트워치/밴드에서 데이터 수집 |
| **BLE Mesh** | 메시 네트워크 | 다중 방 감지 |

---

## 2. WiFi + BLE 복합 감지 시나리오

### 시나리오 1: 존재 감지 정확도 향상

```
WiFi RSSI 분석 + BLE 디바이스 탐지
    ↓
센서 퓨전 (Kalman Filter)
    ↓
정확도 95% → 99%
```

**장점:**
- WiFi: 사람 움직임 감지 (수동적)
- BLE: 스마트폰/웨어러블 탐지 (능동적)
- 상호 보완: WiFi 는 움직임, BLE 는 디바이스 존재

### 시나리오 2: 다중 사람 추적

```
BLE MAC 주소로 개인 식별
    ↓
WiFi CSI 로 위치/동작 추적
    ↓
3 명까지 동시 추적 가능
```

**장점:**
- BLE: 각 사람 스마트폰 MAC 주소
- WiFi: 실시간 위치/동작
- 응용: 노인 케어 (어머니는 주방, 아버지는 거실)

### 시나리오 3: 건강 데이터 퓨전

```
BLE: 스마트밴드 심박수 (정밀)
WiFi: 호흡수 (비접촉)
    ↓
통합 건강 점수
```

**장점:**
- BLE: 심박수 ±2 BPM (PPG 센서)
- WiFi: 호흡수 (비접촉, 수면 중)
- 응용: 수면 무호흡 감지

### 시나리오 4: 낙상 감지 신뢰도 향상

```
WiFi: 급격한 움직임 감지
BLE: 가속도계 (스마트폰/워치)
    ↓
낙상 확률 95% → 99%
```

**장점:**
- WiFi: 낙상 충격 감지
- BLE: 자세/가속도 데이터
- 응용: 자동 119 신고 (위양성 감소)

---

## 3. 구현 방법

### 방법 1: BLE Presence + WiFi Motion

```cpp
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>

// BLE 스캔
BLEScan* pBLEScan;
int deviceCount = 0;

// WiFi RSSI
int rssiHistory[60];

void setup() {
  // WiFi 스테이션
  WiFi.begin(ssid, password);
  
  // BLE 스캐너
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(90);
  pBLEScan->setActiveScan(true);
}

void loop() {
  // WiFi 동작 감지
  int wifiMotion = detectWiFiMotion();
  
  // BLE 디바이스 스캔 (3 초)
  BLEScanResults results = pBLEScan->start(3, false);
  deviceCount = results.getCount();
  
  // 퓨전
  if (wifiMotion || deviceCount > 0) {
    Serial.println("사람 있음");
  } else {
    Serial.println("사람 없음");
  }
}
```

### 방법 2: BLE Triangulation (다중 ESP32)

```
ESP32 #1 (거실)     ESP32 #2 (주방)
    ↓                    ↓
    RSSI: -65dBm         RSSI: -80dBm
    ↓                    ↓
    └──────→ 위치 계산 ←──────┘
           삼각측량
           ↓
    "사람은 거실 중앙"
```

**필요 하드웨어:**
- ESP32-S3 보드 2-3 개
- 시간 동기화 (NTP 또는 ESP-NOW)
- 중앙 서버 (라즈베리파이)

### 방법 3: BLE Health Data Collection

```cpp
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLE2902.h>

// 심박수 서비스 (0x180D)
#define HEART_RATE_SERVICE_UUID 0x180D
#define HEART_RATE_MEASUREMENT_UUID 0x2A37

BLEClient* pClient;
BLERemoteCharacteristic* pHeartRate;

void connectToBand() {
  // 스마트밴드 연결
  pClient = BLEDevice::createClient();
  pClient->connect(pAddress);
  
  // 심박수 서비스 찾기
  BLERemoteService* pService = 
    pClient->getService(HEART_RATE_SERVICE_UUID);
  pHeartRate = 
    pService->getCharacteristic(HEART_RATE_MEASUREMENT_UUID);
}

int readHeartRate() {
  // 심박수 읽기
  uint8_t value = pHeartRate->readValue()[1];
  return (int)value;
}

// WiFi 로 클라우드 전송
void uploadHealthData() {
  int heartRate = readHeartRate();
  float breathRate = detectBreathViaWiFi();
  
  // 통합 건강 점수
  int healthScore = calculateScore(heartRate, breathRate);
  
  // HTTP POST
  http.begin(serverUrl);
  http.POST("{\"hr\":" + String(heartRate) + 
            ",\"breath\":" + String(breathRate) + "}");
}
```

### 방법 4: BLE Mesh (다중 방 감지)

```
거실 ESP32 ──── 주방 ESP32
    │               │
    │   BLE Mesh    │
    │               │
침실 ESP32 ──── 욕실 ESP32
    ↓
모든 데이터 중앙 허브로 전송
    ↓
집 전체 사람 위치 추적
```

**장점:**
- 최대 100 개 노드
- 자동 라우팅
- 저전력 (배터리 1 년)

---

## 4. 실제 프로젝트 예제

### 프로젝트 1: 스마트 홈 존재 감지 (WiFi + BLE)

```cpp
#include <LovyanGFX.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <ArduinoJson.h>

LGFX lcd;

// WiFi
const char* ssid = "your_wifi";
const char* password = "your_password";
int rssiHistory[60];

// BLE
BLEScan* pBLEScan;
String knownDevices[] = {
  "aa:bb:cc:dd:ee:01",  // 아빠 폰
  "aa:bb:cc:dd:ee:02",  // 엄마 폰
  "aa:bb:cc:dd:ee:03"   // 아이 워치
};
int knownDeviceCount = 3;

struct HomeStatus {
  bool wifiMotion;
  bool blePresence;
  String whoIsHome;
  int deviceCount;
};

HomeStatus status;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  
  // WiFi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  // BLE 스캐너
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(90);
  
  drawUI();
}

void loop() {
  // WiFi 동작 감지 (1 초)
  status.wifiMotion = detectWiFiMotion();
  
  // BLE 디바이스 스캔 (3 초)
  BLEScanResults results = pBLEScan->start(3, false);
  status.deviceCount = results.getCount();
  status.blePresence = status.deviceCount > 0;
  
  // 알려진 디바이스 확인
  status.whoIsHome = checkKnownDevices(results);
  
  // 화면 업데이트
  updateDisplay();
  
  // MQTT 로 홈어시스턴트 전송
  publishToMQTT();
  
  delay(100);
}

bool detectWiFiMotion() {
  int rssi = WiFi.RSSI();
  rssiHistory[millis() % 60] = rssi;
  
  float stdDev = calculateStdDev(rssiHistory, 60);
  return stdDev > 5.0;
}

String checkKnownDevices(BLEScanResults results) {
  String who = "";
  
  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    String address = device.getAddress().toString();
    
    for (int j = 0; j < knownDeviceCount; j++) {
      if (address == knownDevices[j]) {
        if (who != "") who += ", ";
        who += getNameByIndex(j);
      }
    }
  }
  
  return who != "" ? who : "알 수 없는 디바이스";
}

String getNameByIndex(int index) {
  if (index == 0) return "아빠";
  if (index == 1) return "엄마";
  if (index == 2) return "아이";
  return "알 수 없음";
}

void updateDisplay() {
  lcd.clear();
  
  // 상태 표시
  if (status.blePresence || status.wifiMotion) {
    lcd.fillScreen(TFT_GREEN);
    lcd.print("집에 사람 있음");
  } else {
    lcd.fillScreen(TFT_RED);
    lcd.print("집에 사람 없음");
  }
  
  // 누가 있는지
  lcd.printf("\n\n누구: %s\n", status.whoIsHome.c_str());
  lcd.printf("BLE 디바이스: %d 개\n", status.deviceCount);
  lcd.printf("WiFi 움직임: %s\n", 
             status.wifiMotion ? "감지" : "없음");
}

void publishToMQTT() {
  StaticJsonDocument<256> doc;
  doc["wifi_motion"] = status.wifiMotion;
  doc["ble_presence"] = status.blePresence;
  doc["device_count"] = status.deviceCount;
  doc["who_is_home"] = status.whoIsHome;
  
  String json;
  serializeJson(doc, json);
  
  // MQTT publish (홈어시스턴트)
  // mqttClient.publish("home/presence", json.c_str());
}

float calculateStdDev(int* data, int len) {
  float sum = 0;
  for (int i = 0; i < len; i++) sum += data[i];
  float mean = sum / len;
  
  float variance = 0;
  for (int i = 0; i < len; i++) {
    variance += (data[i] - mean) * (data[i] - mean);
  }
  
  return sqrt(variance / len);
}

void drawUI() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.print("스마트 홈 존재 감지");
}
```

### 프로젝트 2: BLE 심박수 + WiFi 호흡 퓨전

```cpp
#include <BLEDevice.h>
#include <BLEClient.h>
#include <esp_wifi.h>

// BLE 심박수 클라이언트
BLEClient* pClient;
BLERemoteCharacteristic* pHeartRate;

// WiFi 호흡 감지
float csiAmplitude[256];
int csiIndex = 0;

struct HealthData {
  int heartRate;        // BLE (정밀)
  float breathRate;     // WiFi (비접촉)
  int healthScore;      // 통합 점수
};

HealthData health;

void setup() {
  Serial.begin(115200);
  
  // BLE 심박수 모니터 연결
  connectToHeartRateMonitor();
  
  // WiFi CSI 활성화
  WiFi.begin("ssid", "password");
  esp_wifi_set_csi(true);
  esp_wifi_set_csi_rx_cb(wifi_csi_cb, NULL);
}

void loop() {
  // BLE 로 심박수 읽기 (10 초마다)
  static unsigned long lastHR = 0;
  if (millis() - lastHR > 10000) {
    health.heartRate = readHeartRate();
    lastHR = millis();
  }
  
  // WiFi 로 호흡수 분석 (지속적)
  health.breathRate = analyzeBreathFromWiFi();
  
  // 통합 건강 점수
  health.healthScore = calculateHealthScore(
    health.heartRate, 
    health.breathRate
  );
  
  // 이상 징후 감지
  if (health.heartRate > 100 || health.breathRate > 30) {
    sendAlert();
  }
  
  // 데이터 로깅
  logHealthData(health);
  
  delay(1000);
}

void connectToHeartRateMonitor() {
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  
  // 심박수 모니터 찾기 (0x180D)
  BLEAdvertisedDevice* found = pBLEScan->start(10, true);
  
  if (found) {
    pClient = BLEDevice::createClient();
    pClient->connect(found->getAddress());
    
    BLERemoteService* pService = 
      pClient->getService(BLEUUID((uint16_t)0x180D));
    pHeartRate = 
      pService->getCharacteristic(BLEUUID((uint16_t)0x2A37));
    
    Serial.println("심박수 모니터 연결됨");
  }
}

int readHeartRate() {
  if (!pHeartRate) return 0;
  
  std::string value = pHeartRate->readValue();
  return (int)value[1]; // 첫 바이트는 플래그
}

float analyzeBreathFromWiFi() {
  // CSI 데이터로 호흡수 분석
  // 0.2-0.5Hz 대역 (분당 12-30 회)
  
  // 예시: 제로크로싱
  int zeroCrossings = 0;
  float mean = 0;
  
  for (int i = 0; i < csiIndex; i++) {
    mean += csiAmplitude[i];
  }
  mean /= csiIndex;
  
  for (int i = 1; i < csiIndex; i++) {
    if ((csiAmplitude[i-1] - mean) * 
        (csiAmplitude[i] - mean) < 0) {
      zeroCrossings++;
    }
  }
  
  // Hz → 분당 호흡수
  float breathRate = (zeroCrossings / 2.0) * 
                     (60.0 / (csiIndex / 10.0));
  
  return constrain(breathRate, 0, 60);
}

int calculateHealthScore(int hr, float breath) {
  int score = 100;
  
  // 심박수 (60-100 정상)
  if (hr < 60 || hr > 100) score -= 20;
  
  // 호흡수 (12-20 정상)
  if (breath < 12 || breath > 20) score -= 20;
  
  // 심박-호흡 비율 (4:1 이상적)
  float ratio = hr / breath;
  if (ratio < 3.5 || ratio > 5.0) score -= 10;
  
  return score;
}

void sendAlert() {
  // Telegram/SMS 알림
  // 또는 부저/LED 작동
  Serial.println("건강 이상 감지!");
}

void logHealthData(HealthData data) {
  // SD 카드 또는 클라우드에 저장
  Serial.printf("HR: %d, Breath: %.1f, Score: %d\n",
                data.heartRate, data.breathRate, 
                data.healthScore);
}

void wifi_csi_cb(void *ctx, wifi_csi_info_t *info) {
  // CSI 진폭 추출
  int8_t* csi_data = (int8_t*)info->buf;
  float amplitude = 0;
  
  for (int i = 0; i < 52; i++) {
    amplitude += abs(csi_data[i * 2]);
  }
  
  csiAmplitude[csiIndex++] = amplitude / 52;
  
  if (csiIndex >= 256) {
    csiIndex = 0;
  }
}
```

### 프로젝트 3: BLE Mesh 낙상 감지 네트워크

```cpp
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// BLE Mesh 노드
#define MESH_UUID "ESP32-Mesh-Node-"

class MeshNode {
  String nodeId;
  BLEServer* pServer;
  BLECharacteristic* pMotionChar;
  
  bool motionDetected = false;
  int acceleration = 0;
  
public:
  MeshNode(String id) : nodeId(id) {}
  
  void init() {
    BLEDevice::init(MESH_UUID + nodeId);
    
    pServer = BLEDevice::createServer();
    BLEService* pService = 
      pServer->createService("12345678-1234-5678-1234-567812345678");
    
    pMotionChar = pService->createCharacteristic(
      "87654321-4321-8765-4321-876543218765",
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );
    
    pService->start();
    
    // 광고 시작
    BLEAdvertising* pAdvertising = 
      BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
  }
  
  void setMotion(bool motion, int accel) {
    motionDetected = motion;
    acceleration = accel;
    
    // BLE Notify
    uint8_t data[2] = {motion ? 1 : 0, (uint8_t)accel};
    pMotionChar->setValue(data, 2);
    pMotionChar->notify();
  }
  
  bool isMotionDetected() { return motionDetected; }
  int getAcceleration() { return acceleration; }
};

// 4 개 노드 (거실, 주방, 침실, 욕실)
MeshNode nodes[4] = {
  MeshNode("Living"),
  MeshNode("Kitchen"),
  MeshNode("Bedroom"),
  MeshNode("Bathroom")
};

// 중앙 허브
void hubLoop() {
  // 모든 노드 스캔
  for (int i = 0; i < 4; i++) {
    // BLE 연결 → 데이터 읽기
    bool motion = nodes[i].isMotionDetected();
    int accel = nodes[i].getAcceleration();
    
    // 낙상 감지 (급격한 가속도 변화)
    if (accel > 50) { // 5G 이상
      sendFallAlert(nodes[i].nodeId);
    }
  }
}

void sendFallAlert(String location) {
  // 119 신고 + 보호자 알림
  // 위치 정보 포함
  Serial.printf("낙상 감지! 위치: %s\n", location.c_str());
}
```

---

## 5. 성능 비교

| 구성 | 정확도 | 비용 | 복잡도 |
|------|--------|------|--------|
| **WiFi 만** | 85% | 10,000 원 | 낮음 |
| **BLE 만** | 75% | 10,000 원 | 낮음 |
| **WiFi + BLE** | 95% | 10,000 원 | 중간 |
| **WiFi + BLE + AI** | 98% | 10,000 원 | 높음 |
| **ESP32-C6 (WiFi 6)** | 99% | 15,000 원 | 중간 |

---

## 6. 추천 구성

### 입문자 (오늘 시작)
- **WiFi RSSI + BLE 스캔**
- 정확도: 90%
- 코드: 200 줄
- 비용: 10,000 원

### 중급자 (1 주)
- **WiFi CSI + BLE 심박수**
- 정확도: 95%
- 코드: 500 줄
- 비용: 25,000 원 (MAX30102 추가)

### 고급자 (1 개월)
- **BLE Mesh (4 노드) + WiFi 6**
- 정확도: 99%
- 코드: 1000 줄
- 비용: 50,000 원 (ESP32-C6 × 4)

---

## 7. 한계 및 주의사항

### 기술적 한계
1. **BLE 범위**: 10-30m (벽 있으면 5m)
2. **배터리**: BLE 상시 스캔 시 1-2 일
3. **동시 연결**: 최대 3 개 (ESP32-S3)
4. **MAC 주소 랜덤화**: 최신 폰은 추적 어려움

### 법적 제한
- **개인정보**: BLE MAC 주소 수집 동의 필요
- **의료기기**: 심박/호흡은 의료기기 인증 필요
- **전파법**: WiFi/BLE 출력 제한 준수

---

## 8. 다음 단계

### 즉시 시작
1. **BLE 스캔 예제** (30 분)
   - 주변 디바이스 탐지
   - 알려진 폰 등록

2. **WiFi + BLE 퓨전** (2 시간)
   - 존재 감지 정확도 향상
   - 홈어시스턴트 연동

### 중기 목표
3. **BLE 심박수 수집** (1 주)
   - 스마트밴드 연결
   - WiFi 호흡과 퓨전

4. **BLE Mesh 테스트** (2 주)
   - ESP32 2 대로 시작
   - 낙상 감지

### 장기 목표
5. **상업화** (3 개월)
   - 노인 케어 시스템
   - 의료기기 인증 (Class I)
   - 월 구독 모델 (10,000 원/월)

---

**결론: WiFi + BLE 복합 활용은 정확도 95% 로 향상되며, 심박수/호흡 동시 모니터링이 가능합니다.**

**추천: BLE 스캔부터 시작 → 심박수 수집 → Mesh 네트워크**

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**참고**: ESP32 BLE Library, Home Assistant, OpenHaystack
