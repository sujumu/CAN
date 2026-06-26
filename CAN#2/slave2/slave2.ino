/*
 * 예제117-2-3 (슬레이브2코드)
 * 마스터에 버튼2개가 달려있다
 * 첫번째 버튼은 D3에 연결되어있고 슬레이브 1의 LED를 제어한다
 * 두번째 버튼은 D4에 연결되어있고 슬레이브 2의 LED를 제어한다
 * 슬레이브에는 D3에 LED가 연결되어있다
 * 마스터가 슬레이브1과 슬레이브2에게 아래와 같이 전송한다
 * (예시) [아이디] [작업내용] [명령] [나머지 빈패킷]
 * 0x0F6 0x01 0x00 = ID가 0x0F6인것의 LED제어 명령인데 LED를 꺼라
 * 0x0F6 0x01 0x01 = ID가 0x0F6인것의 LED제어 명령인데 LED를 켜라
 * 0x036 0x01 0x01 = ID가 0x036인것의 LED제어 명령인데 LED를 켜라
 */

#include <SPI.h>
#include <mcp2515.h>

#define slave1 0x0F6
#define slave2 0x036

#define job1 0x00 //생존확인용 job
#define job2 0x01 //LED 제어용 job
#define job3 0x02
#define job4 0x03

#define myled 3

struct can_frame canMsg;
struct can_frame sendMsg;
MCP2515 mcp2515(10);

void setup() {
  Serial.begin(9600);
  pinMode(myled,OUTPUT);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    //자~~ 데이터 들어온다
    //내건지 확인을 해봐야겠다~
    if(canMsg.can_id != slave2) return; 

    //do something..
    if(canMsg.data[0] == job2){
      //LED 제어용 job
      digitalWrite(myled,HIGH);
      delay(2000);
      digitalWrite(myled,LOW);
    }
    
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