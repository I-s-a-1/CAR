#include <AccelStepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;  //藍牙序列

//步進馬達
#define IN1 16
#define IN2 17
#define IN3 21
#define IN4 25
//TT馬達
#define IN5 12
#define IN6 13
//RFID
//SCK-18 MOSI-23 MISO-19
#define SDA_PIN 5
#define RST_PIN 21 
//LED
int RedLED = 15;

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);
bool Left = false;            //判斷是否已左轉
bool Right = false;           //判斷是否已右轉

MFRC522 rfid(SDA_PIN, RST_PIN);
byte MyID[4] = {0x8C,0x10,0x1A,0x23};        //卡片ID
byte readID[4];                              //存放讀取到的卡片ID
bool Lock = true;                            //鎖定

void setup() 
{
  //Serial.begin(9600);         //視窗監控
  //藍牙
  SerialBT.begin("MyESP32Car"); //名稱
  Serial.println("Bluetooth started. Waiting for connections...");
  //步進
  stepper.setMaxSpeed(1000);            //最大速度 (步/秒)
  stepper.setAcceleration(500);         //加速度 (步/秒^2)
  stepper.setCurrentPosition(0);        //位置設為中心
  stepper.moveTo(0);                    //移動到起始位置(0°)
  while(stepper.distanceToGo()!=0)
    stepper.run();
  //TT
  pinMode(IN5,OUTPUT);
  pinMode(IN6,OUTPUT);
  digitalWrite(IN5,LOW);     //停止
  digitalWrite(IN6,LOW);     //停止
  //RFID
  SPI.begin();
  rfid.PCD_Init();
  //Serial.println("請刷卡...");
  //LED
  pinMode(RedLED,OUTPUT);
}

void loop() {
  Rfid();
  //解鎖時執行動作
  if (!Lock) {
    if (SerialBT.available()) {         //藍牙序列埠有資料傳入
      char BT = SerialBT.read();       //讀取一個字元作為指令
      Bluetooth(BT);               //呼叫藍牙指令函式，根據收到的指令做出動作
    }
  }
}

//RFID感應函式
void Rfid()
{
  //是否有卡片靠近
  if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    memcpy(readID,rfid.uid.uidByte,rfid.uid.size);   //複製ID到readID陣列
    //卡片ID正確
    if(isMyCard(readID,rfid.uid.size)) {
      //解鎖
      if(Lock) {
       //Serial.println("解鎖成功！");
        Lock = false;
        digitalWrite(RedLED,HIGH);
        delay(1000);
        digitalWrite(RedLED,LOW);
      }
      //鎖定
      else {
        //Serial.println("鎖定！！");
        stopActions();         //停止動作
        Lock = true;
        for(int i=0;i<2;i++) {
          digitalWrite(RedLED,HIGH);
          delay(400);
          digitalWrite(RedLED,LOW);
          delay(400);
        }
      }
      delay(1000);             //防止多次刷卡
    }
    //卡片ID錯誤
    else {
      //Serial.println("卡片不符合");
      for(int i=0;i<5;i++) {
          digitalWrite(RedLED,HIGH);
          delay(200);
          digitalWrite(RedLED,LOW);
          delay(200);
      }
    }
    rfid.PICC_HaltA();         //停止卡片偵測
  }
}

//藍牙指令函式
void Bluetooth(char BT)
{
  switch (BT) {
    case 'F':  //前進
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, HIGH);
      break;
    case 'B':  //後退
      digitalWrite(IN5, HIGH);
      digitalWrite(IN6, LOW);
      break;
    case 'S':  //停止前進或後退
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, LOW);
      break;
    case 'L':  //左轉
      if (!Left) {
        stepper.moveTo(-341);
        while (stepper.distanceToGo() != 0) {
          stepper.run();
        }
        Left = true;
        Right = true;
      }
      break;
    case 'R':  //右轉
      if (!Right) {
        stepper.moveTo(341);
        while (stepper.distanceToGo() != 0) {
          stepper.run();
        }
        Left = true;
        Right = true;
      }
      break;
    case 'M':  //回到中心
      stepper.moveTo(0);
      while (stepper.distanceToGo() != 0) {
        stepper.run();
      }
      Left = false;
      Right = false;
      break;
  }
}
//停止動作函式
void stopActions()
{
  digitalWrite(IN5,LOW);
  digitalWrite(IN6,LOW);
  stepper.moveTo(0);          //回到中心
  while(stepper.distanceToGo()!=0) {
    stepper.run();
  }
}
//判斷ID
bool isMyCard(byte *id, byte length) {
  if(length!=4) 
    return false;            //確保ID長度正確
  for(byte i=0;i<4;i++) {
    if(id[i]!=MyID[i]) {
      return false;          //如果任一位元組不匹配，返回false
    }
  }
  return true;               //ID完全匹配，返回true
}
