# ESP32-S3 개발환경 설정 가이드

WAVESHARE ESP32-S3-Touch-LCD-4 보드를 활용한 헬스케어 디바이스 개발 환경 설정입니다.

---

## 1. 필수 소프트웨어

### 1.1 Visual Studio Code (추천)

**다운로드:** https://code.visualstudio.com/download

| OS | 링크 |
|----|------|
| Windows | https://code.visualstudio.com/Download#win |
| macOS | https://code.visualstudio.com/Download#mac |
| Linux | https://code.visualstudio.com/Download#linux |

**설치 후 확장 프로그램:**
1. **PlatformIO IDE** (필수)
2. **C/C++** (Microsoft)
3. **Serial Device Monitor**
4. **GitLens**

### 1.2 Python 3.x

**ESP32 Arduino Core 및 PlatformIO 에 필요**

#### Windows
```powershell
# winget 사용 (권장)
winget install Python.Python.3.11

# 또는 공식 인스톨러
# https://www.python.org/downloads/windows/
```

#### macOS
```bash
# Homebrew 사용
brew install python@3.11
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install python3 python3-pip python3-venv
```

**확인:**
```bash
python3 --version  # Python 3.8 이상 필요
```

### 1.3 Git

**버전 관리 및 GitHub 연동**

#### Windows
```powershell
winget install Git.Git
```

#### macOS
```bash
xcode-select --install
```

#### Linux
```bash
sudo apt install git
```

**확인:**
```bash
git --version
```

---

## 2. PlatformIO 설치

### 2.1 VS Code 에서 설치 (권장)

1. **VS Code 실행**
2. **확장 프로그램 탭** (Ctrl+Shift+X)
3. **"PlatformIO IDE" 검색**
4. **설치** (5-10 분 소요)
5. **재시작**

### 2.2 CLI 에서 설치 (대안)

```bash
# pip 사용
pip install platformio

# 또는
python3 -m pip install --upgrade pip
pip install platformio
```

**확인:**
```bash
pio --version
```

---

## 3. ESP32 Arduino Core 설정

PlatformIO 가 자동으로 설치합니다. 수동 설치가 필요한 경우:

### 3.1 platformio.ini 설정

```ini
[env:esp32-s3-touch-lcd-4]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

board_build.arduino.memory_type = qio_opi
board_build.flash_size = 8MB
board_build.psram_type = opi

lib_deps = 
  lovyan/LovyanGFX@^1.1.12
```

### 3.2 라이브러리 설치

```bash
cd 프로젝트폴더
pio lib install
```

---

## 4. WAVESHARE ESP32-S3-Touch-LCD-4 드라이버

### 4.1 USB 드라이버 (CH340/CP210x)

#### Windows
1. **CH340 드라이버 다운로드**
   - https://www.wch.cn/downloads/CH341SER_ZIP.html
2. **설치**
3. **장치 관리자에서 확인**
   - 포트 (COM & LPT) → USB-SERIAL CH340

#### macOS
```bash
# 최신 macOS 는 기본 지원
# 구버전은 드라이버 설치
# https://www.wch.cn/downloads/CH341SER_MAC_ZIP.html
```

#### Linux
```bash
# 기본 지원됨
ls /dev/ttyUSB*  # CH340
ls /dev/ttyACM*  # CP210x
```

### 4.2 포트 권한 설정 (Linux)

```bash
# 사용자 그룹 추가
sudo usermod -a -G dialout $USER

# 또는 udev 규칙
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="1a86", MODE="0666"' | \
  sudo tee /etc/udev/rules.d/50-ch340.rules

sudo udevadm control --reload-rules
sudo udevadm trigger
```

**재부팅 후 확인:**
```bash
ls -l /dev/ttyUSB*
```

---

## 5. 프로젝트 구조

### 5.1 표준 구조

```
esp32-edu-company/
├── firmware/
│   └── waveshare-s3-examples/
│       ├── 01-heart-rate/
│       │   ├── src/
│       │   │   └── main.cpp
│       │   ├── include/
│       │   ├── lib/
│       │   ├── test/
│       │   ├── platformio.ini
│       │   └── README.md
│       ├── 05-wifi-presence/
│       ├── 07-ble-presence/
│       └── ...
├── hardware/
│   ├── schematics/
│   └── pcb/
├── docs/
├── .gitignore
└── README.md
```

### 5.2 PlatformIO 프로젝트 생성

```bash
# CLI 에서 생성
pio project create --board esp32-s3-devkitc-1 --name my-project

# 또는 VS Code 에서
# PlatformIO Home → New Project
```

---

## 6. 코드 업로드

### 6.1 PlatformIO 사용

```bash
cd firmware/waveshare-s3-examples/01-heart-rate

# 빌드
pio run

# 업로드
pio run --target upload

# 시리얼 모니터
pio device monitor --baud 115200

# 빌드 + 업로드 + 모니터
pio run --target upload && pio device monitor
```

### 6.2 VS Code 사용

1. **프로젝트 폴더 열기**
2. **하단 PlatformIO 툴바**
3. **아이콘 클릭:**
   - ✓ → 빌드
   - → → 업로드
   - 🔌 → 시리얼 모니터

---

## 7. 디버깅

### 7.1 시리얼 디버깅

```cpp
void setup() {
  Serial.begin(115200);
  
  Serial.println("디버깅 시작");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
  Serial.println("루프 실행중");
  delay(1000);
}
```

**모니터 실행:**
```bash
pio device monitor --baud 115200
```

**종료:** `Ctrl+C` (Linux/macOS), `Ctrl+Break` (Windows)

### 7.2 로그 레벨

```cpp
// platformio.ini
build_flags = 
  -DCORE_DEBUG_LEVEL=5  # 0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=VERBOSE
```

```cpp
// 코드에서
ESP_LOGE("TAG", "Error message");
ESP_LOGW("TAG", "Warning message");
ESP_LOGI("TAG", "Info message");
ESP_LOGD("TAG", "Debug message");
ESP_LOGV("TAG", "Verbose message");
```

### 7.3 예외 디코더

```bash
# platformio.ini
monitor_filters = esp32_exception_decoder
```

**예외 발생 시:**
```
Guru Meditation Error: Core 1 panic'ed (LoadStoreError)
Backtrace: 0x4008e8f2:0x3ffb3a50 0x4008e9f5:0x3ffb3a70
```

PlatformIO 가 자동으로 심볼릭 주소로 변환합니다.

---

## 8. 문제 해결

### 8.1 업로드 실패

#### 증상 1: "Failed to connect to ESP32-S3"

**해결:**
```bash
# 부트모드 진입
1. 보드의 BOOT 버튼 누름
2. RESET 버튼 누름
3. BOOT 버튼 놓음
4. RESET 버튼 놓음
5. 업로드 재시도
```

#### 증상 2: "Permission denied" (Linux)

**해결:**
```bash
# 포트 권한
sudo chmod 666 /dev/ttyUSB0

# 또는 사용자 그룹 추가 (영구)
sudo usermod -a -G dialout $USER
# 재부팅 필요
```

#### 증상 3: "COM 포트 없음" (Windows)

**해결:**
1. 장치 관리자 확인
2. 드라이버 재설치
3. USB 케이블 변경 (데이터 지원 케이블)
4. USB 포트 변경

### 8.2 빌드 오류

#### 증상: "PSRAM init failed"

**해결:**
```ini
; platformio.ini
board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi
```

#### 증상: "LovyanGFX compilation error"

**해결:**
```ini
; platformio.ini
lib_deps = lovyan/LovyanGFX@^1.1.12

; 또는 최신 버전
; lib_deps = lovyan/LovyanGFX@^1.1.16
```

### 8.3 디스플레이 출력 안 됨

**확인사항:**
1. 백라이트 핀 (GPIO 3) PWM 설정
2. 핀아웃 정확성 (CS, DC, RST)
3. SPI 속도 (40MHz → 20MHz 로 낮춤)

```cpp
// LovyanGFX 설정 확인
cfg.freq_write = 40000000;  // 40MHz
```

---

## 9. 최적화 설정

### 9.1 빌드 최적화

```ini
; platformio.ini
build_flags = 
  -O2  ; 최적화 레벨 (0, 1, 2, 3, s)
  -ffunction-sections
  -fdata-sections
  
board_build.flash_mode = qio
board_build.f_flash = 80000000L
```

### 9.2 파티션 설정

```ini
; 8MB 플래시용
board_build.partitions = default_8MB.csv

; 또는 커스텀
board_build.partitions = partitions.csv
```

**partitions.csv 예제:**
```csv
Name,Type,SubType,Offset,Size,Flags
nvs,data,nvs,0x9000,0x6000,
phy_init,data,phy,0xf000,0x1000,
factory,app,factory,0x10000,0x300000,
storage,data,spiffs,0x310000,0x400000,
```

### 9.3 디버깅 최적화

```ini
; debug 모드
build_type = debug

; 릴리스 모드
build_type = release
```

---

## 10. CI/CD 설정 (선택)

### 10.1 GitHub Actions

`.github/workflows/platformio.yml`:
```yaml
name: PlatformIO CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.11'
      
      - name: Install PlatformIO
        run: |
          python -m pip install --upgrade pip
          pip install platformio
      
      - name: Build Firmware
        run: |
          cd firmware/waveshare-s3-examples/01-heart-rate
          pio run
      
      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: firmware
          path: firmware/waveshare-s3-examples/01-heart-rate/.pio/build/esp32-s3-devkitc-1/firmware.bin
```

### 10.2 자동 포맷

```bash
# clang-format 설치
# .clang-format 파일 생성

# VS Code 확장: C/C++
# 포맷 on save 활성화
```

---

## 11. 추천 개발 워크플로우

### 11.1 일상 개발

```bash
# 1. 코드 수정
code firmware/waveshare-s3-examples/01-heart-rate/src/main.cpp

# 2. 빌드
cd firmware/waveshare-s3-examples/01-heart-rate
pio run

# 3. 업로드
pio run --target upload

# 4. 시리얼 모니터
pio device monitor

# 5. Git 커밋
git add .
git commit -m "feat: 심박수 측정 알고리즘 개선"
git push
```

### 11.2 디버깅

```bash
# 1. 디버그 빌드
pio run --target debug

# 2. 시리얼 모니터 (상세 로그)
pio device monitor --baud 115200 --filter esp32_exception_decoder

# 3. 로그 분석
# VS Code 에서 PlatformIO Serial Monitor 사용
```

### 11.3 릴리스

```bash
# 1. 버전 업데이트
# platformio.ini: version = 1.0.0

# 2. 릴리스 빌드
pio run --target release

# 3. 바이너리 확인
ls -lh .pio/build/esp32-s3-devkitc-1/firmware.bin

# 4. Git 태그
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# 5. GitHub Release 생성
gh release create v1.0.0 \
  .pio/build/esp32-s3-devkitc-1/firmware.bin \
  --title "v1.0.0 - Initial Release" \
  --notes "첫 번째 릴리스"
```

---

## 12. 체크리스트

### 설치 완료 확인

- [ ] VS Code 설치
- [ ] PlatformIO IDE 확장 설치
- [ ] Python 3.8+ 설치
- [ ] Git 설치
- [ ] USB 드라이버 설치 (CH340)
- [ ] 포트 권한 설정 (Linux)

### 테스트 프로젝트

- [ ] 01-heart-rate 빌드 성공
- [ ] 업로드 성공
- [ ] 시리얼 모니터 동작
- [ ] LCD 에 심박수 표시
- [ ] GitHub 푸시 성공

### 개발 준비

- [ ] Git 저장소 클론
- [ ] PlatformIO 라이브러리 설치
- [ ] WiFi 설정 수정
- [ ] 첫 커밋 생성

---

## 13. 추가 리소스

### 공식 문서
- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [WAVESHARE ESP32-S3-Touch-LCD-4](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4)

### 커뮤니티
- [ESP32 Forum](https://esp32.com/)
- [PlatformIO Community](https://community.platformio.org/)
- [Reddit r/esp32](https://www.reddit.com/r/esp32/)

### 튜토리얼
- [ESP32 시작하기](https://randomnerdtutorials.com/projects-esp32/)
- [PlatformIO 튜토리얼](https://docs.platformio.org/en/latest/tutorials/index.html)

---

**최종 수정일**: 2026-08-18  
**작성자**: CTO 용수 박  
**검증**: Windows 11, macOS Sonoma, Ubuntu 22.04
