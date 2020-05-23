#include <Wire.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_INA219.h>



HardwareSerial Serial2(2);
HardwareSerial Serial1(1);

#define v_motor_max 200
#define u_rudder_max 200

#define MA1 26  //Motor
#define MA2 27  //Motor
#define MB1 32 //Ruder
#define MB2 33 //Ruder
#define sw_radio 19 //Mosfet switch
#define sw_camera 15 //Mosfet switch


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
char order[numChars]; // an array to store the received data
char received[numChars+1]; // an array to store the received data
int cycles_no_command; //Wie oft keine order oder handshake empfangen
const int cycles_no_command_limit=1000000;
int counter;
int report_time=0;

//Autopilot
TinyGPSPlus gps;
double lati, lon;
double velocity,heading,heading_sp,akkum_error,old_error;
bool new_gps=false;
bool autopilot=false;
float Kp=1;
float Ki;
float Kd;
int dt;

void setup() 
{
delay(1000);
Serial.begin(115200);
Serial2.begin(9600,SERIAL_8N1, 16, 9); //GPS
Serial1.begin(9600,SERIAL_8N1, 5, 18); //Radio 

delay(1000);

ledcSetup(0, freq, resolution); //Ruder
ledcSetup(1, freq, resolution); //Ruder
ledcSetup(2, freq, resolution); //Motor
ledcSetup(3, freq, resolution); //Motor
ledcAttachPin(MB1,0);
ledcAttachPin(MB2,1);
ledcAttachPin(MA1,2);
ledcAttachPin(MA2,3);

pinMode(sw_radio,OUTPUT);
pinMode(sw_camera,OUTPUT);

digitalWrite(sw_radio,HIGH);
digitalWrite(sw_camera,HIGH);

ina219.begin();


Serial.println("Start Boot"); //Intern Debug
Serial1.println("Start Boot"); //Intern Debug

//I2c als master

Wire.begin();



};

void loop() 
{

get_gps();
get_current();
sensors_send_arduino();
command_receive();  
if (autopilot==false)
{
motor_control();
rudder_control();
} 
else
{
  autopilot_control();
  }
  
};

void get_current()
{
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  float   busvoltage = ina219.getBusVoltage_V();
  float shuntvoltage = ina219.getShuntVoltage_mV();

  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  
  }

void autopilot_control()
{
dt=micros()-dt;
float error=heading-heading_sp;  
akkum_error=akkum_error+error*dt/1000000;
float derror_dt=(error-old_error)/dt;
float CV=Kp*error+Ki*akkum_error+Kd*derror_dt;
if (CV>0)
{CV=CV>100?100:CV;
int ruder=(int)(CV/100.0*u_rudder_max);
ledcWrite(0, ruder);
ledcWrite(1, 0);

}
else
{CV=abs(CV)>100?100:abs(CV);
int ruder=(int)(CV/100.0*u_rudder_max);
ledcWrite(1, ruder);
ledcWrite(0, 0);

}

dt=micros();
}

void sensors_send_arduino()
{

String transfer_string="";
if (new_gps==true)
{
  transfer_string='R'+String(lati,5)+'L'+String(lon,5)+'V'+velocity+'H'+heading+'V'+String(loadvoltage,2)+'P'+String(power_mW)+'A'+String(autopilot)+String(heading_sp,2)+'E';
  new_gps=false;
  Serial1.println(transfer_string);
  Serial.println(transfer_string);

}
else
{
  }
  
  
  
}

void get_gps()
{
    
  
while (Serial2.available())
  {
char temp=Serial2.read();
//Serial.print(temp);
//Serial1.print(temp);

    if (gps.encode(temp))
   {

      lati=gps.location.lat();
      lon=gps.location.lng();
      velocity=gps.speed.kmph();
      heading=gps.course.deg();
      new_gps=true;

    }

    
  }  
  
  
}

void rudder_control()
{
  
   if (order[1]=='B')
  {
ledcWrite(0, u_rudder_max);
ledcWrite(1, 0);


  }


   if (order[1]=='S')
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


void command_receive()
{
cycles_no_command++;


/*                  Wenn eröffnung X dann manuell
                    0. S- Schnell Vor
                    0. V- Normal Vor
                    0. L- Langsam Vor
                    0. N- Motor Aus
                    0. Z- Normal Zurück
                    0. R- Schnell Zurück
                    1. L- links
                    1. N- Geradeaus
                    1. R- Rechts
                    2. P- Position Anfoderdern
                    3. H- Heimat Position setzen/Zurueck Ausgangspunkt
                    
                    
*/
if (Serial1.available()>numChars+2) 
{
counter++;
char anfang = Serial1.read();


//X ist Allgemein anfang
if (anfang == 'X') {

for (int i=0;i<numChars;i++)
{

received[i]=Serial1.read();
}
char ende=Serial1.read();
if (ende=='E')
{
  /*
Serial.print("Empfangen ");
Serial.print(anfang);
Serial.print(received[0]);
Serial.print(received[1]);
Serial.print(received[2]);
Serial.print(received[3]);
Serial.print(received[4]);
Serial.println(ende);



*/
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
order[0]=received[0];
order[1]=received[1];
order[2]=received[2];
order[3]=received[3];
order[4]=received[4];
cycles_no_command=0;
autopilot=false;





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
}
} 
