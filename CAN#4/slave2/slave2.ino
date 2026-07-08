/*
 * 예제127-2-3 (슬레이브2코드)
 * 슬레이브에 각각 스탭모터가 1개씩 장착되어있다
 * 마스터가 슬레이브에 장착된 스탭모터를 각각 제어할 수 있도록 해보자
 * (예시)
 * [command id] 0x00(양의방향) [스탭값1] [스탭값2] [나머지4개바이트는 사용안함]
 * [command id] 0x01(음의방향) [스탭값1] [스탭값2] [나머지4개바이트는 사용안함]
 * (스탭모터) 총2048스탭.. 최소스탭이 32스탭
 * 2048이 360도에 해당됨.. 32스탭이 5도정도 됨..
 * 슬레이브에 연결된 스탭모터는 IN1(D4),IN2(D5),IN3(D6),IN4(D7)
 */

#include <SPI.h>
#include <Stepper.h>
#include <mcp2515.h>

#define slave1 0x0F6
#define slave2 0x036

struct can_frame canMsg;
struct can_frame sendMsg;

const int stepsPerRevolution = 2048;
                                   //IN4 IN2 IN3 IN1
Stepper myStepper(stepsPerRevolution, 7, 5, 6, 4);
MCP2515 mcp2515(10);

void setup() {
  myStepper.setSpeed(10);
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

    //canMsg.data[0]~canMsg.data[7]
    //canMsg.data[0] : command_id
    //canMsg.data[1] : 부호(0x00:양, 0x01:음)
    //canMsg.data[2] : 값1(상위8비트)
    //canMsg.data[3] : 값2(하위8비트)
    //canMsg.data[4~7] : 사용안함
    unsigned int value = canMsg.data[2] * 256 + canMsg.data[3];
    if(canMsg.data[1] == 0x00){
      //양의방향
      myStepper.step(value);
    }else if(canMsg.data[1] == 0x01){
      //음의방향
      myStepper.step(-value);
    }
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();      
    //응답하는 부분
    sendMsg.can_id  = canMsg.can_id;
    sendMsg.can_dlc = canMsg.can_dlc;
    sendMsg.data[0] = canMsg.data[0];
    sendMsg.data[1] = '0';
    sendMsg.data[2] = 'K';
    sendMsg.data[3] = 0x00;
    sendMsg.data[4] = 0x00;
    sendMsg.data[5] = 0x00;
    sendMsg.data[6] = 0x00;
    sendMsg.data[7] = 0x00;
    mcp2515.sendMessage(&sendMsg);
  }
}