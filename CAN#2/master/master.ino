/*
 * 예제117-2-1 (마스터코드)
 * 마스터는 중계역할을 한다
 * 슬레이브1(0x0F6)의 D3포트에 적외선근접센서가 장착되어있다.
 * 슬레이브2(0x036)의 D3포트에 LED가 장착되어있다.
 * (예시) [아이디] [작업내용] [명령] [나머지 빈패킷]
 * (수신)0x0F6 0x02 0x01 = ID가 0x0F6에서 적외선 근접센서 신호가 마스터로 수신됨
 * (송신)0x036 0x01 0x01 = ID가 0x036인것의 LED제어 명령인데 LED를 켜라
 * (송신)0x036 0x01 0x00 = ID가 0x036인것의 LED제어 명령인데 LED를 꺼라
 */

#include <SPI.h>
#include <mcp2515.h>

#define slave1 0x0F6
#define slave2 0x036

#define job1 0x00 //생존확인용 job
#define job2 0x01 //LED 제어용 job
#define job3 0x02 //적외선근접센서용 job
#define job4 0x03

struct can_frame canMsg;
struct can_frame canMsg1;
struct can_frame canMsg2;
MCP2515 mcp2515(10);

bool pre_button1_state = HIGH; //풀업이므로
bool pre_button2_state = HIGH; //풀업이므로

void setup() {
  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Write to CAN");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

    if(canMsg.can_id == slave1 && canMsg.data[0] == job3){
      //슬레이브1이 보낸 데이터가 확실하구나..
      Serial.println("슬레이브2로 신호 전송");
      send_can_message(slave2,job2,HIGH);
    }
    /*
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();
    */
  }

}

void send_can_message(int id, byte job, byte led){
  //임시위치
    //Request slave1
  canMsg1.can_id  = id; //슬레이브1의 ID
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = job;
  canMsg1.data[1] = led; //LED ON
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x00;
  canMsg1.data[7] = 0x00; 
  mcp2515.sendMessage(&canMsg1);
  Serial.print("ID=");
  Serial.println(canMsg1.can_id,HEX);
  Serial.println("[마스터에서 슬레이브1로 보낸 메시지]");
  for (int i = 0; i<canMsg1.can_dlc; i++)  {  // print the data
      Serial.print(canMsg1.data[i],HEX);
      Serial.print(" ");
  }
  Serial.println();
  //Response
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.println("[마스터의 응답데이터]");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();
  }
}
