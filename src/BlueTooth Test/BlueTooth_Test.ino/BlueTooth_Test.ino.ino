// 블루투스 정상 작동을 테스트 하기위한 테스트 프로그램 

#include <SoftwareSerial.h>

int blueTx = 2; //Tx (보내는핀 설정)
int blueRx = 3; //Rx (받는핀 설정)
SoftwareSerial mySerial(blueTx, blueRx); //시리얼 통신을 위한 객체선언

void setup()  
{
  Serial.begin(9600);  //시리얼모니터
  mySerial.begin(9600); //블루투스 시리얼
}

void loop()
{
  if (mySerial.available()) {
    Serial.write(mySerial.read());  // 블루투스측 내용을 시리얼모니터에 출력
  }
  if (Serial.available()) {
    mySerial.write(Serial.read()); //시리얼 모니터 내용을 블루추스 측에 WRITE
  }
}

