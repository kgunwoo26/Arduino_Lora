#include <HX711.h>
#include "HX711.h"
#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define PING  1
#define PONG  2

#define CODE  PING    /* Please define PING or PONG */
#define calibration_factor -7050.0
#define TXpin 11
#define RXpin 10
#define trigPin 6
#define echoPin 7 
#define DOUT 3
#define CLK 2
#define ATSerial Serial

//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";  

HX711 scale(DOUT,CLK);

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

int ledPin = 5;
int buzzerPin = 8;

void setup() {
  ATSerial.begin(115200);
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(5, 0);
  lcd.print("IoT System");
  
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);
  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);

  /* SNIPE LoRa Initialization */
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }

  /* SNIPE LoRa Set Appkey */
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }
  
  /* SNIPE LoRa Set Frequency */
  if (!SNIPE.lora_setFreq(LORA_CH_1)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  /* SNIPE LoRa Set Spreading Factor */
  if (!SNIPE.lora_setSf(LORA_SF_7)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  /* SNIPE LoRa Set Rx Timeout 
   * If you select LORA_SF_12, 
   * RX Timout use a value greater than 5000  
  */
  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }  
    
  DebugSerial.println("SNIPE LoRa PingPong Test");

  scale.set_scale(calibration_factor);
  scale.tare();
}
/*
String distance(){
    digitalWrite(trigPin, LOW);
    digitalWrite(echoPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // echoPin 이 HIGH를 유지한 시간을 저장 한다.
    unsigned long duration = pulseIn(echoPin, HIGH); 
    // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
    float distance = ((float)(340 * duration) / 10000) / 2;
    return String(distance);
}
*/
String lbs() {
  double kg = scale.get_units() * 0.453592;
  return String(kg,2);
}

void loop() {
  
  //초음파 센서 코드
  digitalWrite(trigPin, LOW);
  digitalWrite(echoPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // echoPin 이 HIGH를 유지한 시간을 저장 한다.
  unsigned long duration = pulseIn(echoPin, HIGH); 
  // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
  float distance = ((float)(340 * duration) / 10000) / 2;

  //거리를 퍼센트로 계산
  int percent = ((40-distance)/40)*100;

  SNIPE.lora_send("B/" + String(distance) + "/" + lbs());
  DebugSerial.println("B/" + String(distance) + "/" + lbs());

  //뚜껑 위의 무게가 감지되면 부저 알람 + LED ON
  if(scale.get_units()>0.08){
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
  }

  //초음파로 거리를 측정하여 쓰레기 양을 LCD 모니터로 출력함
  if(percent > 60){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Percent: ");
    lcd.print(percent);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Weight: ");
    lcd.print(lbs());
    lcd.print("g");
  } else {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("GOOD");
  }

  //Lora 통신 delay
  delay(1000);

}
