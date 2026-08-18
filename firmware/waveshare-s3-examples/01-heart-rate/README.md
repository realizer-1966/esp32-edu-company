# 심박수 모니터 (Heart Rate Monitor)

WAVESHARE ESP32-S3-Touch-LCD-4 를 사용한 실시간 심박수 측정 프로젝트입니다.

## 하드웨어 요구사항

- **메인 보드**: WAVESHARE ESP32-S3-Touch-LCD-4
- **센서**: MAX30102 (또는 MAX30105)
- **연결**: I2C (SDA=GPIO8, SCL=GPIO9)

## 회로 연결

```
MAX30102 → ESP32-S3
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → GPIO 8
└─ SCL → GPIO 9
```

## 설치

### Arduino IDE

1. `01-heart-rate.ino` 파일 열기
2. ESP32 by Espressif Systems 보드 매니저에서 설치
3. 보드: `ESP32S3 Dev Module` 선택
4. 업로드

### PlatformIO (추천)

```bash
# 프로젝트 폴더로 이동
cd 01-heart-rate

# 의존성 설치
pio lib install

# 빌드
pio run

# 업로드
pio run --target upload

# 시리얼 모니터
pio device monitor
```

## 기능

- **실시간 심박수 측정**: 1 초당 1 샘플
- **PPG 그래프**: 60 초 데이터 실시간 표시
- **4 인치 LCD**: 480x800 해상도
- **터치 인터페이스**: 추후 확장 가능

## 화면 구성

```
┌─────────────────────────┐
│   심박수 모니터         │ ← 헤더
├─────────────────────────┤
│           BPM           │
│           72            │ ← 심박수 표시
│                         │
├─────────────────────────┤
│  PPG 신호               │
│  ╱╲  ╱╲╱╲  ╱╲          │ ← 실시간 그래프
│ ╱  ╲╱    ╲╱  ╲╲        │
│                         │
└─────────────────────────┘
```

## 사용법

1. 보드에 전원 연결 (USB-C 또는 배터리)
2. 손가락을 MAX30102 센서에 가볍게 올림
3. 10-20 초 후 심박수 표시
4. 그래프로 PPG 신호 확인

## 문제 해결

### MAX30102 를 찾을 수 없음
- I2C 연결 확인 (SDA/SCL)
- 3.3V 전원 확인
- 풀업 저항 (4.7kΩ) 추가

### 심박수가 표시되지 않음
- 손가락을 더 세게 누르지 않기 (혈류 차단)
- 손가락 위치 조정
- 30 초 이상 기다리기

### 그래프가 너무 작음
- `map()` 함수 범위 조정
- 환경에 따라 LED 전류 조절 (`setPulseAmplitudeRed`)

## 확장 아이디어

- [ ] 터치로 그래프 확대/축소
- [ ] SD 카드에 데이터 로깅
- [ ] WiFi 로 클라우드 전송
- [ ] 저심박/고심박 알림
- [ ] 혈중산소포화도 (SpO2) 추가

## 라이선스

MIT License

## 참고

- [WAVESHARE ESP32-S3-Touch-LCD-4 문서](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4)
- [MAX30102 데이터시트](https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf)
- [LovyanGFX 라이브러리](https://github.com/lovyan0320/LovyanGFX)
