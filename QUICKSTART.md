# ESP32-S3 헬스케어 개발환경 빠른 시작 가이드

**5 분 안에 개발 시작!**

---

## 1️⃣ 필수 소프트웨어 설치 (3 분)

### Windows 사용자

```powershell
# 1. VS Code 설치
winget install Microsoft.VisualStudioCode

# 2. Python 설치
winget install Python.Python.3.11

# 3. Git 설치
winget install Git.Git

# 4. PlatformIO (VS Code 에서)
# VS Code 실행 → 확장 프로그램 → "PlatformIO IDE" 검색 → 설치
```

### macOS 사용자

```bash
# 1. Homebrew 설치 (없으면)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. VS Code 설치
brew install --cask visual-studio-code

# 3. Python 설치
brew install python@3.11

# 4. Git 설치 (기본 포함)
# 5. PlatformIO (VS Code 에서 설치)
```

### Linux 사용자 (Ubuntu/Debian)

```bash
# 1. 필수 패키지 설치
sudo apt update
sudo apt install -y code python3 python3-pip python3-venv git

# 2. VS Code 에서 PlatformIO 설치
# 확장 프로그램 → "PlatformIO IDE" → 설치
```

---

## 2️⃣ 저장소 클론 (1 분)

```bash
# 저장소 클론
git clone https://github.com/realizer-1966/esp32-edu-company.git
cd esp32-edu-company

# 현재 브랜치 확인
git branch  # main 브랜치여야 함
```

---

## 3️⃣ 첫 프로젝트 빌드 (1 분)

```bash
# 심박수 모니터 프로젝트로 이동
cd firmware/waveshare-s3-examples/01-heart-rate

# PlatformIO 라이브러리 설치
pio lib install

# 빌드
pio run
```

**성공 출력:**
```
Building in release mode
Compiling .pio/build/esp32-s3-devkitc-1/src/main.cpp.o
Linking .pio/build/esp32-s3-devkitc-1/firmware.elf
Building .pio/build/esp32-s3-devkitc-1/firmware.bin
========================= [SUCCESS] Took 15.23 seconds =========================
```

---

## 4️⃣ 보드 연결 및 업로드

### 4.1 하드웨어 연결

1. **USB-C 케이블**로 보드와 PC 연결
2. **LED 점등** 확인 (충전/연결 표시)
3. **포트 확인:**
   ```bash
   # Windows
   device manager → Ports (COM & LPT)
   
   # macOS
   ls /dev/cu.usbserial*
   
   # Linux
   ls /dev/ttyUSB*
   ```

### 4.2 드라이버 설치 (필요시)

#### Windows
- **CH340 드라이버**: https://www.wch.cn/downloads/CH341SER_ZIP.html
- 설치 후 재부팅

#### Linux
```bash
# 포트 권한 설정
sudo usermod -a -G dialout $USER
# 재부팅 필요
```

### 4.3 업로드

```bash
# 부트모드 진입 (실패시)
1. BOOT 버튼 누름
2. RESET 버튼 누름
3. BOOT 버튼 놓음
4. RESET 버튼 놓음

# 업로드
pio run --target upload

# 시리얼 모니터
pio device monitor --baud 115200
```

---

## 5️⃣ WiFi 설정 (필요시)

WiFi 관련 프로젝트 (05-wifi-presence, 07-ble-presence) 사용시:

```cpp
// firmware/waveshare-s3-examples/05-wifi-presence/05-wifi-presence.ino 수정
const char* ssid = "your_wifi_ssid";     // ← WiFi SSID 입력
const char* password = "your_password";  // ← WiFi 비밀번호 입력
```

**업로드:**
```bash
pio run --target upload
```

---

## 6️⃣ BLE 설정 (필요시)

BLE 존재 감지기 (07-ble-presence) 사용시:

```cpp
// 알려진 디바이스 MAC 주소 수정
KnownDevice knownDevices[] = {
  {"aa:bb:cc:dd:ee:01", "아빠"},  // ← 실제 폰 MAC 주소로 변경
  {"aa:bb:cc:dd:ee:02", "엄마"},
  {"aa:bb:cc:dd:ee:03", "아이"}
};
```

**MAC 주소 찾는 법:**
- **Android**: 설정 → 휴대전화 정보 → 상태 → WiFi MAC 주소
- **iPhone**: 설정 → 일반 → 정보 → Bluetooth 주소

---

## 7️⃣ 테스트 프로젝트

### 추천 순서

| # | 프로젝트 | 시간 | 난이도 | 필요 센서 |
|---|----------|------|--------|-----------|
| 1 | **01-heart-rate** | 10 분 | ⭐ | MAX30102 |
| 2 | **05-wifi-presence** | 15 분 | ⭐⭐ | 없음 |
| 3 | **07-ble-presence** | 20 분 | ⭐⭐⭐ | 없음 |

### 1. 심박수 모니터 (01-heart-rate)

```bash
cd firmware/waveshare-s3-examples/01-heart-rate
pio run --target upload
pio device monitor
```

**할 일:**
- MAX30102 센서 연결 (I2C: SDA=8, SCL=9)
- 손가락을 센서에 올림
- LCD 에 심박수 표시 확인

### 2. WiFi 존재 감지 (05-wifi-presence)

```bash
cd firmware/waveshare-s3-examples/05-wifi-presence
# WiFi SSID/비밀번호 수정
pio run --target upload
```

**할 일:**
- 방 안에 사람 출입 테스트
- LCD 에 "사람 있음/없음" 표시 확인
- RSSI 그래프 관찰

### 3. WiFi+BLE 복합 감지 (07-ble-presence)

```bash
cd firmware/waveshare-s3-examples/07-ble-presence
# WiFi 및 BLE MAC 주소 수정
pio run --target upload
```

**할 일:**
- 스마트폰 BLE 스캔 확인
- WiFi 동작 + BLE 디바이스 동시 감지
- 신뢰도 95% 이상 확인

---

## 8️⃣ 문제 해결

### ❌ "Failed to connect to ESP32-S3"

**해결:**
```bash
# 부트모드 진입
1. BOOT 버튼 누르기
2. RESET 버튼 누르기
3. BOOT 버튼 놓기
4. RESET 버튼 놓기
5. pio run --target upload 재시도
```

### ❌ "Permission denied" (Linux/macOS)

**해결:**
```bash
# Linux
sudo chmod 666 /dev/ttyUSB0

# macOS
sudo chmod 666 /dev/cu.usbserial*

# 영구 설정 (Linux)
sudo usermod -a -G dialout $USER
# 재부팅
```

### ❌ "COM 포트 없음" (Windows)

**해결:**
1. 장치 관리자 확인
2. CH340 드라이버 재설치
3. USB 케이블 변경 (데이터 지원)
4. USB 포트 변경

### ❌ 빌드 오류 "PSRAM init failed"

**해결:**
```ini
; platformio.ini 확인
board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi
```

---

## 9️⃣ 다음 단계

### ✅ 개발환경 완료 후

1. **문서 읽기:**
   ```bash
   # 개발환경 상세 가이드
   code docs/DEVELOPMENT_ENVIRONMENT.md
   
   # WiFi 동작감지 가이드
   code docs/wifi-sensing-guide.md
   
   # BLE 복합 활용 가이드
   code docs/bluetooth-hybrid-guide.md
   ```

2. **프로젝트 확장:**
   - 02-environment-monitor (BME680 환경 센서)
   - 08-ble-heart-rate (BLE 심박수 수집)
   - 커스텀 프로젝트 생성

3. **GitHub 기여:**
   ```bash
   git checkout -b feature/my-feature
   # 코드 수정
   git add .
   git commit -m "feat: 새로운 기능 추가"
   git push origin feature/my-feature
   # Pull Request 생성
   ```

---

## 🔟 체크리스트

### 설치 완료

- [ ] VS Code 설치
- [ ] PlatformIO IDE 확장 설치
- [ ] Python 3.8+ 설치
- [ ] Git 설치
- [ ] USB 드라이버 설치 (CH340)

### 테스트 완료

- [ ] 저장소 클론
- [ ] 01-heart-rate 빌드 성공
- [ ] 업로드 성공
- [ ] 시리얼 모니터 동작
- [ ] LCD 에 출력 표시

### 개발 준비

- [ ] WiFi 설정 수정 (필요시)
- [ ] BLE MAC 주소 수정 (필요시)
- [ ] 첫 커밋 생성
- [ ] GitHub 푸시

---

## 📞 도움이 필요하신가요?

### 공식 문서
- [개발환경 상세 가이드](docs/DEVELOPMENT_ENVIRONMENT.md)
- [WiFi 동작감지 가이드](docs/wifi-sensing-guide.md)
- [BLE 복합 활용 가이드](docs/bluetooth-hybrid-guide.md)

### 커뮤니티
- [GitHub Issues](https://github.com/realizer-1966/esp32-edu-company/issues)
- [ESP32 Forum](https://esp32.com/)
- [PlatformIO Community](https://community.platformio.org/)

---

**축하합니다! 개발환경이 준비되었습니다.** 🎉

이제 ESP32-S3 로 헬스케어 디바이스를 개발할 수 있습니다!

**문의:** contact@esp32edutech.kr  
**GitHub:** https://github.com/realizer-1966/esp32-edu-company

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**검증 시간**: 5 분
