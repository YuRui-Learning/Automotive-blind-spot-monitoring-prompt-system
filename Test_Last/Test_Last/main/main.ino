#include <SoftwareSerial.h>
#include <Wire.h>
#include "Gesture.h"

#include <Arduino.h>
#include <U8x8lib.h>

#define GES_ENTRY_TIME      800       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s). 
#define GES_QUIT_TIME     1000
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display

const int buttonPin0 = 2;     // the number of the pushbutton pin
const int buttonPin1 = 3;     // the number of the pushbutton pin
const int buttonPin2 = 4;     // the number of the pushbutton pin
const int beep = 7;     // the number of the pushbutton pin
const int TrigPin1 = 9;
const int EchoPin1 = 8;
const int TrigPin0 = 12;
const int EchoPin0 = 13;
const int LED0 = A2;
const int LED1 = A1;
const int LED2 = A0;



SoftwareSerial softSerial(10, 11); // RX, TX
typedef struct
{
  int data[50][4] = {{}};
  int len = 0;
}List;
List list;


uint8_t flag=0;
uint8_t kai=0;
uint8_t dis=0;

uint8_t dis2=0;//右后方
uint8_t dis0=0;//左后方
float distance0;
float distance1;

uint8_t carlocate=0;
uint8_t motorcyclelocate=0;
uint8_t kind=0;
int buttonState0 = 0;         // variable for reading the pushbutton status
int buttonState1 = 0;         // variable for reading the pushbutton status
int buttonState2 = 0;         // variable for reading the pushbutton status


void setup()
{
  uint8_t error = 0;
  Serial.begin(115200);
  softSerial.begin(115200);
  ///////////////////////////////////IO口初始化///////////////////////////////////////////////////////////
  pinMode(TrigPin0, OUTPUT);
  pinMode(EchoPin0, INPUT);
  pinMode(TrigPin1, OUTPUT);
  pinMode(EchoPin1, INPUT);
  pinMode(beep, OUTPUT);
  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(beep, HIGH);
  /////////////////////////////////////OLED////////////////////////////////
   u8x8.begin();
  u8x8.setPowerSave(0);

}



void loop()
{
  char displaybuf[32];
  memset(displaybuf,0,sizeof(displaybuf));
  uint8_t data = 0, data1 = 0, error;
  //error = GestureReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  u8x8.setFont(u8x8_font_8x13_1x2_r);//u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(3,0,"BSD system");
  u8x8.refreshDisplay();    // only required for SSD1606/7  
  buttonState0 = digitalRead(buttonPin0);
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);
   /////////////////////////////////////////按键输入输出////////////////////////////////////////////////
  key();
  Serial.print("flag:"); 
  Serial.print(flag  );
  Serial.print('\t');
  Serial.print("kai:"); 
  Serial.println(kai);
  digitalWrite(beep, HIGH);
    ///////////////////////////oled///////////////////////////
  sprintf(displaybuf,"Input:%d",kai);
  u8x8.drawString(0,2,displaybuf);
  sprintf(displaybuf,"FAR:%d %d %d ",dis0,dis,dis2);
  u8x8.drawString(0,4,displaybuf);  
//  sprintf(displaybuf,"Lf:%d ",dis0);
//  u8x8.drawString(0,4,displaybuf);
//  sprintf(displaybuf,"Rf:%d ",dis2);
//  u8x8.drawString(8,4,displaybuf);
//  sprintf(displaybuf,"Behind:%d  ",dis);
//  u8x8.drawString(2,5,displaybuf);
  sprintf(displaybuf,"CL:%d",carlocate);
  u8x8.drawString(0,6,displaybuf);
  sprintf(displaybuf,"ML:%d ",motorcyclelocate);
  u8x8.drawString(8,6,displaybuf);


  /////////////////////////////////////HC-SR04//////////////////////////////////////////////////////////////
  
  digitalWrite(TrigPin0, LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin0, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin0, LOW);
  distance1 = pulseIn(EchoPin0, HIGH) / 58.0; //算成厘米
  dis2=int(distance1*10);
  Serial.print("distance1:");
  Serial.print(distance1);
  Serial.print("cm");
  delay(50);
  digitalWrite(TrigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin1, LOW);
  distance0 = pulseIn(EchoPin1, HIGH) / 58.0;   //echo time conversion into a distance
  distance0 = (int(distance0 * 100.0)) / 100.0;       //keep two decimal places
  dis0=int(distance0*10);
  Serial.print("distance0：");
  Serial.print(distance0);
  Serial.print("cm");
  Serial.print('\t');
  Serial.println();
  delay(50);
 


///////////////////////////////////////////////////////蜂鸣器判断工作//////////////////////////////////////////////
  
  if (kai == 0){
    if ((carlocate == 0)or(motorcyclelocate == 0))
     {if (dis0<=100){
        noise();
        digitalWrite(LED0, LOW);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
      }
     }
     else
     {
        digitalWrite(LED0, HIGH);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
      }
    }
  if (kai == 1){
    if ((carlocate == 1)or(motorcyclelocate == 1))
     {if (dis<=100){
        noise();
        digitalWrite(LED0, HIGH);
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, HIGH);
      }
     }
     else
     {
        digitalWrite(LED0, HIGH);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
      }
    }
  if (kai == 2){
    if ((carlocate == 2)or(motorcyclelocate == 2))
     {if (dis2<=100){
        noise();
        digitalWrite(LED0, HIGH);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, LOW);
      }
     }
     else
     {
        digitalWrite(LED0, HIGH);
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);

      }
    }

////////////////////////////串口/////////////////////////////
  if(softSerial.available())
   {
    getList();
    for (int i=0; i<list.len; i++)
    {
      Serial.print(list.data[i][0]);
      if(list.data[i][0]>2)
      {
        dis=list.data[i][0];
        }
      Serial.print('\t');
      Serial.print(list.data[i][1]);
      if(list.data[i][1]<=2)
      {
        carlocate=list.data[i][1];
       }
      Serial.print('\t');
      Serial.print(list.data[i][2]);
      if(list.data[i][2]<=2)
      {
        motorcyclelocate=list.data[i][2];
       }
      Serial.print('\t');
      Serial.println(list.data[i][3]);
      kind=list.data[i][3];
    }
    Serial.println("=======================================");
    clearList();
   }

}

void key()
{
  buttonState0 = digitalRead(buttonPin0);
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);
  Serial.print("buttonState:");
  Serial.print(buttonState0);
  Serial.print(buttonState1);
  Serial.println(buttonState2);
  if ((buttonState1 == LOW)and(buttonState2 == LOW)){
     {kai=2;}
    }
  if ((buttonState0 == LOW)and(buttonState2 == LOW)){
     {kai=1;}
    }
  if ((buttonState1 == LOW)and(buttonState0 == LOW)){
     {kai=0;}
    }
}

void noise()
{
  digitalWrite(beep, HIGH);   // turn the beep on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(beep, LOW);    // turn the beep off by making the voltage LOW
  delay(100); 
}

String detectString()
{
  while(softSerial.read() != '{');
  return(softSerial.readStringUntil('}'));
}
void clearList()
{
  memset(list.data, sizeof(list.data),0);
  list.len = 0;
}
void getList()
{
  int a=0;
  String s = detectString();
  String numStr = "";
  for(int i = 0; i<s.length(); i++)
  {
    if(s[i]=='('){
      numStr = "";
    }
    else if((s[i] == ',')and(a==0)){
      list.data[list.len][0] = numStr.toInt();
      numStr = "";
      a=1;
    }
    else if((s[i]==',')and(a==1)){
      list.data[list.len][1] = numStr.toInt();
      numStr = "";
      a=2;
    }
    else if((s[i]==',')and(a==2)){
      list.data[list.len][2] = numStr.toInt();
      numStr = "";
      a=0;
    }
    else if(s[i]==')'){
      list.data[list.len][3] = numStr.toInt();
      numStr = "";
      list.len++;
    }    
    else{
      numStr += s[i];
    }
  }
}
