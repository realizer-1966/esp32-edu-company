/*
 * ESP32-S3 WiFi 존재 감지기 (RSSI 기반)
 * 
 * 하드웨어:
 * - WAVESHARE ESP32-S3-Touch-LCD-4
 * 
 * 기능:
 * - WiFi 신호 강도 (RSSI) 모니터링
 * - 표준편차로 동작 감지
 * - 4 인치 LCD 에 상태 표시
 * 
 * 원리:
 * - 사람이 움직이면 WiFi 신호가 간섭받음
 * - RSSI 변동성이 증가하면 "사람 있음"으로 판단
 * 
 * 작성일: 2026-08-18
 * 라이선스: MIT
 */

#include <LovyanGFX.h>
#include <WiFi.h>

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
const char* ssid = "your_wifi_ssid";     // ← WiFi SSID 입력
const char* password = "your_password";  // ← WiFi 비밀번호 입력

// RSSI 데이터
int rssiHistory[60];  // 60 초간 데이터
int rssiIndex = 0;
int lastRSSI = 0;

// 감지 임계값
const float MOTION_THRESHOLD = 5.0;  // 표준편차 5dB 이상이면 동작 감지
const int CALIBRATION_TIME = 10;     // 초기 10 초 보정

bool isOccupied = false;
bool isCalibrating = true;
unsigned long startTime = 0;

// 통계
float currentStdDev = 0;
int motionCount = 0;
unsigned long lastMotionTime = 0;

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
  lcd.setCursor(20, 250);
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
    Serial.println("WiFi 연결 실패");
    while (1) delay(1000);
  }
  
  startTime = millis();
}

void loop() {
  // RSSI 읽기
  int rssi = WiFi.RSSI();
  
  // 히스토리 저장
  rssiHistory[rssiIndex++] = rssi;
  if (rssiIndex >= 60) {
    rssiIndex = 0;
  }
  
  // 보정 기간 확인
  if (millis() - startTime < CALIBRATION_TIME * 1000) {
    // 보정 중
    int remaining = CALIBRATION_TIME - (millis() - startTime) / 1000;
    lcd.fillRect(0, 350, 480, 50, TFT_BLACK);
    lcd.setTextColor(TFT_YELLOW);
    lcd.setTextSize(2);
    lcd.setCursor(100, 365);
    lcd.printf("보정중... %d 초", remaining);
  } else {
    isCalibrating = false;
    
    // 표준편차 계산
    currentStdDev = calculateStdDev(rssiHistory, 60);
    
    // 동작 감지
    bool motion = currentStdDev > MOTION_THRESHOLD;
    
    if (motion && !isOccupied) {
      isOccupied = true;
      motionCount++;
      lastMotionTime = millis();
      lcd.fillScreen(TFT_GREEN);
      drawOccupiedUI();
      Serial.println("동작 감지! 사람 있음");
    } else if (!motion && isOccupied) {
      // 30 초간 움직임 없으면 "사람 없음"
      if (millis() - lastMotionTime > 30000) {
        isOccupied = false;
        lcd.fillScreen(TFT_RED);
        drawEmptyUI();
        Serial.println("동작 없음. 사람 없음");
      }
    }
    
    // 통계 업데이트
    updateStats();
  }
  
  // RSSI 그래프 업데이트
  drawRSSIGraph();
  
  delay(100); // 10Hz 샘플링
}

void drawUI() {
  lcd.fillScreen(TFT_BLACK);
  
  // 헤더
  lcd.fillRect(0, 0, 480, 50, TFT_NAVY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(120, 15);
  lcd.print("WiFi 존재 감지기");
  
  // RSSI 표시
  lcd.fillRect(10, 70, 460, 100, TFT_DARKGREY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(20, 85);
  lcd.print("현재 RSSI:");
  
  lcd.setTextSize(4);
  lcd.setCursor(200, 100);
  lcd.print("   dBm");
  
  // 상태 표시 영역
  lcd.fillRect(10, 190, 460, 120, TFT_BLACK);
  lcd.drawRect(10, 190, 460, 120, TFT_GRAY);
  
  // 그래프 영역
  lcd.fillRect(10, 330, 460, 150, TFT_BLACK);
  lcd.drawRect(10, 330, 460, 150, TFT_GRAY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(20, 345);
  lcd.print("RSSI 추이 (60 초)");
  
  // 통계
  lcd.fillRect(10, 500, 460, 80, TFT_DARKBLUE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(20, 515);
  lcd.print("통계:");
}

void drawOccupiedUI() {
  lcd.fillRect(10, 190, 460, 120, TFT_GREEN);
  lcd.drawRect(10, 190, 460, 120, TFT_WHITE);
  
  lcd.setTextColor(TFT_BLACK);
  lcd.setTextSize(4);
  lcd.setCursor(100, 230);
  lcd.print("사람 있음");
  
  lcd.setTextSize(2);
  lcd.setCursor(150, 270);
  lcd.printf("감지 횟수: %d", motionCount);
}

void drawEmptyUI() {
  lcd.fillRect(10, 190, 460, 120, TFT_RED);
  lcd.drawRect(10, 190, 460, 120, TFT_WHITE);
  
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(4);
  lcd.setCursor(100, 230);
  lcd.print("사람 없음");
  
  lcd.setTextSize(2);
  lcd.setCursor(120, 270);
  lcd.print("30 초간 움직임 없음");
}

void updateStats() {
  lcd.fillRect(20, 540, 440, 30, TFT_DARKBLUE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(20, 545);
  lcd.printf("표준편차: %.2f dB | 감지: %d 회", currentStdDev, motionCount);
}

void drawRSSIGraph() {
  // 그래프 지우기
  lcd.fillRect(12, 360, 456, 118, TFT_BLACK);
  
  // 최소/최대 찾기
  int minRSSI = -100;
  int maxRSSI = -30;
  
  // 그리기
  for (int i = 0; i < 59; i++) {
    if (rssiHistory[i] == 0 || rssiHistory[i+1] == 0) continue;
    
    int x1 = 12 + (i * 456) / 59;
    int x2 = 12 + ((i + 1) * 456) / 59;
    int y1 = 360 + map(rssiHistory[i], minRSSI, maxRSSI, 118, 0);
    int y2 = 360 + map(rssiHistory[i+1], minRSSI, maxRSSI, 118, 0);
    
    y1 = constrain(y1, 360, 478);
    y2 = constrain(y2, 360, 478);
    
    // 색상: 변동성 크면 빨강
    uint16_t color = abs(rssiHistory[i+1] - rssiHistory[i]) > 5 ? TFT_RED : TFT_GREEN;
    lcd.drawLine(x1, y1, x2, y2, color);
  }
}

void drawCurrentRSSI(int rssi) {
  lcd.fillRect(200, 100, 200, 50, TFT_DARKGREY);
  lcd.setTextColor(rssi < -70 ? TFT_RED : TFT_GREEN);
  lcd.setTextSize(4);
  lcd.setCursor(200, 100);
  lcd.printf("%d", rssi);
}

float calculateStdDev(int* data, int len) {
  // 평균 계산
  float sum = 0;
  for (int i = 0; i < len; i++) {
    sum += data[i];
  }
  float mean = sum / len;
  
  // 분산 계산
  float variance = 0;
  for (int i = 0; i < len; i++) {
    variance += (data[i] - mean) * (data[i] - mean);
  }
  
  // 표준편차
  return sqrt(variance / len);
}
