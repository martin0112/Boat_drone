#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <Wire.h>

//Display
//Display
SSD1306 display(0x3c, 4, 15);


#define SS 18
#define RST 14
#define DI0 26
// #define BAND 429E6 //915E6

// #define BAND 434500000.00
#define BAND 434500000.00

#define spreadingFactor 7
 #define SignalBandwidth 62.5E3
//#define SignalBandwidth 31.25E3
//#define SignalBandwidth 15.25E3

#define preambleLength 4
#define codingRateDenominator 8

bool Lora_working;
int counter;

void setup() {
//Display
 pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
 pinMode(25,OUTPUT);


Serial.begin(921600);                       // Open serial port to computer
display.clear();
display.setFont(ArialMT_Plain_16);
display.drawString(0, 0, "Serial Start");
display.display();
delay(1000);



lora_start();
display.clear();
if (Lora_working)
display.drawString(0, 0, "LoRa Start");
else
display.drawString(0, 0, "LoRa Failed");
display.display();

delay(1000);
display.clear();
display.display();

}

void loop() {

/*  
  counter++;

    String message=(String)counter;
  Serial.println(message);

  LoRa.beginPacket();
  LoRa.print((char)counter);
  
  
  LoRa.endPacket();
  counter=counter>254?0:counter;
delay(20);
  
  
  unsigned long zeit=micros();
  String message="VBN";

  LoRa.beginPacket();

  LoRa.println(message);

  LoRa.endPacket();
  
  float loop_time=(micros()-zeit)/1000.0;
  

  Serial.println(loop_time+message);
  zeit=micros();
  message="ZSN";

  LoRa.beginPacket();

  LoRa.println(message);

  LoRa.endPacket();
  
  loop_time=(micros()-zeit)/1000.0;
  

  Serial.println(loop_time+message);
 
  
  */
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
  String message;
  while(LoRa.available()){                     // If Arduino's HC12 rx buffer has data
  
  message+=(String)(char)LoRa.read();
                // Send the data to the computer
  }
  display.clear();
  display.drawString(0, 20, message);
  display.display();
  
  Serial.print(message);    
    
 }
    
  if(Serial.available()>2){                   // If Arduino's computer rx buffer has data
   
  
  char temp=Serial.read();
  
  if (temp=='X')
  {
  String order;
  
  LoRa.beginPacket();
      
  while (Serial.available())
  {
  temp=Serial.read();  
 
    if (temp=='J')
      {
          LoRa.endPacket(true);
    
          display.clear();
          display.drawString(0, 0, order);
          display.display();
          
      }
      else
      { LoRa.write(temp);
       order+=(String)temp;
      }

  }
  }
  
  
  
   }
  
  }
  
  
   

   


void lora_start()
{
SPI.begin(5,19,27,18);
LoRa.setPins(SS,RST,DI0);

if(LoRa.begin(BAND))
Lora_working=true;

  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(SignalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);

  LoRa.setPreambleLength(preambleLength);


}
