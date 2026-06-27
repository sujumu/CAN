/*
 * 예제123-2-1 (마스터코드)
 * 마스터에 0.91인치 OLED 디스플레이를 연결하자
 * SDA를 A4에 SCL을 A5에 연결하면 I2C 통신이 된다!
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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     4 
#define slave1 0x0F6
#define slave2 0x036

#define dhtcommand 0x0D
#define dustcommand 0x0F

struct can_frame recvMsg1;
struct can_frame recvMsg2;
struct can_frame canMsg1;
struct can_frame canMsg2;
MCP2515 mcp2515(10);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  canMsg1.can_id  = 0x0F6; //슬레이브1의 ID
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = dhtcommand;
  canMsg1.data[1] = 0x00;
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x00;
  canMsg1.data[7] = 0x00;

  canMsg2.can_id  = 0x036; //슬레이브2의 ID
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = dustcommand;
  canMsg2.data[1] = 0x00;
  canMsg2.data[2] = 0x00;
  canMsg2.data[3] = 0x00;
  canMsg2.data[4] = 0x00;
  canMsg2.data[5] = 0x00;
  canMsg2.data[6] = 0x00;
  canMsg2.data[7] = 0x00;
  
  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Write to CAN");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);        
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  
  //Request slave1
  mcp2515.sendMessage(&canMsg1);
  Serial.println("[마스터에서 슬레이브1로 보낸 메시지]");
  for (int i = 0; i<canMsg1.can_dlc; i++)  {  // print the data
      Serial.print(canMsg1.data[i],HEX);
      Serial.print(" ");
  }
  Serial.println();
  //Response
  delay(50);
  if (mcp2515.readMessage(&recvMsg1) == MCP2515::ERROR_OK) {
    Serial.println("[슬레이브1의 응답데이터]");
    //Serial.print(canMsg.can_id, HEX); // print ID
    //Serial.print(" "); 
    //Serial.print(canMsg.can_dlc, HEX); // print DLC
    //Serial.print(" ");
    if(recvMsg1.data[0] != dhtcommand){
      Serial.println("NO DHT DATA!");
      return;
    }
    for (int i = 0; i<recvMsg1.can_dlc; i++)  {  // print the data
      Serial.print(recvMsg1.data[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    //canMsg.data[1] ...[2]  온도
    //canMsg.data[3] ...[4]  습도
    float t = ((recvMsg1.data[1] * 256 + recvMsg1.data[2])-10000) / 100.0f;
    float h = ((recvMsg1.data[3] * 256 + recvMsg1.data[4])) / 100.0f;
    Serial.print("온도=");
    Serial.print(t);
    Serial.print(", 습도=");
    Serial.println(h);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(t);
    display.print("'C, ");
    display.print(h);
    display.println("%");
    display.display();
  }
  //Request slave1
  mcp2515.sendMessage(&canMsg2);
  Serial.println("[마스터에서 슬레이브2로 보낸 메시지]");
  for (int i = 0; i<canMsg2.can_dlc; i++)  {  // print the data
      Serial.print(canMsg2.data[i],HEX);
      Serial.print(" ");
  }
  Serial.println();
  delay(50);
  //Response
  if (mcp2515.readMessage(&recvMsg2) == MCP2515::ERROR_OK) {
    Serial.println("[슬레이브2의 응답데이터]");
    //Serial.print(canMsg.can_id, HEX); // print ID
    //Serial.print(" "); 
    //Serial.print(canMsg.can_dlc, HEX); // print DLC
    //Serial.print(" ");
    if(recvMsg2.data[0] != dustcommand){
      Serial.println("NO DUST DATA!");
      return;
    }
    for (int i = 0; i<recvMsg2.can_dlc; i++)  {  // print the data
      Serial.print(recvMsg2.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();

    float dust = ((recvMsg2.data[1] * 256 + recvMsg2.data[2])) / 100.0f;
    Serial.print("미세먼지=");
    Serial.println(dust);
    display.print(dust);
    display.println("ug/m3");
    display.display();
  }
  Serial.println("Messages sent");
  
  
  delay(5000);
}