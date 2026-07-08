/*
 * 예제127-2-1 (마스터코드)
 * 슬레이브에 각각 스탭모터가 1개씩 장착되어있다
 * 마스터가 슬레이브에 장착된 스탭모터를 각각 제어할 수 있도록 해보자
 * (예시)
 * [command id] 0x00(양의방향) [스탭값1] [스탭값2] [나머지4개바이트는 사용안함]
 * [command id] 0x01(음의방향) [스탭값1] [스탭값2] [나머지4개바이트는 사용안함]
 * (스탭모터) 총2048스탭.. 최소스탭이 32스탭
 * 2048이 360도에 해당됨.. 32스탭이 5도정도 됨..
 * 마스터에 2축 조이스틱이 장착되었다.
 * x축을 A0포트에 y축을 A1포트에 연결해서 사용하자!
 * x축을 움직이면 슬레이브1의 스탭모터가 제어되고
 * y축을 움직이면 슬레이브2의 스탭모터가 제어된다.
 * 
 */

#include <SPI.h>
#include <mcp2515.h>

#define slave1 0x0F6
#define slave2 0x036

#define job1 0x00 //생존확인용 job
#define job2 0x01
#define job3 0x02
#define job4 0x03

#define xpin A0
#define ypin A1

struct can_frame canMsg2;
MCP2515 mcp2515(10);


void setup() {
  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Write to CAN");
}

void loop() {
  //Request slave1
         //command_id, 부호 , 값1, 값2
  int x = analogRead(xpin);
  int y = analogRead(ypin);
  if(x > 700){
    //조이스틱의 x축이 양의방향(오른쪽)
    Serial.println("오른쪽");
    byte data[8] = {job1,0x00,0x00,128,0x00,0x00,0x00,0x00}; 
                 //양의방향으로 20도씩 움직여라
    mcp2515_send(slave1,data);

    //Response slave1
    byte recv[8];
    unsigned int id = mcp2515_receive(recv);
    if(id == -1){
      Serial.println("슬레이브1 오프라인!");
    }else{
      //데이터 정상수신
      //프린트
      Serial.print("수신ID=");
      Serial.println(id,HEX);
      for(int i = 0;i<8;i++){
        Serial.print(recv[i]);
        Serial.print(",");
      }
      Serial.println();
    }
  }else if(x<300){
    //왼쪽
    Serial.println("왼쪽");
    byte data[8] = {job1,0x01,0x00,128,0x00,0x00,0x00,0x00}; 
                 //양의방향으로 20도씩 움직여라
    mcp2515_send(slave1,data);

    //Response slave1
    byte recv[8];
    unsigned int id = mcp2515_receive(recv);
    if(id == -1){
      Serial.println("슬레이브1 오프라인!");
    }else{
      //데이터 정상수신
      //프린트
      Serial.print("수신ID=");
      Serial.println(id,HEX);
      for(int i = 0;i<8;i++){
        Serial.print(recv[i]);
        Serial.print(",");
      }
      Serial.println();
    }
  }
  if(y > 700){
    //아래쪽
    Serial.println("아래쪽");
    //Request slave2
    byte data2[8] = {job1,0x01,0x00,128,0x00,0x00,0x00,0x00}; 
                  //양의방향으로 20도씩 움직여라
    mcp2515_send(slave2,data2);

    //Response slave2
    byte recv[8];
    unsigned int id = mcp2515_receive(recv);
    if(id == -1){
      Serial.println("슬레이브2 오프라인!");
    }else{
      //데이터 정상수신
      //프린트
      Serial.print("수신ID=");
      Serial.println(id,HEX);
      for(int i = 0;i<8;i++){
        Serial.print(recv[i]);
        Serial.print(",");
      }
      Serial.println();
    }
  }else if(y<300){
    //위쪽
    Serial.println("위쪽");
    //Request slave2
    byte data2[8] = {job1,0x00,0x00,128,0x00,0x00,0x00,0x00}; 
                  //양의방향으로 20도씩 움직여라
    mcp2515_send(slave2,data2);

    //Response slave2
    byte recv[8];
    unsigned int id = mcp2515_receive(recv);
    if(id == -1){
      Serial.println("슬레이브2 오프라인!");
    }else{
      //데이터 정상수신
      //프린트
      Serial.print("수신ID=");
      Serial.println(id,HEX);
      for(int i = 0;i<8;i++){
        Serial.print(recv[i]);
        Serial.print(",");
      }
      Serial.println();
    }
  }
}


void mcp2515_send(unsigned int id, byte data[]){
  struct can_frame canMsg1;
  canMsg1.can_id  = id; //슬레이브1의 ID
  canMsg1.can_dlc = 8;
  for(int i =0;i<8;i++){
    canMsg1.data[i] = data[i];
  }
  mcp2515.sendMessage(&canMsg1);
  Serial.println("[마스터에서 슬레이브로 보낸 메시지]");
  Serial.println(id,HEX);
  for (int i = 0; i<canMsg1.can_dlc; i++)  {  // print the data
      Serial.print(canMsg1.data[i],HEX);
      Serial.print(" ");
  }
  Serial.println();
}
unsigned int mcp2515_receive(byte recv[]){
  struct can_frame canMsg;
  unsigned long t = millis(); //이 타이밍의 시간을 측정
  while(1){
    if(millis() - t > 3000){
      //루프 진입후 3초가 지난 시점
      //Serial.println("슬레이브1 오프라인");
      return -1;
      break;
    }
    //나 슬레이브1의 응답데이터가 있을때까지 무조건 기다릴거야!
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
      for(int i =0;i<8;i++){
        recv[i] = canMsg.data[i];
      }
      return canMsg.can_id;
      break;
    }
  }
}
