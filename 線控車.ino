#include <AccelStepper.h>
#include <SPI.h>
#include <MFRC522.h>

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
//搖桿
int xPin = 36;
int yPin = 39;
int zPin = 32; 
int xval = 0;     //Ｘ軸變數
int yval = 0;     //Ｙ軸變數
int zval = 0;     //Ｚ軸變數
//LED
int RedLED = 15;

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);
bool Left = false;            //判斷是否已左轉
bool Right = false;           //判斷是否已右轉

MFRC522 rfid(SDA_PIN,RST_PIN);
byte MyID[4] = {0x8C,0x10,0x1A,0x23};        //卡片ID
byte readID[4];                              //存放讀取到的卡片ID
bool Lock = true;                            //鎖定狀態

void setup() 
{
  //Serial.begin(9600);         //視窗
  //搖桿
  pinMode(zPin,INPUT_PULLUP);   //按鈕數位輸入有提昇電阻
  //步進
  stepper.setMaxSpeed(1000);            //最大速度 (步/秒)
  stepper.setAcceleration(500);         //加速度 (步/秒^2)
  stepper.setCurrentPosition(0);        //位置設為中心
  stepper.moveTo(0);                    //移動到起始位置(0)
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

void loop() 
{
  Rfid();
  //解鎖狀態時執行動作
  if(!Lock) {
    Action();
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
//執行動作函式
void Action()
{
  //搖桿
  xval = analogRead(xPin);     //讀取X軸數值
  yval = analogRead(yPin);     //讀取Y軸數值
  zval = digitalRead(zPin);    //讀取Z軸數值
  //TT
  if(yval<1600) {              //前進
    digitalWrite(IN5,LOW);
    digitalWrite(IN6,HIGH);
  }
  if(yval>2200) {              //後退
    digitalWrite(IN5, HIGH);
    digitalWrite(IN6, LOW);
  }
  if(yval>1600 && yval<2200) { //停
    digitalWrite(IN5, LOW);
    digitalWrite(IN6, LOW);
  }
  //步進
  if(xval<1400 && !Left) {     //左轉
    stepper.moveTo(-341);      //轉動30°所需的步數
    while(stepper.distanceToGo()!=0) {
      stepper.run();
    }
    Left=true;                 //zval==0才可以再次觸發左轉或右轉
    Right=true;                //zval==0才可以再次觸發左轉或右轉
  }
  if(xval>2200 && !Right) {    //右轉
    stepper.moveTo(341);       //轉動30°所需的步數
    while(stepper.distanceToGo()!=0) {
      stepper.run();
    }
    Left=true;                //zval==0才可以再次觸發左轉或右轉
    Right=true;               //zval==0才可以再次觸發左轉或右轉
  }
  if(zval==0) {               //中心重置
    stepper.moveTo(0);        //重設到中心位置(0°)
    while(stepper.distanceToGo()!=0) {
      stepper.run();
    }
    Left = false;           //重置左轉標誌
    Right = false;          //重置右轉標誌
  }

  /*char buf[100];
  sprintf(buf, "X=%d, Y=%d, Z=%d", xval, yval, zval);
  Serial.println(buf);
  delay(100);               //短暫延遲*/ 
}
//停止動作函式
void stopActions()
{
  digitalWrite(IN5,LOW);
  digitalWrite(IN6,LOW);
  stepper.moveTo(0);          //步進馬達回歸中心
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
