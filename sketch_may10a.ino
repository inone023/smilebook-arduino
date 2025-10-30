#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LED.h>

#define RST_PIN         D2  // Reset pin
#define SS_PIN          D4  // SDA pin

#define GREEN_LED_PIN   D3  // Green LED
#define RED_LED_PIN     D8  // Red LED
#define BUZZER_PIN      D5  // Buzzer

#define PIR_PIN         D1  // PIR sensor pin

const char* ssid = "Dosirak_4218989";
const char* password = "45173745";
const char* server = "3.39.9.175";
const int serverPort = 8080;

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient client;

struct BorrowRequest {
  String memberRFID;
  String bookRFID;
};

BorrowRequest borrowRequest;

bool sentRequest = false;
bool pirTriggered = false;
unsigned long pirCooldown = 0;
bool tagRead = false;
String lastTag = "";

void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  connectToWiFi();
}

void loop() {
  if (!sentRequest && digitalRead(PIR_PIN) == HIGH && millis() - pirCooldown > 5000) { // PIR motion detected and cooldown time passed
    Serial.println("PIR 동작 감지됨!"); // PIR 동작 감지 메시지 출력
    pirTriggered = true;
    sentRequest = true; // sentRequest를 true로 설정하여 중복 요청을 방지함
    borrowRequest.memberRFID = "";
    borrowRequest.bookRFID = "";

    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // RFID 리더기 초기화
    SPI.begin();
    rfid.PCD_Init();

    unsigned long startTime = millis();
    bool tagFound = false;

    // Wait for RFID tag within 20 seconds
    while (millis() - startTime < 30000 && !tagFound) {
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        String tagValue = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
          tagValue += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
          tagValue += String(rfid.uid.uidByte[i], HEX);
        }
        tagValue.toUpperCase();
        
        if (tagValue != lastTag) {
          lastTag = tagValue;
          if (tagValue == "47A29E63" || tagValue == "27BA9463") {
            borrowRequest.memberRFID = tagValue;
            Serial.println("회원 RFID가 설정되었습니다.");
          } else if (tagValue == "E7869D63" || tagValue == "E73B063" || tagValue == "3716A163" || tagValue == "27DDAD63" || tagValue == "572A9E63") {
            borrowRequest.bookRFID = tagValue;
            Serial.println("도서 RFID가 설정되었습니다.");
          }

          if (borrowRequest.memberRFID != "" && borrowRequest.bookRFID != "") {
            // Both RFID tags are set, break the loop
            tagFound = true;
          }
        }
      }
    }

    if (tagFound) {
      if (sendTagRequest(borrowRequest)) {
        Serial.println("회원 및 도서 RFID 태그를 서버에 성공적으로 보냈습니다.");
      } else {
        Serial.println("회원 및 도서 RFID 태그를 서버에 보내는 데 실패했습니다.");
      }
    } else {
      Serial.println("RFID 태그를 인식하지 못했습니다.");
      digitalWrite(RED_LED_PIN, HIGH); // 빨간 LED 켜기
      digitalWrite(GREEN_LED_PIN, LOW);
      tone(BUZZER_PIN, 1000, 500); // 부저 소리 작게 울리기
      delay(3000); // 1초 대기
      digitalWrite(RED_LED_PIN, LOW); // 빨간 LED 끄기
      noTone(BUZZER_PIN); // 부저 멈추기
    }

    pirCooldown = millis(); // Set cooldown time for PIR sensor
    pirTriggered = false; // Reset PIR trigger status
  }

  delay(1000); // Delay to avoid PIR sensor jitter
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

bool sendTagRequest(BorrowRequest request) {
  StaticJsonDocument<200> doc;
  doc["memberRFID"] = request.memberRFID;
  doc["bookRFID"] = request.bookRFID;

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  Serial.println("서버에 연결 중...");
  if (client.connect(server, serverPort)) {
    client.print("POST /api/borrow HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(server);
    client.print(":");
    client.print(serverPort);
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(measureJson(doc));
    client.print("\r\n\r\n");
    client.print(jsonBuffer);

    String response = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        response += c;
      }
    }
    client.stop();
    
    Serial.print("서버 응답: ");
    Serial.println(response);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
    if (response.indexOf("성공") != -1) {
      // 서버 응답이 성공일 때 LED 제어
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(RED_LED_PIN, LOW);
      noTone(BUZZER_PIN); // 부저 멈추기
    } else {
      // 서버 응답이 실패일 때 LED 및 부저 제어
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, HIGH);
      tone(BUZZER_PIN, 1000, 500); // 부저
      tone(BUZZER_PIN, 1000, 500); // 부저 울리기
    }

    return response.indexOf("성공") != -1;
  } else {
    Serial.println("서버에 연결할 수 없습니다.");
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 500); // 부저 울리기
    return false;
  }
}