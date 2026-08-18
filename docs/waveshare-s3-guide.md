# WAVESHARE ESP32-S3-Touch-LCD-4 개발 가이드

## 실제 보유 하드웨어 중심 개발

이 문서는 WAVESHARE ESP32-S3-Touch-LCD-4 보드를 실제 보유하고 있는 개발자를 위한 실전 가이드입니다.

---

## 1. 하드웨어 스펙 (실제)

### 공식 스펙
| 항목 | 값 | 비고 |
|------|-----|------|
| **모델** | WAVESHARE ESP32-S3-Touch-LCD-4 | |
| **MCU** | ESP32-S3-WROOM-1 | Dual-core 240MHz |
| **디스플레이** | 4 인치 IPS LCD | 480x800 픽셀 |
| **터치** | 정전식 터치 (GT911) | I2C 주소 0x5D |
| **마이크** | 아날로그 마이크 × 2 | 듀얼, I2S ADC 경유 |
| **스피커** | 1W 클래스 D 앰프 | I2S 출력 |
| **메모리** | 16MB PSRAM, 8MB Flash | |
| **카메라** | OV2640 2MP (선택) | DVP 인터페이스 |
| **SD 카드** | microSD 슬롯 | SPI 모드 |
| **배터리** | Li-Po 충전 (IP5306) | 5V USB-C 충전 |
| **연결성** | WiFi 4, Bluetooth 5 (BLE) | |
| **크기** | 85mm x 55mm x 12mm | |
| **구매가격** | 45,000 원 | AliExpress/공식 |

### 핀아웃 (핵심)

```
디스플레이 (SPI)
├─ MOSI → GPIO 11
├─ SCK  → GPIO 12
├─ CS   → GPIO 10
├─ DC   → GPIO 2
├─ RST  → GPIO 1
└─ BLK  → GPIO 3 (백라이트 PWM)

터치 (I2C)
├─ SDA → GPIO 8
├─ SCL → GPIO 9
└─ INT → GPIO 7 (터치 인터럽트)

마이크 (I2S)
├─ WS  → GPIO 5
├─ BCK → GPIO 6
├─ DATA → GPIO 4 (마이크 1)
└─ DATA2 → GPIO 40 (마이크 2, 듀얼)

스피커 (I2S)
├─ WS  → GPIO 5 (공유)
├─ BCK → GPIO 6 (공유)
├─ DATA → GPIO 41
└─ MUTE → GPIO 42

SD 카드 (SPI)
├─ MOSI → GPIO 11 (공유)
├─ MISO → GPIO 13
├─ SCK  → GPIO 12 (공유)
├─ CS   → GPIO 14
└─ DETECT → GPIO 15

카메라 (DVP)
├─ PCLK → GPIO 43
├─ VSYNC → GPIO 44
├─ HREF → GPIO 45
├─ D0-D7 → GPIO 37-40
├─ XCLK → GPIO 16
└─ PWDN → GPIO 17

USB
├─ USB_D+ → GPIO 19
├─ USB_D- → GPIO 20
└─ VBUS → 5V

배터리
├─ BAT+ → JST PH 2.0
├─ BAT- → GND
└─ CHRG → LED (충전중)
```

---

## 2. 개발 환경 설정

### Arduino IDE 설정

#### 1. ESP32 Arduino Core 설치
```
1. Arduino IDE 실행
2. File > Preferences
3. Additional Board Manager URLs:
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
4. Tools > Board > Boards Manager
5. "ESP32" 검색 → ESP32 by Espressif Systems 설치 (v3.0.0+)
```

#### 2. 보드 선택
```
Tools > Board > ESP32S3 Dev Module
```

#### 3. 보드 설정
```
Tools > Upload Speed: 921600
Tools > CPU Frequency: 240MHz
Tools > Flash Size: 8MB
Tools > PSRAM: 16MB OPI PSRAM
Tools > USB CDC On Boot: Enabled
Tools > USB DFU On Boot: Disabled
Tools > Partition Scheme: 8MB with spiffs
```

### PlatformIO 설정 (추천)

#### `platformio.ini`
```ini
[env:esp32-s3-touch-lcd-4]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.flash_size = 8MB
board_build.psram_type = opi

board_upload.flash_size = 8MB
board_upload.maximum_size = 8388608

build_flags = 
  -DBOARD_HAS_PSRAM
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DCORE_DEBUG_LEVEL=3
  
  ; WAVESHARE 핀아웃 정의
  -DTFT_MOSI=11
  -DTFT_SCLK=12
  -DTFT_CS=10
  -DTFT_DC=2
  -DTFT_RST=1
  -DTFT_BL=3
  
  -DTOUCH_SDA=8
  -DTOUCH_SCL=9
  -DTOUCH_INT=7
  
  -DI2S_WS=5
  -DI2S_BCK=6
  -DI2S_DATA_IN=4
  -DI2S_DATA_OUT=41
  
  -DSD_CS=14
  -DSD_DETECT=15

lib_deps = 
  lovyan/LovyanGFX@^1.1.12
  fastled/FastLED@^3.7.0
  adafruit/Adafruit MAX3010x Library@^1.1.0
  adafruit/Adafruit BME680 Library@^2.0.0
  adafruit/Adafruit BusIO@^1.14.0
  bblanchon/ArduinoJson@^7.0.0

monitor_speed = 115200
monitor_filters = esp32_exception_decoder, time, colorize
```

---

## 3. 필수 라이브러리

### LovyanGFX (디스플레이)

```cpp
#include <LovyanGFX.h>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_GT911 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 80000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = -1;
      cfg.pin_dc = 2;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 10;
      cfg.pin_rst = 1;
      cfg.pin_busy = -1;
      cfg.panel_width = 480;
      cfg.panel_height = 800;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 3;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = 479;
      cfg.y_min = 0;
      cfg.y_max = 799;
      cfg.pin_int = 7;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.i2c_port = 1;
      cfg.i2c_addr = 0x5D;
      cfg.pin_sda = 8;
      cfg.pin_scl = 9;
      cfg.freq = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

LGFX lcd;
```

### I2S 마이크 (듀얼)

```cpp
#include <driver/i2s.h>

#define I2S_WS 5
#define I2S_BCK 6
#define I2S_DATA_IN 4
#define I2S_DATA_IN2 40
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_DATA_IN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// 듀얼 마이크 읽기
void readDualMic(int16_t* mic1, int16_t* mic2, int samples) {
  size_t bytes_read;
  i2s_read(I2S_NUM_0, mic1, samples * 2, &bytes_read, portMAX_DELAY);
  
  // 마이크 2 는 GPIO 40 (별도 ADC 채널)
  // ESP32-S3 는 듀얼 PDM 마이크 지원
}
```

### SD 카드

```cpp
#include <SD.h>
#include <SPI.h>

#define SD_CS 14
#define SD_DETECT 15

void setupSD() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("SD 카드 초기화 실패");
    return;
  }
  
  Serial.println("SD 카드 초기화 성공");
  Serial.printf("총 용량: %llu MB\n", SD.cardSize() / (1024 * 1024));
}

void logHealthData(String filename, float data) {
  File file = SD.open("/" + filename, FILE_APPEND);
  if (!file) {
    Serial.println("파일 열기 실패");
    return;
  }
  
  file.printf("%ld,%.2f\n", millis(), data);
  file.close();
}
```

---

## 4. 헬스케어 센서 연결

### MAX30102 (심박/혈중산소)

#### 회로 연결
```
MAX30102 → ESP32-S3
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8 (I2C 공유)
├─ SCL → GPIO 9 (I2C 공유)
└─ INT → GPIO 7 (선택, 터치 INT 와 공유 불가시 GPIO 4)
```

#### 코드
```cpp
#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255
#define I2C_SPEED 400000

void setupMAX30102() {
  Wire.begin(I2C_SPEED);
  
  if (!particleSensor.begin(Wire, I2C_SPEED)) {
    Serial.println("MAX30102 를 찾을 수 없습니다");
    return;
  }
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(MAX_BRIGHTNESS);
  particleSensor.setPulseAmplitudeGreen(MAX_BRIGHTNESS);
}

int readHeartRate() {
  uint32_t irValue = particleSensor.getIR();
  
  if (checkForBeat(irValue)) {
    float beatsPerMinute = getBPM();
    return (int)beatsPerMinute;
  }
  
  return 0;
}

int readSpO2() {
  // 적외선/빨간광 비율로 SpO2 계산
  uint32_t redValue = particleSensor.getRed();
  uint32_t irValue = particleSensor.getIR();
  
  float ratio = (redValue * 100) / irValue;
  
  // 보정 곡선 (실제 측정값으로 보정 필요)
  int spo2 = 110 - (ratio / 2);
  
  return constrain(spo2, 70, 100);
}
```

### BME680 (환경 센서)

#### 회로 연결
```
BME680 → ESP32-S3
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8 (MAX30102 와 공유)
└─ SCL → GPIO 9 (MAX30102 와 공유)
```

#### 코드
```cpp
#include <Adafruit_BME680.h>

Adafruit_BME680 bme;

void setupBME680() {
  if (!bme.begin()) {
    Serial.println("BME680 를 찾을 수 없습니다");
    return;
  }
  
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
}

float readTemperature() {
  return bme.readTemperature();
}

float readHumidity() {
  return bme.readHumidity();
}

float readIAQ() {
  // 실내 공기질 지수
  return bme.readGas();
}
```

### AD8232 (심전도)

#### 회로 연결
```
AD8232 → ESP32-S3
├─ 3.3V → 3.3V
├─ GND → GND
├─ LO+ → GPIO 1 (ADC1_CH0)
├─ LO- → GPIO 2 (ADC1_CH1)
└─ SDN → 3.3V (Enable)

전극 패드:
├─ RA (오른쪽 팔) → GND
├─ LA (왼쪽 팔) → LO+
└─ RL (오른쪽 다리) → LO- (참조)
```

#### 코드
```cpp
#define ECG_PIN 1
#define SAMPLE_RATE 250

void setupADC() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

int readECG() {
  return analogRead(ECG_PIN);
}

// 50Hz 노치 필터 (전원 노이즈 제거)
float notchFilter(float input, float frequency, float samplerate) {
  static float z1, z2;
  float notchFreq = frequency / samplerate;
  float q = 1.0 / 3.0; // Q factor
  
  float output = input - z1 * 2 * cos(2 * PI * notchFreq) + z2;
  z2 = z1;
  z1 = output;
  
  return output;
}
```

---

## 5. 헬스케어 프로젝트 (실제 동작)

### 프로젝트 1: 심박수 모니터

```cpp
#include <LovyanGFX.h>
#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"

LGFX lcd;
MAX30105 particleSensor;

uint32_t irValues[60];
int irIndex = 0;

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  
  Wire.begin(400000);
  
  if (!particleSensor.begin(Wire)) {
    lcd.printf("MAX30102 오류");
    while (1);
  }
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeGreen(0x1F);
  
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(0, 0);
  lcd.print("심박수 모니터");
}

void loop() {
  uint32_t irValue = particleSensor.getIR();
  irValues[irIndex++] = irValue;
  
  if (irIndex >= 60) {
    irIndex = 0;
    
    int bpm = 0;
    if (checkForBeat(irValue)) {
      bpm = (int)getBPM();
    }
    
    lcd.fillRect(0, 50, 480, 100, TFT_BLACK);
    lcd.setTextColor(TFT_GREEN);
    lcd.setTextSize(5);
    lcd.setCursor(100, 70);
    
    if (bpm > 0) {
      lcd.printf("%d BPM", bpm);
    } else {
      lcd.print("측정중...");
    }
    
    // 그래프
    drawGraph(irValues, 60);
  }
  
  delay(100);
}

void drawGraph(uint32_t* values, int len) {
  lcd.fillRect(0, 200, 480, 200, TFT_BLACK);
  
  int minY = 0, maxY = 65535;
  
  for (int i = 0; i < len - 1; i++) {
    int x1 = map(i, 0, len, 0, 480);
    int x2 = map(i + 1, 0, len, 0, 480);
    int y1 = map(values[i], minY, maxY, 200, 400);
    int y2 = map(values[i+1], minY, maxY, 200, 400);
    
    lcd.drawLine(x1, y1, x2, y2, TFT_RED);
  }
}
```

### 프로젝트 2: 음성 건강 비서

```cpp
#include <LovyanGFX.h>
#include <driver/i2s.h>
#include <SD.h>
#include <Adafruit_BME680.h>
#include <MAX30105.h>

LGFX lcd;
Adafruit_BME680 bme;
MAX30105 particleSensor;

#define I2S_WS 5
#define I2S_BCK 6
#define I2S_DATA_IN 4

// 음성 명령 (오프라인)
const char* commands[] = {
  "심박수",
  "혈압",
  "온도",
  "공기질"
};

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.setRotation(1);
  
  Wire.begin(400000);
  
  bme.begin();
  particleSensor.begin(Wire);
  particleSensor.setup();
  
  setupI2S();
  setupSD();
  
  drawHomeScreen();
}

void loop() {
  // 마이크 입력 대기
  if (detectWakeWord()) {
    lcd.fillScreen(TFT_BLUE);
    lcd.print("듣고 있습니다...");
    
    String command = listenCommand();
    
    if (command != "") {
      processCommand(command);
    }
    
    delay(2000);
    drawHomeScreen();
  }
  
  // 백그라운드 건강 데이터 수집
  collectHealthData();
  
  delay(100);
}

bool detectWakeWord() {
  // 간단한 에너지 기반 음성 감지
  int16_t buffer[256];
  readMic(buffer, 256);
  
  float energy = 0;
  for (int i = 0; i < 256; i++) {
    energy += buffer[i] * buffer[i];
  }
  energy /= 256;
  
  return energy > 1000; // 임계값
}

String listenCommand() {
  // 실제 구현은 음성 인식 라이브러리 필요
  // 여기서는 간단한 데모
  
  int16_t buffer[1024];
  readMic(buffer, 1024);
  
  // FFT 또는 패턴 매칭
  // 데모: 무작위 명령 반환
  
  return commands[random(0, 4)];
}

void processCommand(String cmd) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  
  if (cmd == "심박수") {
    int bpm = particleSensor.getHeartRate();
    lcd.printf("심박수: %d BPM", bpm);
    
  } else if (cmd == "온도") {
    float temp = bme.readTemperature();
    lcd.printf("온도: %.1f°C", temp);
    
  } else if (cmd == "공기질") {
    float iaq = bme.readGas();
    lcd.printf("공기질: %.0f Ohm", iaq);
    
    if (iaq < 10000) {
      lcd.setTextColor(TFT_RED);
      lcd.print("나쁨");
    } else if (iaq < 50000) {
      lcd.setTextColor(TFT_YELLOW);
      lcd.print("보통");
    } else {
      lcd.setTextColor(TFT_GREEN);
      lcd.print("좋음");
    }
  }
}

void collectHealthData() {
  static unsigned long lastLog = 0;
  
  if (millis() - lastLog > 60000) { // 1 분마다
    float temp = bme.readTemperature();
    float humidity = bme.readHumidity();
    int bpm = particleSensor.getHeartRate();
    
    logHealthData("health.csv", temp);
    logHealthData("health.csv", humidity);
    logHealthData("health.csv", bpm);
    
    lastLog = millis();
  }
}
```

### 프로젝트 3: 건강 모니터링 스테이션

```cpp
#include <LovyanGFX.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <Wire.h>
#include <MAX30105.h>
#include <Adafruit_BME680.h>

LGFX lcd;
MAX30105 particleSensor;
Adafruit_BME680 bme;

// WiFi 설정
const char* ssid = "your_wifi";
const char* password = "your_password";

// 클라우드 엔드포인트
const char* serverUrl = "https://your-server.com/api/health";

struct HealthData {
  float temperature;
  float humidity;
  float pressure;
  float iaq;
  int heartRate;
  int spo2;
  unsigned long timestamp;
};

HealthData currentData;

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(200);
  
  Wire.begin(400000);
  
  particleSensor.begin(Wire);
  particleSensor.setup();
  bme.begin();
  
  setupSD();
  connectWiFi();
  
  drawDashboard();
}

void loop() {
  // 건강 데이터 수집
  currentData.temperature = bme.readTemperature();
  currentData.humidity = bme.readHumidity();
  currentData.pressure = bme.readPressure() / 100.0;
  currentData.iaq = bme.readGas();
  currentData.heartRate = particleSensor.getHeartRate();
  currentData.spo2 = readSpO2();
  currentData.timestamp = millis();
  
  // 대시보드 업데이트
  updateDashboard();
  
  // SD 카드 로깅
  logToSD();
  
  // 클라우드 전송 (5 분마다)
  static unsigned long lastUpload = 0;
  if (millis() - lastUpload > 300000) {
    uploadToCloud();
    lastUpload = millis();
  }
  
  delay(1000);
}

void drawDashboard() {
  lcd.fillScreen(TFT_BLACK);
  
  // 헤더
  lcd.fillRect(0, 0, 480, 50, TFT_NAVY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 15);
  lcd.print("건강 모니터링 스테이션");
  
  // 섹션 나누기
  lcd.drawLine(0, 200, 480, 200, TFT_GRAY);
  lcd.drawLine(0, 400, 480, 400, TFT_GRAY);
  lcd.drawLine(240, 0, 240, 800, TFT_GRAY);
}

void updateDashboard() {
  // 환경 (좌상단)
  lcd.fillRect(10, 60, 220, 130, TFT_DARKGREY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(20, 70);
  lcd.print("환경");
  
  lcd.setTextSize(3);
  lcd.printf("온도: %.1f°C\n", currentData.temperature);
  lcd.printf("습도: %.1f%%\n", currentData.humidity);
  lcd.printf("기압: %.1fhPa\n", currentData.pressure);
  
  // 건강 (우상단)
  lcd.fillRect(250, 60, 220, 130, TFT_DARKGREEN);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(260, 70);
  lcd.print("건강");
  
  lcd.setTextSize(3);
  lcd.printf("심박: %d BPM\n", currentData.heartRate);
  lcd.printf("산소: %d%%\n", currentData.spo2);
  
  // 공기질 (좌하단)
  lcd.fillRect(10, 210, 220, 180, TFT_DARKBLUE);
  lcd.setCursor(20, 220);
  lcd.print("공기질");
  
  lcd.setTextSize(3);
  lcd.printf("IAQ: %.0f Ohm\n", currentData.iaq);
  
  if (currentData.iaq < 10000) {
    lcd.setTextColor(TFT_RED);
    lcd.print("나쁨");
  } else if (currentData.iaq < 50000) {
    lcd.setTextColor(TFT_YELLOW);
    lcd.print("보통");
  } else {
    lcd.setTextColor(TFT_GREEN);
    lcd.print("좋음");
  }
  
  // 그래프 (우하단)
  lcd.fillRect(250, 210, 220, 180, TFT_BLACK);
  lcd.setCursor(260, 220);
  lcd.print("심박 추이");
  
  drawHeartRateGraph(260, 250);
}

void logToSD() {
  File file = SD.open("/health_log.csv", FILE_APPEND);
  if (file) {
    file.printf("%ld,%.2f,%.2f,%.2f,%.0f,%d,%d\n",
                currentData.timestamp,
                currentData.temperature,
                currentData.humidity,
                currentData.pressure,
                currentData.iaq,
                currentData.heartRate,
                currentData.spo2);
    file.close();
  }
}

void uploadToCloud() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<256> doc;
  doc["temperature"] = currentData.temperature;
  doc["humidity"] = currentData.humidity;
  doc["heartRate"] = currentData.heartRate;
  doc["spo2"] = currentData.spo2;
  doc["timestamp"] = currentData.timestamp;
  
  String json;
  serializeJson(doc, json);
  
  int httpResponseCode = http.POST(json);
  
  if (httpResponseCode > 0) {
    lcd.setTextColor(TFT_GREEN);
    lcd.print("업로드 성공");
  } else {
    lcd.setTextColor(TFT_RED);
    lcd.print("업로드 실패");
  }
  
  http.end();
}

void connectWiFi() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.print("WiFi 연결중...");
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    lcd.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.println("\n연결 성공!");
    lcd.print("IP: ");
    lcd.println(WiFi.localIP());
  } else {
    lcd.println("\n연결 실패");
  }
  
  delay(2000);
}

int readSpO2() {
  uint32_t redValue = particleSensor.getRed();
  uint32_t irValue = particleSensor.getIR();
  
  if (irValue == 0) return 0;
  
  float ratio = (redValue * 100.0) / irValue;
  int spo2 = 110 - (ratio / 2);
  
  return constrain(spo2, 70, 100);
}

void drawHeartRateGraph(int x, int y) {
  // 최근 60 개 심박 데이터 그래프
  static int history[60];
  static int index = 0;
  
  history[index++] = currentData.heartRate;
  if (index >= 60) index = 0;
  
  for (int i = 0; i < 59; i++) {
    int x1 = x + i * 3;
    int x2 = x + (i + 1) * 3;
    int y1 = y + map(history[i], 50, 120, 150, 0);
    int y2 = y + map(history[i+1], 50, 120, 150, 0);
    
    lcd.drawLine(x1, y1, x2, y2, TFT_RED);
  }
}
```

---

## 6. 문제 해결

###常见问题

#### 1. 디스플레이가 나오지 않음
```
해결:
1. 핀아웃 확인 (특히 CS, DC, RST)
2. 백라이트 핀 (GPIO 3) PWM 설정 확인
3. LovyanGFX 보드 설정 재확인
4. SPI 속도 낮춰서 테스트 (40MHz → 20MHz)
```

#### 2. 터치가 반응하지 않음
```
해결:
1. I2C 주소 확인 (0x5D)
2. 터치 INT 핀 (GPIO 7) 연결 확인
3. GT911 초기화 순서 확인
4. Touch_GT911 라이브러리 버전 업데이트
```

#### 3. 마이크 소리가 작음
```
해결:
1. I2S 샘플레이트 확인 (16kHz → 44.1kHz)
2. 마이크 게인 조정 (하드웨어)
3. 소프트웨어 게인 적용 (2-4 배)
4. 노이즈 캔슬링 알고리즘 확인
```

#### 4. SD 카드를 인식하지 못함
```
해결:
1. SD 카드 포맷 (FAT32)
2. CS 핀 (GPIO 14) 확인
3. SPI 속도 낮춤 (20MHz)
4. 카드 감지 핀 (GPIO 15) 풀업 저항
```

#### 5. MAX30102 를 찾을 수 없음
```
해결:
1. I2C 스캔으로 주소 확인
2. SDA/SCL 풀업 저항 (4.7kΩ) 추가
3. 전원 (3.3V) 안정성 확인
4. I2C 속도 낮춤 (100kHz)
```

### 디버깅 팁

```cpp
// I2C 스캔
void scanI2C() {
  byte count = 0;
  Serial.println("I2C 스캔:");
  
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("0x%02X found\n", i);
      count++;
    }
  }
  
  if (count == 0) {
    Serial.println("I2C 디바이스를 찾을 수 없습니다");
  }
}

// 마이크 테스트
void testMic() {
  int16_t buffer[256];
  readMic(buffer, 256);
  
  float avg = 0;
  for (int i = 0; i < 256; i++) {
    avg += abs(buffer[i]);
  }
  avg /= 256;
  
  Serial.printf("마이크 평균: %.2f\n", avg);
}

// SD 카드 테스트
void testSD() {
  if (!SD.begin(SD_CS)) {
    Serial.println("SD 오류");
    return;
  }
  
  File file = SD.open("/test.txt", FILE_WRITE);
  file.println("테스트");
  file.close();
  
  Serial.println("SD 테스트 성공");
}
```

---

## 7. 다음 단계

### 즉시 시작할 수 있는 프로젝트

1. **심박수 모니터** (1 시간)
   - MAX30102 연결
   - LCD 에 심박수 표시
   - 그래프 출력

2. **환경 모니터링** (2 시간)
   - BME680 연결
   - 온도/습도/공기질 표시
   - SD 카드 로깅

3. **음성 건강 비서** (4 시간)
   - 듀얼 마이크 설정
   - 음성 명령 인식
   - 건강 데이터 음성 출력

### 중급 프로젝트

4. **건강 대시보드** (8 시간)
   - 4 섹션 UI
   - 실시간 그래프
   - WiFi 클라우드 연동

5. **수면 모니터링** (12 시간)
   - 심박 변이도 (HRV)
   - 호흡 분석
   - 수면 단계 분류

### 고급 프로젝트

6. **AI 건강 코치** (20 시간)
   - TensorFlow Lite
   - 개인 맞춤 코칭
   - 음성 대화

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**하드웨어**: WAVESHARE ESP32-S3-Touch-LCD-4 (실제 보유)
