/*
 * ESP32-S3-Touch-LCD-4 심박수 모니터
 * 
 * 하드웨어:
 * - WAVESHARE ESP32-S3-Touch-LCD-4
 * - MAX30102 센서 (I2C: SDA=8, SCL=9)
 * 
 * 기능:
 * - 실시간 심박수 측정
 * - 4 인치 LCD 에 그래프 표시
 * - 60 초 데이터 버퍼
 * 
 * 작성일: 2026-08-18
 * 라이선스: MIT
 */

#include <LovyanGFX.h>
#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"

// 디스플레이 초기화
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
MAX30105 particleSensor;

// 60 초 데이터 버퍼 (1 초당 1 샘플)
uint32_t irValues[60];
int irIndex = 0;
unsigned long lastSample = 0;

// 화면 영역
const int GRAPH_X = 0;
const int GRAPH_Y = 200;
const int GRAPH_WIDTH = 480;
const int GRAPH_HEIGHT = 200;

void setup() {
  Serial.begin(115200);
  
  // LCD 초기화
  lcd.init();
  lcd.setRotation(1);  // 세로 모드
  lcd.setBrightness(128);
  lcd.fillScreen(TFT_BLACK);
  
  // I2C 초기화
  Wire.begin(400000);  // 400kHz
  
  // MAX30102 초기화
  Serial.println("MAX30102 초기화...");
  if (!particleSensor.begin(Wire)) {
    lcd.setTextColor(TFT_RED);
    lcd.setTextSize(3);
    lcd.setCursor(50, 100);
    lcd.print("MAX30102 오류");
    Serial.println("MAX30102 를 찾을 수 없습니다");
    while (1);
  }
  
  Serial.println("MAX30102 성공");
  
  // 센서 설정
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);   // LED 전류 12.8mA
  particleSensor.setPulseAmplitudeGreen(0x1F); // LED 전류 12.8mA
  
  // 화면 그리기
  drawUI();
}

void loop() {
  // 1 초마다 샘플링
  if (millis() - lastSample >= 1000) {
    lastSample = millis();
    
    // IR 값 읽기
    uint32_t irValue = particleSensor.getIR();
    irValues[irIndex++] = irValue;
    
    // 버퍼 순환
    if (irIndex >= 60) {
      irIndex = 0;
    }
    
    // 심박수 계산
    int bpm = 0;
    if (checkForBeat(irValue)) {
      bpm = (int)getBPM();
    }
    
    // 화면 업데이트
    updateDisplay(bpm, irValue);
    
    // 시리얼 출력
    Serial.printf("IR: %lu, BPM: %d\n", irValue, bpm);
  }
  
  delay(10);
}

void drawUI() {
  lcd.fillScreen(TFT_BLACK);
  
  // 헤더
  lcd.fillRect(0, 0, 480, 50, TFT_NAVY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(140, 15);
  lcd.print("심박수 모니터");
  
  // BPM 표시 영역
  lcd.fillRect(10, 60, 460, 120, TFT_DARKGREY);
  lcd.setTextColor(TFT_GREEN);
  lcd.setTextSize(5);
  lcd.setCursor(100, 100);
  lcd.print("BPM");
  
  // 그래프 영역
  lcd.fillRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, TFT_BLACK);
  lcd.drawRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, TFT_GRAY);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, GRAPH_Y + 20);
  lcd.print("PPG 신호");
}

void updateDisplay(int bpm, uint32_t irValue) {
  // BPM 업데이트
  lcd.fillRect(250, 90, 200, 80, TFT_DARKGREY);
  lcd.setTextColor(bpm > 0 ? TFT_GREEN : TFT_YELLOW);
  lcd.setTextSize(6);
  lcd.setCursor(280, 100);
  
  if (bpm > 0) {
    lcd.printf("%d", bpm);
  } else {
    lcd.print("--");
  }
  
  // 그래프 업데이트
  drawGraph();
  
  // 상태 LED
  lcd.fillCircle(450, 30, 10, bpm > 0 ? TFT_GREEN : TFT_RED);
}

void drawGraph() {
  // 그래프 지우기
  lcd.fillRect(GRAPH_X + 2, GRAPH_Y + 40, GRAPH_WIDTH - 4, GRAPH_HEIGHT - 42, TFT_BLACK);
  
  if (irIndex < 2) return; // 최소 2 개 샘플 필요
  
  // 최소/최대 값 찾기
  uint32_t minValue = irValues[0];
  uint32_t maxValue = irValues[0];
  
  for (int i = 0; i < 60; i++) {
    if (irValues[i] < minValue && irValues[i] > 0) {
      minValue = irValues[i];
    }
    if (irValues[i] > maxValue) {
      maxValue = irValues[i];
    }
  }
  
  // 그래프 그리기
  for (int i = 0; i < 59; i++) {
    if (irValues[i] == 0 || irValues[i+1] == 0) continue;
    
    int x1 = GRAPH_X + 2 + (i * (GRAPH_WIDTH - 4)) / 59;
    int x2 = GRAPH_X + 2 + ((i + 1) * (GRAPH_WIDTH - 4)) / 59;
    int y1 = GRAPH_Y + 40 + map(irValues[i], minValue, maxValue, GRAPH_HEIGHT - 42, 0);
    int y2 = GRAPH_Y + 40 + map(irValues[i+1], minValue, maxValue, GRAPH_HEIGHT - 42, 0);
    
    // Y 좌표 클램핑
    y1 = constrain(y1, GRAPH_Y + 40, GRAPH_Y + GRAPH_HEIGHT - 2);
    y2 = constrain(y2, GRAPH_Y + 40, GRAPH_Y + GRAPH_HEIGHT - 2);
    
    lcd.drawLine(x1, y1, x2, y2, TFT_RED);
  }
}
