#include <Wire.h>
#include <TinyGPS.h>

#define ARDUINO_adress 8

//
TinyGPS gps;
long lat, lon;
float velocity,heading;
bool new_gps=false;

//Autopilot

void setup() {
  // put your setup code here, to run once:
Wire.begin(D2,D1);
Serial.begin(9600);
Serial.println("Start GPS");
}

void loop() {
  // put your main code here, to run repeatedly:
get_gps();
position_send_arduino();

delay(10);

}


void position_send_arduino()
{
String transfer_string="";
if (new_gps)
{
  transfer_string='R'+(String)lat+'L'+(String)lon+'V'+velocity+'H'+heading+'E';
  new_gps=false;

}
else
{
  transfer_string="NOGPS";
}
  
  Wire.beginTransmission(ARDUINO_adress);
  for (int i=0;i<transfer_string.length();i++)
{  Wire.write(transfer_string.charAt(i));
}
  Wire.endTransmission();
  
  Serial.println(transfer_string);

  
  
}
 
  
  
  

void get_gps()
{

  
while (Serial.available())
  {
    
    int c = Serial.read();
//Serial.print(c);

    if (gps.encode(c))
    {
      gps.get_position(&lat,&lon);
      velocity=gps.f_speed_mps();
      heading=gps.f_course();
      new_gps=true;
    }
    
  }  
  
  
  
  }
