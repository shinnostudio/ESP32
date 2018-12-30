/**
 * @file ESP32FBServoControll.h
 * @brief ESP32 Servo controll via Firebase
 * @author Shinnosuke Ishihara
 * @date 20 Dec 2018
 */

#include "ESP32_SSD1331.h"
#include "ESP32_SPIFFS_ShinonomeFNT.h"
#include "ESP32_SPIFFS_UTF8toSJIS.h"
#include <WiFi.h>
#include <Wire.h>
#include <FirebaseESP32.h>
#include <Adafruit_PWMServoDriver.h>

//Wi-fi
const char* ssid = "ssid here";
const char* password = "password here";

//Firebase
#define FIREBASE_HOST "firebase-host"
#define FIREBASE_AUTH "firebase-auth"

//Servo pulse length
#define SERVOMIN  150
#define SERVOMAX  600

//Servo Pins
#define SERVO_0_PORT 15

// Fonts(SPIFF)
ESP32_SPIFFS_ShinonomeFNT SFR;
const char* UTF8SJIS_file         = "/Utf8Sjis.tbl";  // UTF8 Shift_JIS 変換テーブル
const char* Shino_Zen_Font_file   = "/shnmk16.bdf";   // 全角フォント
const char* Shino_Half_Font_file  = "/shnm8x16.bdf";  // 半角フォント
 
// ESPr pins
const uint8_t SCLK_OLED   = 14; // SCLK
const uint8_t MOSI_OLED   = 13; // MOSI
const uint8_t MISO_OLED   = 12; // MISO
const uint8_t DC_OLED     = 26; // OLED DC
const uint8_t RST_OLED    = 25; // OLED Reset
const uint8_t CS1_OLED    = 15; // CS
const uint8_t I2C_SDA     = 21; // CS
const uint8_t I2C_SCL     = 22; // CS

// Analog seosor inputs
// const uint8_t PRESSURE_SENSOR = 4;

// LCD params
String    test_str;
uint8_t   test_buf[12][16] = {};
uint16_t  test_sj_length; 

// LCD mappings
uint8_t   H_Size, V_Size;   // H_Size(1,2,4) V_Size(1,2,4)
uint8_t   X0, X1, Y0, Y1;   // LCD表示座標
uint8_t   red, green, blue; // Max red=7, green=7, blue=3

// PWMServoDriver
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); 

// ESP32_SSD1331
ESP32_SSD1331 ssd1331(SCLK_OLED, MISO_OLED, MOSI_OLED, CS1_OLED, DC_OLED, RST_OLED);


void setup() {
  
  Serial.begin(115200);
  ssd1331Activation();
  wifiConnection(); 
  firebaseActivation();
  servoActivation();
}

void servoActivation() {
  
  Wire.begin(I2C_SDA,I2C_SCL);  // I2C
  pwm.begin();  
  pwm.setPWMFreq(60);           // 60 Hz
  servo_write(0,90);            // Set Default position
  delay(10);
}

void ssd1331Activation() {
  
  ssd1331.SSD1331_Init();
  SFR.SPIFFS_Shinonome_Init3F(UTF8SJIS_file, Shino_Half_Font_file, Shino_Zen_Font_file);
  Serial.println();  
}

void wifiConnection() {

  WiFi.begin(ssid, password); 
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP()); 
  delay(2000);
}

void firebaseActivation() {
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

 
void loop() {

  //String str = Firebase.getString("test");
  int angle = Firebase.getInt("angle");
  if(angle < 0 || angle > 180) { angle = 90; }

  displayLCD(String(angle) + " degree");
  servo_write(SERVO_0_PORT,angle);
  delay(1000);
}

void displayLCD(String val) {

  H_Size = 1; V_Size = 1;
  X0 = 0; Y0 = 0; X1 = 95;
  red = 7; green = 7; blue = 3;
  test_sj_length = SFR.StrDirect_ShinoFNT_readALL(val, test_buf);
  ssd1331.HVsizeUp_8x16_Font_DisplayOut(H_Size, V_Size, test_sj_length, X0, Y0, red, green, blue, test_buf);
}

void servo_write(int n, int angle) {
  
  angle = map(angle,0,180,SERVOMIN,SERVOMAX);
  pwm.setPWM(n,0,angle);
}
