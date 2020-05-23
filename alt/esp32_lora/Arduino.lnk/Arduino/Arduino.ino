#include <Wire.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <MPU9250_asukiaaa.h>

//Display
SSD1306 display(0x3c, 4, 15);

//MPU9250
MPU9250 mySensor;
double tilt;
double roll;

HardwareSerial Serial2(1);

#define v_motor_max 200
#define u_rudder_max 200

#define MA1 13  //Motor
#define MA2 21  //Motor
#define MB1 12 //Ruder
#define MB2 33 //Ruder
#define sw_camera 17 //Mosfet switch
#define oled_pin 16

//PWM infos
int freq = 1000;
int resolution = 8;
int dutyCycle = 0;

//Power Measurement
Adafruit_INA219 ina219;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

//Communikation
const byte numChars = 5; //Anfangs X Vor/Neutral/Zurück;Links/Neutral/Rechts
char order[numChars]={'N','N','N','N','N'}; // an array to store the received data
int cycles_no_command; //Wie oft keine order oder handshake empfangen
const int cycles_no_command_limit=1000000;
int counter;
int report_time=0;

//Autopilot
TinyGPSPlus gps;
double lati, lon;
double velocity,heading,heading_sp,akkum_error,old_error;
bool autopilot=false;
float Kp=1;
float Ki;
float Kd;
int dt;

//Sensorinfos Senden
const float sending_interval=1; 
int last_send;

//Allgemein
unsigned long last_cycle;

//Lora
#define SS 18
#define RST 14
#define DI0 26
#define BAND 434500000.00
#define spreadingFactor 7
 #define SignalBandwidth 62.5E3
//#define SignalBandwidth 31.25E3

#define preambleLength 4
#define codingRateDenominator 8




void setup() 
{
//Display
 pinMode(16,OUTPUT);
  digitalWrite(oled_pin, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(oled_pin, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
 pinMode(25,OUTPUT);

//Camera
pinMode(sw_camera,OUTPUT);
digitalWrite(sw_camera,HIGH);


  //Serial
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1, 36, 37); //GPS

//MPU9250
  Wire.begin(4, 15); // SDA, SCL
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();

  
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Setup");
  display.display();
  delay(2000);

//Lora
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);

    if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(SignalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setPreambleLength(preambleLength);
  display.clear();
  
  display.drawString(0, 0, "LoRa started");
  display.display();
  delay(1000);
  display.clear();
  display.display();
  
  //Power measurement
    ina219.begin();

//Motor
ledcSetup(0, freq, resolution); //Ruder
ledcSetup(1, freq, resolution); //Ruder
ledcSetup(2, freq, resolution); //Motor
ledcSetup(3, freq, resolution); //Motor
ledcAttachPin(MB1,0);
ledcAttachPin(MB2,1);
ledcAttachPin(MA1,2);
ledcAttachPin(MA2,3);


 
};

void loop() 
{
command_receive();
get_current();
get_orienation();
send_sensor_data();
if (autopilot==false)
{
motor_control();
rudder_control();
} 



};

void send_sensor_data()
{
unsigned long cycle_time=(micros()-last_cycle )/1000;
last_cycle=micros();
display.clear();
display.drawString(0,10,"Order:"+ (String)order[0]+(String)order[1]);
display.drawString(50,10,"U="+ (String)loadvoltage+"V");
display.drawString(0,20,"A:"+ (String)autopilot+"H:"+(String)(int)heading_sp);
display.drawString(0,30,"Tilt:"+ (String)(int)tilt+"Roll:"+(String)(int)roll);
display.drawString(0,40,"cycle time:"+ (String)cycle_time);


float time_since_sending=(millis()-last_send)/1000.0; //in s

  if (time_since_sending>sending_interval)  
  {

  String transfer_string='T'+String(tilt,0)+'L'+String(roll,0)+'V'+velocity+'H'+heading+'U'+String(loadvoltage,2)+'C'+String(current_mA)+'A'+String(autopilot)+String(heading_sp,2)+'I'+String(lati)+'D'+String(lon);
  
  LoRa.beginPacket();
  LoRa.println(transfer_string);
  LoRa.endPacket(true);
  display.drawString(0,50,"Sending");
  last_send=millis();
  }

display.display();

      
  
}

void get_orienation()
{
  mySensor.accelUpdate();
  double accelX = mySensor.accelX();
  double accelY = mySensor.accelY();
  double accelZ = mySensor.accelZ();
  
tilt =tilt-0.25*tilt+0.25*atan2 (accelY ,( sqrt ((accelX * accelX) + (accelZ * accelZ))))/1.5*90;
roll =roll-0.25*roll+0.25* atan2(-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))))/1.5*90;
  
}

void get_current()
{
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  float   busvoltage = ina219.getBusVoltage_V();
  float shuntvoltage = ina219.getShuntVoltage_mV();

  loadvoltage = busvoltage + (shuntvoltage / 1000);
   
  }




void command_receive()
{
cycles_no_command++;


                    
                    

int packetSize = LoRa.parsePacket();
if (packetSize==1) {
char received=LoRa.read();

      

switch (received) {
  case 'N':
order[0]='N';
order[1]='N';
cycles_no_command=0;
autopilot=false;

    
  break;

      
      case 'P':
order[0]='N';
order[1]='R';
cycles_no_command=0;
autopilot=false;

    
    break;

      case 'O':
order[0]='N';
order[1]='L';
cycles_no_command=0;
autopilot=false;
    break;



      case 'S':
order[0]='S';
order[1]='N';
cycles_no_command=0;
autopilot=false;
    break;

      case 'V':
order[0]='V';
order[1]='N';
cycles_no_command=0;
autopilot=false;
    break;



      case 'L':
order[0]='L';
order[1]='N';
cycles_no_command=0;
autopilot=false;
    break;

      case 'A':
order[0]='S';
order[1]='L';
cycles_no_command=0;
autopilot=false;
    break;

      case 'B':
order[0]='V';
order[1]='L';
cycles_no_command=0;
autopilot=false;
    break;

      case 'C':
order[0]='L';
order[1]='L';
cycles_no_command=0;
autopilot=false;
    break;


      case 'D':
order[0]='S';
order[1]='R';
cycles_no_command=0;
autopilot=false;
    break;

      case 'E':
order[0]='V';
order[1]='R';
cycles_no_command=0;
autopilot=false;
    break;

      case 'F':
order[0]='L';
order[1]='R';
cycles_no_command=0;
autopilot=false;
    break;


      case 'G':
order[0]='Z';
order[1]='L';
cycles_no_command=0;
autopilot=false;
    break;

      case 'H':
order[0]='Z';
order[1]='R';
cycles_no_command=0;
autopilot=false;
    break;

      case 'K':
order[0]='Z';
order[1]='N';
cycles_no_command=0;
autopilot=false;
    break;



  
}



      


}

if (packetSize>1)
{
  char received=LoRa.read();
  if (received=='H')
  {
   char temp1=(char)LoRa.read();

   char temp2=(char)LoRa.read();

   char temp3=(char)LoRa.read();


   String heading_sp_temp=(String)temp1+(String)temp2+(String)temp3;
   

   
    heading_sp=heading_sp_temp.toDouble();
    
    autopilot=true;
    

  }
  
}

  


//Wenn keine oder handshake für x zyklen dann notfallprotokoll
if (cycles_no_command>cycles_no_command_limit)
{

order[0]='V';
order[1]='N';
order[2]='N';
order[3]='N';
order[4]='N';

  
  }

}


void rudder_control()
{
  
   if (order[1]=='R')
  {
ledcWrite(0, u_rudder_max);
ledcWrite(1, 0);


  }


   if (order[1]=='L')
  {
ledcWrite(1, u_rudder_max);
ledcWrite(0, 0);
  }

  if (order[1]=='N')
  {
ledcWrite(1, 0);
ledcWrite(0, 0);
  }
  
  
  
}






void motor_control()
{

int speed_percent=0;
int vorwarts=0;
int rueckwaerts=0;

int schnell=0;
int langsam=0;


   if (order[0]=='V')
  {
   vorwarts=1;   
   schnell=0;
   langsam=0;
  }

   if (order[0]=='S')
  {
   vorwarts=1;   
   schnell=1;
   langsam=0;
  
  
  
  }

    if (order[0]=='L')
  {
   vorwarts=1;   
   schnell=0;
   langsam=1;
  
  }

  

    if (order[0]=='Z')
  {
   rueckwaerts=1;
   schnell=0;
   langsam=0;
  
  }

      if (order[0]=='R')
  {
   rueckwaerts=1;
   schnell=1;
   langsam=0;
  
  }


 
 speed_percent=vorwarts*(50+schnell*50-langsam*25)-rueckwaerts*(50+schnell*50-langsam*25);
 


int geschwindigkeit=(int)(speed_percent/100.0*v_motor_max);


 if (speed_percent>0)
 {
  ledcWrite(2, geschwindigkeit);
ledcWrite(3, 0);

 }
if (speed_percent<0)
 {
    ledcWrite(3, geschwindigkeit);
ledcWrite(2, 0);
 }
 if (speed_percent==0)
 {
  ledcWrite(2, 0);
ledcWrite(3, 0);
  }
    
  
}




/*
if (received[0]='P')
{
double K_p_temp=0;
double K_i_temp=0;
double K_d_temp=0;
   K_p_temp=received[1];
   K_i_temp=received[2];
   K_d_temp=received[3];
  
   Kp=K_p_temp; 
   Ki=K_i_temp;
   Kd=K_d_temp;     
   Serial.println("Param Update"+String(Kp)+";"+String(Ki)+";"+String(Kd)); 
   
  
}

else if (received[0]='H')
{
  String heading_sp_temp;
   heading_sp_temp=(int)received[1];
   heading_sp_temp=heading_sp+(int)received[2];
   heading_sp_temp=heading_sp+(int)received[2];
  

   
    heading_sp=heading_sp_temp.toDouble();
    autopilot=true;
    
    
     
  }
else{
*/
