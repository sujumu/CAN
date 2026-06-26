/*
 * 예제117-2-2 (슬레이브1코드)
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
#define job3 0x02
#define job4 0x03

#define sensor_pin 3

struct can_frame canMsg1;
MCP2515 mcp2515(10);
bool pre_sensor = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(sensor_pin, INPUT_PULLUP); //풀업방식
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
}

void loop() {
   //센서값이 생기면 마스터에게 보고만 하면 역할 끝
   bool sensor = digitalRead(sensor_pin);
   if(pre_sensor==HIGH && sensor == LOW){
    //적외선센서에 센싱이 된 그 찰나의 순간
    //마스터에게 보고서를 만들어서 제출
    send_can_message(slave1,job3,HIGH);
   }
   pre_sensor = sensor;
}

void send_can_message(int id, byte job, byte led){
  //임시위치
    //Request slave1
  canMsg1.can_id  = id; 
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
}