/*
 * ESP32-S3 WiFi + BLE 복합 존재 감지기
 * 
 * 하드웨어:
 * - WAVESHARE ESP32-S3-Touch-LCD-4
 * 
 * 기능:
 * - WiFi RSSI 로 동작 감지
 * - BLE 스캔으로 디바이스 탐지
 * - 두 신호 퓨전하여 정확도 향상
 * - 알려진 디바이스 (가족 폰) 식별
 * 
 * 작성일: 2026-08-18
 * 라이선스: MIT
 */

#include <LovyanGFX.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <ArduinoJson.h>

// 디스플레이 설정
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

// WiFi 설정
const char* ssid = "your_wifi_ssid";
const char* password = "your_password";

// BLE 설정
BLEScan* pBLEScan;
bool bleScanning = false;

// 알려진 디바이스 (가족 폰/워치 MAC 주소)
// 실제 MAC 주소로 변경하세요!
struct KnownDevice {
  String mac;
  String name;
};

KnownDevice knownDevices[] = {
  {"aa:bb:cc:dd:ee:01", "아빠"},  // ← 실제 MAC 주소로 변경
  {"aa:bb:cc:dd:ee:02", "엄마"},
  {"aa:bb:cc:dd:ee:03", "아이"}
};
const int knownDeviceCount = 3;

// WiFi 데이터
int rssiHistory[60];
int rssiIndex = 0;
float wifiStdDev = 0;
bool wifiMotion = false;

// BLE 데이터
int bleDeviceCount = 0;
String detectedDevices = "";
bool blePresence = false;

// 퓨전 결과
bool isOccupied = false;
int occupancyConfidence = 0;  // 0-100%
unsigned long lastMotionTime = 0;

// 통계
int totalDetections = 0;
int falsePositives = 0;

void setup() {
  Serial.begin(115200);
  
  // LCD 초기화
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(200);
  lcd.fillScreen(TFT_BLACK);
  
  // UI 그리기
  drawUI();
  
  // WiFi 연결
  lcd.setTextColor(TFT_YELLOW);
  lcd.setTextSize(2);
  lcd.setCursor(20, 100);
  lcd.print("WiFi 연결중...");
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    lcd.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setTextColor(TFT_GREEN);
    lcd.printf("\n연결됨!\nRSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("WiFi 연결됨, RSSI: %d dBm\n", WiFi.RSSI());
  } else {
    lcd.setTextColor(TFT_RED);
    lcd.print("\n연결 실패");
    while (1) delay(1000);
  }
  
  // BLE 초기화
  lcd.setCursor(20, 200);
  lcd.print("BLE 초기화...");
  
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(90);
  pBLEScan->setActiveScan(true);
  
  lcd.setTextColor(TFT_GREEN);
  lcd.print("완료");
  
  delay(1000);
  lcd.fillScreen(TFT_BLACK);
  drawUI();
}

void loop() {
  // 1. WiFi 동작 감지 (1 초)
  wifiMotion = detectWiFiMotion();
  
  // 2. BLE 디바이스 스캔 (3 초)
  bleDeviceCount = scanBLEDevices();
  blePresence = bleDeviceCount > 0;
  
  // 3. 퓨전 (가중치: WiFi 40%, BLE 60%)
  int wifiWeight = 40;
  int bleWeight = 60;
  
  occupancyConfidence = 0;
  
  if (wifiMotion) {
    occupancyConfidence += wifiWeight;
  }
  
  if (blePresence) {
    occupancyConfidence += bleWeight;
    
    // 알려진 디바이스면 추가 점수
    if (detectedDevices != "") {
      occupancyConfidence += 20;  // 보너스
    }
  }
  
  // 임계값: 50% 이상이면 "사람 있음"
  isOccupied = occupancyConfidence >= 50;
  
  if (isOccupied) {
    lastMotionTime = millis();
    totalDetections++;
  }
  
  // 4. 화면 업데이트
  updateDisplay();
  
  // 5. 시리얼 로그
  Serial.printf("WiFi: %s, BLE: %d 개, 점수: %d%%, 결과: %s\n",
                wifiMotion ? "동작" : "정지",
                bleDeviceCount,
                occupancyConfidence,
                isOccupied ? "사람 있음" : "사람 없음");
  
  delay(100);
}

bool detectWiFiMotion() {
  int rssi = WiFi.RSSI();
  rssiHistory[rssiIndex++] = rssi;
  
  if (rssiIndex >= 60) {
    rssiIndex = 0;
  }
  
  wifiStdDev = calculateStdDev(rssiHistory, 60);
  
  // 표준편차 5dB 이상이면 동작
  return wifiStdDev > 5.0;
}

int scanBLEDevices() {
  // BLE 스캔 (3 초)
  BLEScanResults results = pBLEScan->start(3, false);
  
  int count = results.getCount();
  detectedDevices = "";
  
  // 알려진 디바이스 확인
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    String address = device.getAddress().toString();
    
    // 알려진 디바이스인지 확인
    for (int j = 0; j < knownDeviceCount; j++) {
      if (address.startsWith(knownDevices[j].mac.substring(0, 8))) {
        if (detectedDevices != "") {
          detectedDevices += ", ";
        }
        detectedDevices += knownDevices[j].name;
      }
    }
  }
  
  return count;
}

void updateDisplay() {
  // 상태에 따라 배경색 변경
  if (isOccupied) {
    lcd.fillScreen(TFT_GREEN);
  } else {
    lcd.fillScreen(TFT_RED);
  }
  
  // 헤더
  lcd.fillRect(0, 0, 480, 50, TFT_NAVY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(80, 15);
  lcd.print("WiFi+BLE 존재 감지");
  
  // 주요 상태
  lcd.setTextColor(TFT_BLACK);
  lcd.setTextSize(4);
  lcd.setCursor(100, 100);
  
  if (isOccupied) {
    lcd.print("사람 있음");
  } else {
    lcd.print("사람 없음");
  }
  
  // 신뢰도
  lcd.setTextSize(2);
  lcd.setCursor(150, 160);
  lcd.printf("신뢰도: %d%%", occupancyConfidence);
  
  // WiFi 정보
  lcd.fillRect(20, 220, 440, 80, TFT_DARKGREY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(30, 235);
  lcd.printf("WiFi: %s (표준편차: %.2f dB)",
             wifiMotion ? "동작 감지" : "정지",
             wifiStdDev);
  
  // BLE 정보
  lcd.fillRect(20, 320, 440, 80, TFT_DARKBLUE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(30, 335);
  lcd.printf("BLE: %d 개 디바이스", bleDeviceCount);
  
  if (detectedDevices != "") {
    lcd.setCursor(30, 365);
    lcd.printf("알려진: %s", detectedDevices.c_str());
  }
  
  // 통계
  lcd.fillRect(20, 420, 440, 60, TFT_DARKGREEN);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(30, 435);
  lcd.printf("총 감지: %d 회", totalDetections);
  
  // 그래프 (RSSI)
  drawRSSIGraph();
}

void drawRSSIGraph() {
  int graphX = 20;
  int graphY = 500;
  int graphWidth = 440;
  int graphHeight = 100;
  
  lcd.fillRect(graphX, graphY, graphWidth, graphHeight, TFT_BLACK);
  lcd.drawRect(graphX, graphY, graphWidth, graphHeight, TFT_GRAY);
  
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(graphX + 10, graphY + 20);
  lcd.print("RSSI 추이 (60 초)");
  
  // 그래프 그리기
  int minRSSI = -100;
  int maxRSSI = -30;
  
  for (int i = 0; i < 59; i++) {
    if (rssiHistory[i] == 0 || rssiHistory[i+1] == 0) continue;
    
    int x1 = graphX + 10 + (i * (graphWidth - 20)) / 59;
    int x2 = graphX + 10 + ((i + 1) * (graphWidth - 20)) / 59;
    int y1 = graphY + 40 + map(rssiHistory[i], minRSSI, maxRSSI, graphHeight - 50, 0);
    int y2 = graphY + 40 + map(rssiHistory[i+1], minRSSI, maxRSSI, graphHeight - 50, 0);
    
    y1 = constrain(y1, graphY + 40, graphY + graphHeight - 10);
    y2 = constrain(y2, graphY + 40, graphY + graphHeight - 10);
    
    uint16_t color = abs(rssiHistory[i+1] - rssiHistory[i]) > 5 ? TFT_RED : TFT_GREEN;
    lcd.drawLine(x1, y1, x2, y2, color);
  }
}

void drawUI() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  lcd.setCursor(60, 50);
  lcd.print("WiFi+BLE 존재 감지");
  
  lcd.setTextSize(2);
  lcd.setCursor(50, 150);
  lcd.print("초기화중...");
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
