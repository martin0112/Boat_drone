#include <Wire.h>
#include <SC16IS750.h>

SC16IS750 i2cuart =SC16IS750(77);


void setup() 
{
    Serial.begin(9600);
    Serial.println("Hallo");
    Wire.begin();

};

void loop() 
{
i2cuart.write('a');
i2cuart.write('x');
i2cuart.write('z');

if (i2cuart.available()>0)
{
Serial.println("Empfangen:");

while (i2cuart.available()>0)
{
  
int a=i2cuart.read();

Serial.print(a);
}
Serial.println("Fertig empfangen");

}
  
};
