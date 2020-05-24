void get_orientation()
{
  mySensor.accelUpdate();
  float accelX = mySensor.accelX();
  float accelY = mySensor.accelY();
  float accelZ = mySensor.accelZ();

  mySensor.gyroUpdate();
  float gX = mySensor.gyroX() + 1.5;
  float gY = mySensor.gyroY() + 0.5;
  float gZ = mySensor.gyroZ();




  float tilt_accel_now = accelZ > 0 ? -180 - atan2 (accelY , ( sqrt ((accelX * accelX) + (accelZ * accelZ)))) / 1.5 * 90 : atan2 (accelY , ( sqrt ((accelX * accelX) + (accelZ * accelZ)))) / 1.5 * 90;
  tilt_accel_now = tilt_accel_now < -180 ? 360 + tilt_accel_now : tilt_accel_now;
  float roll_accel_now = accelZ > 0 ? 180 - atan2(-accelX , ( sqrt((accelY * accelY) + (accelZ * accelZ)))) / 1.5 * 90 : atan2(-accelX , ( sqrt((accelY * accelY) + (accelZ * accelZ)))) / 1.5 * 90;
  roll_accel_now = roll_accel_now > 180 ? roll_accel_now - 360 : roll_accel_now;
  float accel_now =  sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  total_accel = total_accel + (accel_now - total_accel) / T_accel * loop_time;
  

  tilt_accel = (1 - T_accel) * tilt_accel_now + T_accel * tilt_accel;
  roll_accel = (1 - T_accel) * roll_accel_now + T_accel * roll_accel;
  /*
    Serial.print(accelX );
    Serial.print(";");
    Serial.print(accelY );
    Serial.print(";");
    Serial.print(accelZ);
    Serial.print(";");

    Serial.println(roll_accel);
  */
  tilt = 0.98 * (tilt + gX * loop_time) + 0.02 * tilt_accel;
  roll = 0.98 * (roll + gY * loop_time) + 0.02 * roll_accel;


}
