# ESP32 EduTech 🚀

**모든 학생이 IoT 개발자가 될 수 있다**

ESP32 기반 교육용 IoT 키트를 개발하고 보급하는 교육 기술 스타트업입니다.

---

## 📦 제품 라인업

### 1. Starter Kit (입문자용)
- **가격**: 49,000 원
- **대상**: 중학생 이상, 코딩 처음 시작하는 분
- **구성**: ESP32 DevKit, 10 종 센서, 기본 부품 40+ 개
- **프로젝트**: LED 제어, 센서 읽기, WiFi 연결, 웹 서버 만들기

### 2. Advanced Kit (중급자용)
- **가격**: 89,000 원
- **대상**: 고등학생, 대학생, IoT 취미 개발자
- **구성**: Starter Kit + 카메라, OLED, 모터, GPS, NFC 등
- **프로젝트**: 이미지 캡처, 디스플레이, 블루투스 오디오, 위치 추적

### 3. IoT Smart Home Kit (심화용)
- **가격**: 129,000 원
- **대상**: 대학생, 개발자, 스마트홈 애호가
- **구성**: Advanced Kit + ESP32 2 개 추가, MQTT, Zigbee, LoRa
- **프로젝트**: 스마트홈 시스템, 음성 제어, 클라우드 연동

### 4. Healthcare Kit (헬스케어/바이오)
- **가격**: 159,000 원
- **대상**: 의대생, 바이오공학과, 헬스케어 스타트업, 연구자
- **구성**: Advanced Kit + 심전도/근전도/심박/혈중산소/체온/호흡 센서
- **프로젝트**: 심박 측정, 심전도 기록, 수면 모니터링, 낙상 감지, 원격 환자 관리

### 5. ESP32-S3 헬스케어 키트 (고급 디스플레이) 🆕
- **Healthcare Kit Pro**: 189,000 원 (1.28 인치 원형 터치, 웨어러블)
- **Healthcare Kit Station**: 249,000 원 (4 인치 터치, 듀얼 마이크, 거치형)
- **Healthcare Kit AI**: 299,000 원 (2.16 인치 AMOLED, AI 가속, 프리미엄)
- **프로젝트**: 심박 워치, 음성 건강 비서, AI 수면 코치, 스마트 워치

---

## 🎓 교육 커리큘럼

```
Level 1: 기초 (4 주)
├─ 주 1: ESP32 소개 + LED 제어
├─ 주 2: 센서 읽기 + 시리얼 통신
├─ 주 3: WiFi 연결 + 웹 서버
└─ 주 4: 최종 프로젝트

Level 2: 중급 (6 주)
├─ 주 1-2: 카메라 + 디스플레이
├─ 주 3-4: 모터 + 액츄에이터
├─ 주 5: 블루투스 + 오디오
└─ 주 6: 통합 프로젝트

Level 3: 고급 (8 주)
├─ 주 1-2: MQTT 프로토콜
├─ 주 3-4: 클라우드 연동
├─ 주 5-6: 스마트폰 앱 개발
├─ 주 7: 음성 제어
└─ 주 8: 최종 프로젝트 (스마트홈)

Level 4: 헬스케어 전문 (6 주) 🆕
├─ 주 1: WAVESHARE ESP32-S3-Touch-LCD-4 셋업
├─ 주 2: MAX30102 심박/혈중산소 측정
├─ 주 3: BME680 환경 센서 (온도/습도/공기질)
├─ 주 4: 듀얼 마이크 음성 인식
├─ 주 5: 건강 모니터링 스테이션 (WiFi 클라우드)
└─ 주 6: AI 건강 코치 (TensorFlow Lite)
```

---

## 📚 문서

- [사업 계획서](docs/business-plan.md)
- [제품 라인업](docs/product-lineup.md)
- [커리큘럼](docs/curriculum.md) (준비 중)
- [펌웨어 가이드](firmware/README.md) (준비 중)
- [하드웨어 가이드](hardware/README.md) (준비 중)

---

## 🛠️ 개발 환경

### 필요 소프트웨어
- [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- [PlatformIO (VS Code)](https://platformio.org/)
- ESP32 Arduino Core

### ESP32 Arduino Core 설치
```bash
# Arduino IDE 에서
# File > Preferences > Additional Board Manager URLs
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

# Tools > Board > Boards Manager > ESP32 by Espressif Systems 설치
```

---

## 📦 예제 코드

각 키트별 10-20 개 프로젝트 예제 코드가 포함됩니다.

```cpp
// 예제: LED 깜빡이기 (Starter Kit 프로젝트 2)
#define LED_PIN 2

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
```

전체 예제 코드는 [examples/](examples/) 디렉토리에서 확인하세요.

---

## 🤝 커뮤니티

- **Discord**: [참여 링크] (준비 중)
- **YouTube**: [채널 링크] (준비 중)
- **Blog**: [블로그 링크] (준비 중)
- **이메일**: contact@esp32edutech.kr

---

## 📅 로드맵

### 2026 Q1 (1-3 월): 준비
- [x] 회사 설립
- [x] 사업 계획 수립
- [ ] 초기 제품 소싱
- [ ] 웹사이트 오픈

### 2026 Q2 (4-6 월): 론칭
- [ ] Starter Kit 출시
- [ ] YouTube 채널 오픈
- [ ] 메이커 페어 참가
- [ ] 얼리버드 판매

### 2026 Q3 (7-9 월): 확장
- [ ] Advanced Kit 출시
- [ ] 온라인 강의 오픈
- [ ] 교육 기관 파트너십

### 2026 Q4 (10-12 월): 성장
- [ ] IoT Smart Home Kit 출시
- [ ] 커뮤니티 5,000 명
- [ ] 흑자 전환

---

## 📄 라이선스

- **하드웨어**: CERN OHL (Open Hardware License)
- **펌웨어**: MIT License
- **문서**: CC BY-SA 4.0

---

## 🏢 회사 정보

- **회사명**: ESP32 EduTech (가칭)
- **대표**: 용수 박
- **설립일**: 2026 년
- **이메일**: contact@esp32edutech.kr
- **웹사이트**: https://esp32edutech.kr (준비 중)

---

** visi on**: "모든 학생이 IoT 개발자가 될 수 있다"  
**mission**: 하드웨어 프로그래밍 장벽을 낮추고 창의적인 IoT 프로젝트 경험을 제공한다
