#include <MPU9250_asukiaaa.h>

#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 4
#define SCL_PIN 15
#endif

MPU9250 mySensor;

uint8_t sensorId;
float accelX, accelY, accelZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ;

void setup() {
  while(!Serial);
  Serial.begin(115200);
  Serial.println("started");

#ifdef _ESP32_HAL_I2C_H_ // For ESP32
  Wire.begin(SDA_PIN, SCL_PIN); // SDA, SCL
#else
  Wire.begin();
#endif

  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();

  // You can set your own offset for mag values
  // mySensor.magXOffset = -50;
  // mySensor.magYOffset = -55;
  // mySensor.magZOffset = -10;

  sensorId = mySensor.readId();
}

void loop() {

  mySensor.accelUpdate();
  accelX = mySensor.accelX();
  accelY = mySensor.accelY();
  accelZ = mySensor.accelZ();
  aSqrt = mySensor.accelSqrt();
  
  mySensor.gyroUpdate();
  gX = mySensor.gyroX();
  gY = mySensor.gyroY();
  gZ = mySensor.gyroZ();
  
  mySensor.magUpdate();
  mX = mySensor.magX();
  mY = mySensor.magY();
  mZ = mySensor.magZ();
  mDirection = mySensor.magHorizDirection();
  
  float pitch = atan2 (accelY ,( sqrt ((accelX * accelX) + (accelZ * accelZ))))/1.5*90;
  float roll = atan2(-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))))/1.5*90;
  Serial.println("P:"+(String)pitch+"R:"+(String)roll);

   
  delay(10);
}
