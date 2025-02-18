                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     #define MOSFET_PIN 25
#define SDA 21
#define SCL 22
#define RX1 13
#define TX1 10
#define RX2 14
#define TX2 12
#define send_every 200


#define R_0 33
#define R_1 32 
#define M_0 19 
#define M_1 26







#include <U8g2lib.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <MPU9250_asukiaaa.h>
#include <I2CEEPROM.h>
//Display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

TinyGPSPlus gps;
double lati, lon, velocity, heading, heading_filter,no_sat, lat_home, lng_home;

MPU9250 mySensor;
float tilt, tilt_gyro, tilt_accel;
float roll, roll_gyro, roll_accel;
float total_accel;
float T_accel = 0.3;
bool fix;

//Motordad
#define v_motor_max 200
#define u_rudder_max 200

//Flugschreiber
#define CHIP_ADDRESS 0x50 // Address of EEPROM chip (24LC256->0x50)
#define EEPROM_BYTES 32768 // Number of bytes in EEPROM chip
#define t_cycle_recorder 0.1
#define amount_data 9
int last_record = 0;
int16_t no_records = 0;
String betrieb = "nichts";
I2CEEPROM i2c_eeprom(CHIP_ADDRESS); // Create I2C EEPROM instance
unsigned int current_address = 0;

//Autopilot
float wp_lng[10] = {7.5411, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float wp_lat[10] = {51.5316, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int wp_act = 0;
float heading_sp = 0;
float distance = 0;
char autopilot_mode = 'M';
unsigned long last_received;
int home_after_not_received = 30000; //nach 1000 ms Home modus wenn kein empfang
float kp = 1;
float ki = 0;
float td = 0;
float int_error;
float last_error;
int cv_rudder;
int error;
bool home_set = false;

//switch
float switch_state_float;
bool switch_state;
float T_switch = 1; //nach s an

//cycle time
unsigned long previos_micros;
int n_loop;
int loop_average = 10;
float loop_time;

//telemetry
unsigned long last_send;

//MOtor Rudder
int freq = 1000;
int resolution = 8;
int dutyCycle = 0;

//Radio
char commands[] = { 'N', 'N', 'N', 'N', 'N'};

void setup() {
  Wire.begin(SDA, SCL);

  //Motor

  ledcSetup(0, freq, resolution); //Ruder
  ledcSetup(1, freq, resolution); //Ruder
  ledcSetup(2, freq, resolution); //Motor
  ledcSetup(3, freq, resolution); //Motor
  ledcAttachPin(R_0, 0);
  ledcAttachPin(R_1, 1);
  ledcAttachPin(M_0, 2);
  ledcAttachPin(M_1, 3);

  //Dpsiplay
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

  Serial.begin(115200);
  Serial2.println("Start Boot");
  //GPS
  Serial1.begin(115200, SERIAL_8N1, RX1, TX1);

  //MOSFET
  pinMode(  MOSFET_PIN, OUTPUT);



  //Radio Zigbee
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);

  //MPU9250
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();

  //cycle time
  previos_micros = millis();



  //Rudder test
  ledcWrite(0, u_rudder_max);
  ledcWrite(1, 0);
  delay(500);
  ledcWrite(1, u_rudder_max);
  ledcWrite(0, 0);
  delay(500);
  ledcWrite(0, 0);
  ledcWrite(1, 0);



  
}

void loop() {
  read_radio();
  get_GPS();
  get_orientation();
  send_tele();
  set_home();
  autopilot();
  //check_emergency();
  flugschreiber();
  loop_time_measurement();

  // transfer internal memory to the display
  delay(10);

}
