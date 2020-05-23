#include <Wire.h>

#define v_motor_max 200
#define u_rudder_max 200

#define MA1 6 //Motor
#define MA2 5 //Motor
#define MB1 9 //Ruder
#define MB2 10 //Ruder
#define sw 4 //Mosfet switch


const byte numChars = 5; //Anfangs X Vor/Neutral/Zurück;Links/Neutral/Rechts
char order[numChars]; // an array to store the received data
char received[numChars+1]; // an array to store the received data
int cycles_no_command; //Wie oft keine order oder handshake empfangen
const int cycles_no_command_limit=1000000;
int counter;


//Autopilot
bool autopilot=false;


void setup() 
{
delay(1000);
Serial1.begin(9600);
delay(1000);

pinMode(MA1,OUTPUT); //Motor 
pinMode(MA2,OUTPUT);  //Motor
pinMode(MB1,OUTPUT);  //Ruder
pinMode(MB2,OUTPUT); //Ruder
pinMode(sw,OUTPUT); //mosfet switch

digitalWrite(MA1,LOW);
digitalWrite(MA2,LOW);
digitalWrite(MB1,LOW);
digitalWrite(MB1,LOW);
digitalWrite(sw,HIGH);

//Debugging
Serial.begin(9600);
Serial.println("Start Boot"); //Intern Debug

//I2c als slave

Wire.begin(8);
Wire.onReceive(I2creceive);

};

void loop() 
{
  
command_receive();  
motor_control();
rudder_control();


  
};

void I2creceive(int how_many)
{

  while (Wire.available())
  {
    char c=Wire.read();
    Serial.print(c);
    Serial1.print(c);
    }
  
  }

void rudder_control()
{
   if (order[1]=='B')
  {
analogWrite(MB1,u_rudder_max);
analogWrite(MB2,0);
  }


   if (order[1]=='S')
  {
analogWrite(MB1,0);
analogWrite(MB2,u_rudder_max);
  }

  if (order[1]=='N')
  {
analogWrite(MB1,0);
analogWrite(MB2,0);
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
 {analogWrite(MA1,geschwindigkeit);
 analogWrite(MA2,0); 
 }
if (speed_percent<0)
 {analogWrite(MA2,geschwindigkeit);
 analogWrite(MA1,0);
 }
 if (speed_percent==0)
 {
  analogWrite(MA2,0);
 analogWrite(MA1,0);
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

//X ist manueller Empfang
if (anfang == 'X') {


for (int i=0;i<numChars;i++)
{
received[i]=Serial1.read();
    
}
char ende=Serial1.read();

if (ende=='E')
{
  
Serial1.print("Empfangen ");
Serial1.print(anfang);
Serial1.print(received[0]);
Serial1.print(received[1]);
Serial1.print(received[2]);
Serial1.print(received[3]);
Serial1.print(received[4]);
Serial1.println(ende);






order[0]=received[0];
order[1]=received[1];
order[2]=received[2];
order[3]=received[3];
order[4]=received[4];
cycles_no_command=0;


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
