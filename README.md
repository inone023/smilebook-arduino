# 🔌 SmileBook Arduino Firmware

**스마트 도서관 IoT 하드웨어 제어용 펌웨어 코드**입니다.  
RC522 RFID 리더기, PIR 센서, LED, 부저를 이용하여  
도서 대출·반납·도난 방지 로직을 제어합니다.

---

## ⚙️ Hardware Spec

| 구성 요소 | 역할 |
|------------|------|
| RC522 RFID Reader | 회원카드 및 도서 태그 인식 |
| PIR Sensor | 출입 감지 |
| LED (Red/Green) | 대출/반납 상태 표시 |
| Buzzer | 경고음 출력 |
| Wemos D1 R2 (ESP8266) | 서버 통신 및 제어 |

---

## 🧠 기능 개요

1. **출입 감지:**  
   PIR 센서가 움직임을 감지하면 RFID 리더 활성화 (20초 타이머)
2. **대출 감지:**  
   회원 태그 및 도서 태그 인식 → 서버로 요청 전송
3. **반납 감지:**  
   대출 상태의 도서 태그 인식 시 상태 ‘대출 가능’로 변경
4. **도난 방지:**  
   예약자 불일치 또는 미인증 시 빨간 LED + 부저 경고음 발생
5. **서버 통신:**  
   Spring Boot API로 HTTP 요청/응답 처리

---

## 📡 통신 구조

[Arduino RFID & PIR]
⇅ (HTTP)
[Spring Boot API Server]
⇅
[Android Client]

---

## 💡 개발 환경

| 항목 | 내용 |
|------|------|
| IDE | Arduino IDE 2.3.2 |
| Board | Wemos D1 R2 |
| Library | MFRC522, ESP8266WiFi, ArduinoJson |
| Protocol | HTTP (REST API) |

---

## ⚙️ 예시 동작

| 상태 | LED | 부저 |
|------|------|------|
| 대출 성공 | 🟢 Green ON | Silent |
| 반납 성공 | 🟢 Green ON | Silent |
| 인증 실패 / 도난 | 🔴 Red ON | 🚨 Alarm Sound |

---

## 🧾 관련 프로젝트
- [🌐 SmileBook-API (Server)](https://github.com/inone023/smilebook-api)
- [📱 SmileBook-Android (App)](https://github.com/inone023/smilebook-android)

---

© 2024 SmileBook Team. All rights reserved.
