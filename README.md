# 🔧 안전한 모터 보안 제어 시스템 (Safe Motor Project)

RFID 기반 사용자 인증 + IR 리모컨 제어 + RTOS 구조를 활용해  
**인증된 사용자만 모터를 동작할 수 있도록 만든 보안 임베디드 시스템**입니다.

---

## 🎯 프로젝트 개요 및 목표

이 프로젝트는 **사용자 인증 기반 모터 제어 시스템**을 구축하는 것을 목표로 합니다.

### 🔑 주요 목표
- **RFID UID 기반 사용자 인증**  
  - 등록된 UID만 시스템 사용 가능  
  - 미등록 UID는 거부

- **IR 리모컨 모터 제어 시스템 구현**  
  - 1번: 정방향 / 2번: 역방향 / 3번: OFF  
  - 로그인·로그아웃 / 등록·삭제 모드 지원

- **RTOS 기반 안정적 구조 구현**  
  - Task 분리로 실시간성과 안정성 확보  
  - 멀티태스킹 기반 모듈화 구조

---

## 🛠️ 사용 부품 및 하드웨어 구성

| 부품 | 역할 | 연결 핀(STM32) |
|------|------|----------------|
| **STM32 Nucleo-F411RE** | 메인 MCU, 시스템 제어 | - |
| **RFID 리더기(MFRC522)** | UID 인증/등록 | PA5(SCK), PA6(MISO), PA7(MOSI), PB6(SDA), PA9(RST) |
| **IR 수신기** | 리모컨 입력 수신 | PB10(IR-SIG) |
| **스텝 모터 + ULN2003** | 모터 구동 | PA8(IN1), PB4(IN2), PB5(IN3), PB3(IN4) |

---

## ⚙️ 시스템 동작 흐름 (시나리오)
<img width="623" height="164" alt="image" src="https://github.com/user-attachments/assets/67ad27f2-39a0-40a9-bb72-a027e9e0b04c" />


### 🔐 1) 사용자 인증  
- RFID 태그 → UID 읽기 → 등록 여부 확인  
- 등록된 UID일 경우 **인증 상태 ON**

### 🔄 2) 모터 제어  
- IR 리모컨 입력에 따라 동작  
  - 🔹 **1번**: 모터 정방향  
  - 🔹 **2번**: 모터 역방향  
  - 🔹 **3번**: OFF  
- 인증 상태일 때만 동작 허용

### 🔓 3) 인증 해제  
- IR 리모컨 **5번 = 로그아웃**  
- 인증 상태 OFF

### 🛠️ 4) 등록/삭제(관리자 기능)
- **4번 버튼: 등록 모드**, RFID 태깅 → UID 저장  
- **Blue 버튼: 삭제 모드**, RFID 태깅 → UID 삭제

---

## 🧵 RTOS 기반 Task 구성

| Task | Priority | 역할 |
|------|----------|------|
| **defaultTask (Core)** | Normal | 시스템 메인제어, UID 관리, 모터 상태 관리 |
| **Motor** | Low | 모터 제어 |
| **IR** | Low | IR 수신 신호 처리 |
| **RC552 + UID_MGR** | Low | RFID 신호 분석 및 UID 저장/삭제 |

---

## 💻 소프트웨어 주요 로직
<img width="1943" height="1246" alt="image" src="https://github.com/user-attachments/assets/bfd9bc56-8825-406b-ba2a-a351264a6b7a" />

### 🧠 Core Logic
- 메시지 큐(`core_msg_queue`)를 기반으로 Task 간 이벤트 처리  
- 초기화: RFID, UID Manager, Motor Sleep, Timer 설정, IR 초기화  
- 이벤트 처리:
  - **RFID_LOGIN** → Motor_Awake()  
  - **RFID_LOGOUT** → Motor_Sleep()  
  - **RECEIVE_IR_SIG** →  
    - 좌회전(setRotateLeft)  
    - 우회전(setRotateRight)  
    - 모터 토글(MotorOnOffToggle)  
    - UID 등록 모드 / 삭제 모드  
    - 로그아웃

### 🔑 UID Manager Logic
- 모드: normal(0) / delete(1) / register(2)
- 기능:
  - UIDManager_Init(): 플래시 → 캐시 로딩  
  - UIDManager_IsRegistered(): UID 존재 확인  
  - UIDManager_SaveUID(): UID 등록  
  - UIDManager_DeleteUID(): UID 삭제  

---

## 🎬 시연 결과 (Summary)
<img width="328" height="188" alt="image" src="https://github.com/user-attachments/assets/3f6f78ab-7201-4dd1-9c5f-84c6ae9cb37b" />

- 등록/미등록 카드 동작 확인  
- 등록 모드 진입 → UID 저장 성공  
- 삭제 모드 진입 → UID 삭제 성공  
- 모터 제어  
  - 1번: CCW  
  - 2번: CW  
  - 3번: OFF  

---

## 🚀 향후 개선 방향

| 개선 항목 | 내용 | 기대 효과 |
|-----------|------|------------|
| UID 권한 구분 | 관리자/사용자 권한 레벨 도입 | 인증 제어의 유연성 증가 |
| 인증 타임아웃 | 일정 시간 후 자동 로그아웃 | 무단 사용 방지 / 보안 강화 |
| 로그 기록 기능 | UID 태깅, 인증 결과, 명령어 로그 기록 | 사용 이력 추적 가능 |

