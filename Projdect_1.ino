/* --- DUY SMART LOCK - BẢN SỬA LỖI MẤT TÍN HIỆU WEB --- */
#include <Wire.h>
#include "I2CKeyPad.h"
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// CẤU HÌNH CHÂN
#define LCD_ADDR 0x27
#define KEYPAD_ADDR 0x20
#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 6
#define BUZZER_PIN 8

// KHỞI TẠO
SoftwareSerial fingerSerial(A2, A3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
SoftwareSerial espSerial(A0, A1); // Giao tiếp ESP32
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Servo myServo;
I2CKeyPad keyPad(KEYPAD_ADDR);

char keymap[19] = "123A456B789C*0#D";
String password = "1234";
String inputPassword = "";

unsigned long lastCheckTime = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600); // Mở Serial Monitor để xem lỗi
  
  fingerSerial.begin(57600);
  espSerial.begin(9600);

  Wire.begin();
  keyPad.begin();

  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init(); lcd.backlight();
  myServo.attach(SERVO_PIN);
  myServo.write(0);

  lcd.print("SYSTEM READY");
  delay(1000);
  showIdle();
}

void loop() {
  // --- CHIẾN THUẬT MỚI: XOAY VÒNG LẮNG NGHE ---
  // Arduino Uno không thể nghe 2 nơi cùng lúc. 
  // Ta sẽ chia thời gian: Check Web -> Check Vân tay -> Check Web -> ...
  
  // 1. KIỂM TRA LỆNH TỪ WEB (ESP32) - Ưu tiên cao
  espSerial.listen(); // Bắt đầu nghe ESP32
  // Cho Arduino 50ms để "hóng" tin từ ESP32. 
  // Nếu không có delay này, dữ liệu chưa kịp tới đã bị chuyển kênh.
  delay(30); 
  
  if (espSerial.available() > 0) {
    char cmd = espSerial.read();
    Serial.print("Nhan duoc tu ESP32: "); Serial.println(cmd); // In ra để debug
    if (cmd == 'O') openDoor("LENH TU WEB");
  }

  // 2. KIỂM TRA VÂN TAY (Chỉ kiểm tra nhanh)
  fingerSerial.listen(); // Chuyển sang nghe Vân tay
  if (finger.getImage() == FINGERPRINT_OK) {
    if (finger.image2Tz() == FINGERPRINT_OK && finger.fingerFastSearch() == FINGERPRINT_OK) {
      openDoor("VAN TAY OK");
    } else {
      // accessDenied(); // Tạm tắt báo lỗi sai để đỡ ồn khi nhiễu
    }
  }

  // 3. RFID & Keypad (Không dùng SoftwareSerial nên chạy vô tư)
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    openDoor("THE TU OK");
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  if (keyPad.isPressed()) {
    uint8_t index = keyPad.getKey();
    if (index < 16) {
      char key = keymap[index];
      beep(1, 50);
      if (key == '#') {
        if (inputPassword == password) openDoor("MAT KHAU OK");
        else accessDenied();
        inputPassword = "";
      } else if (key == '*') {
        inputPassword = ""; showIdle();
      } else {
        inputPassword += key;
        lcd.setCursor(0, 1); lcd.print("Pass: ");
for(int i=0; i<inputPassword.length(); i++) lcd.print("*");
      }
    }
  }
}

void openDoor(String msg) {
  espSerial.println("OPEN_DOOR"); // Báo ngược lại cho Web
  beep(2, 100);
  lcd.clear(); lcd.print("MO CUA THANH CONG");
  lcd.setCursor(0, 1); lcd.print(msg);
  myServo.write(90);
  delay(5000);
  myServo.write(0);
  espSerial.println("LOCKED"); 
  showIdle();
}

void accessDenied() {
  beep(1, 1000);
  lcd.clear(); lcd.print("SAI THONG TIN!");
  delay(2000);
  showIdle();
}

void showIdle() {
  lcd.clear(); lcd.print("MOI XAC THUC...");
  lcd.setCursor(0,1); lcd.print("G3 Smart Lock");
}

void beep(int times, int duration) {
  for (int i=0; i<times; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(duration);
    digitalWrite(BUZZER_PIN, LOW); if (times > 1) delay(100);
  }
}
