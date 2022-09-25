#include "SPI.h"
#include "MFRC522.h"

#define R 2
#define G 3
#define B 4

#define BUZZER 5

#define TRIG 6 //TRIG 핀 설정 (초음파 보내는 핀) 
#define ECHO 7 //ECHO 핀 설정 (초음파 받는 핀)

#define RST_PIN 8
#define SDA_PIN 10

MFRC522 mfrc522(SDA_PIN, RST_PIN); //인스턴스 설정
MFRC522::MIFARE_Key key;

int in; //자동차 들어와있는지 아닌지 flag
int idL; //id카드 들어와있는지 아닌지 flag
int idCheck();

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  pinMode(TRIG, OUTPUT); //초음파 보내는 핀
  pinMode(ECHO, INPUT); //초음파 받는 핀

}

void loop() 
{ 
  if ( in == 1 && idL == 0 ) {
    Serial.println("NO ID CARD"); //ID카드가 없는 차량일 경우
     analogWrite(R, 255),analogWrite(G, 150),analogWrite(B, 0); //주황색 불을 켠다
     in = inCheck();
     while (in == 1 && idL == 0){
      Serial.println("keep Yellow");
      tone(BUZZER, 700, 500); //나가라고 경고
      delay(1000);
      tone(BUZZER, 700, 500); //나가라고 경고
      in = inCheck();
      idL = idCheck();
      if (idL == 1) { //ID카드가 늦게 인식된 경우를 고려. 늦게라도 인식되면 빨간불이 되도록.
        analogWrite(R, 255),analogWrite(G, 0),analogWrite(B, 0);
        Serial.println("Go Welcome");
        return;
      }
     }
  }
  else if ( in == 1 && idL == 1 ) {
    Serial.println("Welcome"); //ID카드 있는 차량
     analogWrite(R, 255), analogWrite(G, 0), analogWrite(B, 0);
     while (in == 1){
        in = inCheck(); //차량이 있는지 확인
        if (in == 0){
          idL = 0;
          return;
        }
        Serial.println("keep WELCOME"); //차량이 있다면 = 주차중이라면 계속 빨간 불
        delay(5000);
     }
  }
  else if ( in == 0 && idL == 0) {
    Serial.println("No car in here"); //차량이 없는 경우 초록불을 유지
     analogWrite(R, 0),analogWrite(G, 255),analogWrite(B, 0);
  }
  else {
    Serial.println("error");
  } 
  
  //아래 부분은 반드시 위의 if-else if-else 구문 뒤에 작성되어야만 함.
  Serial.println("loop start");
  in = inCheck();
  
  if (in) {
    Serial.print("\nid_check:");
    idL = idCheck();
    Serial.println(idL);
  }
  
  Serial.print("\n\n");
  Serial.print("in, id ");
  Serial.print(in);
  Serial.print(idL);
  Serial.println();
  delay(1000);
}

int inCheck(){
  int check_in = 0;
  int cnt = 0;
  while ( check_in < 3 && cnt < 5 ) {
    if ( disCheck() <= 3 ) {
      check_in++;
      Serial.print("check: ");
      Serial.print(check_in);
      Serial.print("\n");
    }
    else {
      Serial.print("\n");
      cnt++;
    }
    delay(1000);
  }
  if ( check_in >= 3 ){
   return 1;
  }
  else {
    return 0;
  }
}

long disCheck() {
  long duration, distance;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(20);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  Serial.print("\nDura : ");
  distance = duration * 17 / 1000; 
  Serial.println(duration ); //초음파가 반사되어 돌아오는 시간을 보여줍니다.
  Serial.print("\nDIstance : ");
  Serial.print(distance); //측정된 물체로부터 거리값(cm값)을 보여줍니다.
  Serial.println(" cm");
  return distance;
}

int idCheck(){
  int id = 0;
  int id_cnt = 0;
  while ( id_cnt < 5){
    id_cnt++;
    if (!mfrc522.PICC_IsNewCardPresent()){
      id = 0;
     continue;
    }
    if (!mfrc522.PICC_ReadCardSerial()){
     id = 0;
     continue;
    }
  
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    
  
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) 
      {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
        id = 0;
      }
  
    String rfid = "";
    for (byte i = 0; i < 4; i++) {
      rfid +=
      (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") +
      String(mfrc522.uid.uidByte[i], HEX) +
      (i!=3 ? ":" : "");
    }
    
    Serial.print("Card key: ");
    Serial.println(rfid);
    id = 1;
    delay(1000);
    Serial.println("id checked");
  
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    break;
  }
  Serial.println("IDCHECK : ");
  Serial.print(id);
  return id;
}
