                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     #define MOSFET_PIN 25
#define SWITCH_PIN 36
#define SDA 21
#define SCL 22
#define RX1 13
#define TX1 10
#define RX2 14
#define TX2 12
#define send_every 200
#define M_1 18
#define M_2 5
#define R_1 17
#define R_2 16






#include <U8g2lib.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <MPU9250_asukiaaa.h>

//Display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

TinyGPSPlus gps;
double lati, lon,velocity,heading,no_sat,lat_home,lng_home;

MPU9250 mySensor;
float tilt,tilt_gyro,tilt_accel;
float roll,roll_gyro,roll_accel;
float total_accel;
float T_accel=0.3;
bool fix;

//Autopilot
float wp_lng[10]={7.5411,0,0,0,0,0,0,0,0,0};
float wp_lat[10]={51.5316,0,0,0,0,0,0,0,0,0};
int wp_act=0;
float heading_sp=0;
float distance=0;
char autopilot_mode='M';
unsigned long last_received; 
int home_after_not_received=1000; //nach 1000 ms Home modus wenn kein empfang
float kp=0;
float ki=0;
float td=0;
float int_error;
float last_error;
float cv_rudder;

//switch
float switch_state_float;
bool switch_state;
float T_switch=1; //nach s an

//cycle time
unsigned long previos_micros;
int n_loop;
int loop_average=10;
float loop_time;

//telemetry
unsigned long last_send; 


//Radio
char commands[]={ 'N', 'N', 'N', 'N', 'N'};

#line 73 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void setup(void);
#line 108 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void loop(void);
#line 122 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void check_emergency();
#line 134 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void loop_time_measurement();
#line 155 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void read_switch();
#line 174 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void read_radio();
#line 221 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void send_tele();
#line 241 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void write_display();
#line 269 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void get_orienation();
#line 312 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void get_GPS();
#line 356 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void set_home();
#line 376 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void autopilot();
#line 73 "C:\\Users\\Martin\\Desktop\\arduino_esp\\Boot\\boot\\sketch_nov04a\\sketch_nov04a.ino"
void setup(void) {
  Wire.begin(SDA,SCL);  
  
  //Dpsiplay
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  
  Serial.begin(115200);
  Serial.println("Start Boot");
  //GPS
  Serial1.begin(115200,SERIAL_8N1, RX1, TX1); 
  
  //MOSFET
  pinMode(  MOSFET_PIN,OUTPUT);
  
  //SWITCH
  pinMode(  SWITCH_PIN,INPUT);
  
  
 //Radio Zigbee
  Serial2.begin(9600,SERIAL_8N1, RX2, TX2); 
  
  //MPU9250
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
    mySensor.beginMag();

  //cycle time
  previos_micros=millis();
  Serial.print("Loop time");
  Serial.println(loop_time);
  
}

void loop(void) {
  read_radio();
  read_switch();
  get_GPS();
  //write_display();
  send_tele();
  set_home();
  check_emergency();
  loop_time_measurement();
  
           // transfer internal memory to the display
  delay(10);  
}

void check_emergency() //Falls kein Funksignal
{
  if ((home_after_not_received-last_received)>home_after_not_received)
  {
    autopilot_mode='H';
    
    
    }
  
  
  }

void loop_time_measurement()
{

   n_loop++;
  
  if (n_loop==loop_average)
  {
    n_loop=0;
    loop_time=(millis()-previos_micros)/loop_average/1000.0;
    //Serial.println(loop_time);
    previos_micros=millis();

    
    }
  


}



void read_switch()
{
  int in=digitalRead(SWITCH_PIN);
  

  
  switch_state_float=switch_state_float+(in-switch_state_float)/T_switch*loop_time;
  switch_state=false;
  if (switch_state_float>0.9)
  {
    switch_state=true;
    
    
    }

  
 }


void read_radio()
{
  while (Serial2.available()>0)
  {
    //Get manual commands
    if (Serial2.read()=='O')
    {
      char commands_temp[5];
      commands_temp[0]=Serial2.read();
      commands_temp[1]=Serial2.read();
      commands_temp[2]=Serial2.read();
      commands_temp[3]=Serial2.read();
      commands_temp[4]=Serial2.read();

      if (Serial2.read()=='X')
      {
        strncpy(commands_temp,commands,5);
        autopilot_mode='M';
        last_received=millis();
        
        
      }
      
          
    }
    //Get home command
    if (Serial2.read()=='H')
    {


      if (Serial2.read()=='X')
      {
        autopilot_mode='H';
        Serial2.print("SReturnHomeX");
        last_received=millis();       
        
      }
      
          
    }
    
  
    
  }
  
}

void send_tele()
{
  
  if ((millis()-last_send)>send_every)
  { 
    int lati_digits=(lati-int(lati))*100000;
    int longi_digits=(lon-int(lon))*100000;
    int heading_digits=round(heading);
    
    
    String temp_radio="T"+String(lati_digits)+"L"+String(longi_digits)+"S"+String(velocity,2)+"H"+String(heading_digits)+"E";
    
    
    Serial2.println(temp_radio);
    Serial.println(temp_radio);
    
    last_send=millis();
    }
  }

void write_display()
{
  u8g2.clearBuffer();         // clear the internal memory
  String temp_orient="Roll:"+String(roll)+" Tilt:" +String(tilt);
  String temp_gps_0="No FIX "+String(no_sat,0);
  String temp_gps_1;
  String temp_gps_2;
  
  if (gps.location.isValid())
  {

    
    temp_gps_0="Lat:"+String(lati,4)+" Lon:" +String(lon,4);
    temp_gps_1="Speed:"+String(velocity,2)+" Heading"+String(heading,0);
    temp_gps_2= "No Sat:"+String(no_sat,0);
  
    }

  u8g2.drawStr(0,10, temp_orient.c_str());  // write something to the internal memory
  u8g2.drawStr(0,25, temp_gps_0.c_str());  // write something to the internal memory
u8g2.drawStr(0,40, temp_gps_1.c_str());  // write something to the internal memory
u8g2.drawStr(0,55, temp_gps_2.c_str());  // write something to the internal memory
  
  u8g2.sendBuffer(); 

  }


void get_orienation()
{
  mySensor.accelUpdate();
  float accelX = mySensor.accelX();
  float accelY = mySensor.accelY();
  float accelZ = mySensor.accelZ();

   mySensor.gyroUpdate();
   float gX = mySensor.gyroX()+1.5;
   float gY = mySensor.gyroY()+0.5;
   float gZ = mySensor.gyroZ();




float tilt_accel_now=accelZ>0?-180-atan2 (accelY ,( sqrt ((accelX * accelX) + (accelZ * accelZ))))/1.5*90:atan2 (accelY ,( sqrt ((accelX * accelX) + (accelZ * accelZ))))/1.5*90;
tilt_accel_now=tilt_accel_now<-180?360+tilt_accel_now:tilt_accel_now;
float roll_accel_now=accelZ>0?180-atan2(-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))))/1.5*90:atan2(-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))))/1.5*90;
roll_accel_now=roll_accel_now>180?roll_accel_now-360:roll_accel_now;
float accel_now=  sqrt(accelX*accelX+accelY*accelY+accelZ*accelZ);

total_accel=total_accel+(accel_now-total_accel)/T_accel*loop_time;  


tilt_accel=(1-T_accel)*tilt_accel_now+T_accel*tilt_accel;
roll_accel=(1-T_accel)*roll_accel_now+T_accel*roll_accel;
/*
Serial.print(accelX );
Serial.print(";");
Serial.print(accelY );
Serial.print(";");
Serial.print(accelZ);
Serial.print(";");

Serial.println(roll_accel);
*/
tilt=0.98*(tilt+gX*loop_time)+0.02*tilt_accel;
roll=0.98*(roll+gY*loop_time)+0.02*roll_accel;


}


void get_GPS()
{

while (Serial1.available() > 0)
{
  if (gps.encode(Serial1.read()))
    {
      
      

  if (gps.location.isValid())
  {

    
    //Serial.println("FIX");
   
    lati=gps.location.lat();
    lon=gps.location.lng();
    heading=gps.course.deg();
    velocity=gps.speed.mps();
    no_sat=gps.satellites.value();
    fix=true;
    
    
    }
  else
  {
    //Serial.println("No FIX");
    //Serial.println("Char from GPS: "+String(gps.charsProcessed()));
    no_sat=gps.satellites.value();
    fix=false;
    //Serial.println(String(no_sat,0));
    lati=0;
    lon=0;
    heading=0;
    velocity=0;
    
    }
    }

    
}
}

void set_home()
{
if (switch_state&&gps.location.isValid())
{
  lat_home=lati;
  lng_home=lon; 
  int lati_digits=(lati-int(lati))*100000;
  int longi_digits=(lon-int(lon))*100000;
    
    
    String temp_radio="H"+String(lati_digits)+"L"+String(longi_digits)+"E";
      Serial.println(temp_radio);
      Serial2.println(temp_radio);
      
  switch_state_float=0;
}

  
}

void autopilot()
{

//
//float wp_actual_lng=wp_lng[wp_act];
//float wp_actual_lat=wp_lat[wp_act];
//heading_sp=gps.courseTo((double)lati,(double)lon,wp_actual_lat,(double)wp_actual_lng);
//distance=gps.distanceBetween((double)lati,(double)lon,wp_actual_lat,(double)wp_actual_lng);

if (autopilot_mode=='M')
{ 
  heading_sp=gps.courseTo((double)lati,(double)lon,lat_home,(double)lng_home);
  distance=gps.distanceBetween((double)lati,(double)lon,lat_home,(double)lng_home);
  float error=heading_sp-heading;
  if (error>180)
  {
        error=error-360;
    }
    
  else if(error<-180)
  {
        error=360+error;
  }
  float derror_dt=(error-last_error)/loop_time;
  int_error=int_error+error*loop_time;
  
  last_error=error;
  
  cv_rudder=kp*error+ki*int_error+td*derror_dt;
  Serial.print("PV:");
  Serial.print(heading);
  Serial.print(" SP:");
  Serial.print(heading_sp);
  Serial.print(" Error:");
  Serial.print(error);
  Serial.print(" CV:");
  Serial.println(cv_rudder);
  
  
  
}



}
