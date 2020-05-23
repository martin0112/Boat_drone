//433Mhz HC12 an Computer "Dongle"
#include <SoftwareSerial.h>


SoftwareSerial HC12(D6, D7, false, 128); 

void setup() {

delay(1000);
Serial.begin(115200);                       // Open serial port to computer
HC12.begin(9600);                         // Open serial port to HC12

    
}

void loop() {


  if(HC12.available()){                     // If Arduino's HC12 rx buffer has data
    Serial.write(HC12.read());              // Send the data to the computer
    
    
    }
  if(Serial.available()){                   // If Arduino's computer rx buffer has data
    
    HC12.write(Serial.read());   
    
    }
    
    // Send that data to serial

}
