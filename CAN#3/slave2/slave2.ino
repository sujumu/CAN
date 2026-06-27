/*
 * 예제123-2-3 (슬레이브2코드)
 * 슬레이브1에 온습도센서(DHT11)가 있다
 * 슬레이브2에 먼지센서(GP2Y)가 있다
 * 슬레이브1과 2의 ID는 define에 참조
 * command id는 온습도가 0x0D이고 먼지센서는 0x0F로 하자
 * (예시) 0x0F6 0x0D 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 
 * → ID가 0x0F6인 슬레이브에게 온습도 센서값을 요청하는 패킷
 * (예시) 0x036 0x0F 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 
 * → ID가 0x036인 슬레이브에게 먼지센서값을 요청하는 패킷
 */

#include <SPI.h>
#include <mcp2515.h>

#define slave1 0x0F6
#define slave2 0x036

int measurePin = A0;
int ledPower = 3; 
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

struct can_frame canMsg;
struct can_frame sendMsg;
MCP2515 mcp2515(10);

void setup() {
  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    //자~~ 데이터 들어온다
    //내건지 확인을 해봐야겠다~
    if(canMsg.can_id != slave2) return; 
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();      
    //응답하는 부분
    sendMsg.can_id  = canMsg.can_id;
    sendMsg.can_dlc = canMsg.can_dlc;
    sendMsg.data[0] = canMsg.data[0];

    //값을 측정
    float dust = dustsensor();
    //0.00~ ????.00 ... *100
    //0~ ????00  //655.35까지 표현가능
    sendMsg.data[1] = (int)(dust*100) / 256;
    sendMsg.data[2] = (int)(dust*100) % 256;
    sendMsg.data[3] = 0x00;
    sendMsg.data[4] = 0x00;
    sendMsg.data[5] = 0x00;
    sendMsg.data[6] = 0x00;
    sendMsg.data[7] = 0x00;
    mcp2515.sendMessage(&sendMsg);
  }
}

float dustsensor(){
  float voMeasured = 0;
  float calcVoltage = 0;
  float dustDensity = 0;
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin); // read the dust value
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (5.0 / 1024.0);
  dustDensity = 0.17 * calcVoltage - 0.1;
  return dustDensity;
}
