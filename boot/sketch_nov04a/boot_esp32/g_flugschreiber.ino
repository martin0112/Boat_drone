
void flugschreiber()
{



  if (betrieb == "lesen")
  {

  Serial2.println("Reading data");


    byte big_byte = i2c_eeprom.read(0);
    byte small_byte = i2c_eeprom.read(1);
    int16_t number_records = int16_t(big_byte << 8) + int16_t(small_byte);
  Serial2.print("No Records");
  Serial2.println(number_records);

  


    for (int i = 0; i < number_records; i++)
    {
      for (int k = 0; k < amount_data; k++)
      {
        byte big_byte = i2c_eeprom.read(amount_data * i * 2 + k * 2 + 2);
        byte small_byte = i2c_eeprom.read(amount_data * i * 2 + k * 2 + 1 + 2);
        int16_t recorded_data = int16_t(big_byte << 8) + int16_t(small_byte);
        Serial2.print(recorded_data);
        Serial2.print(";");



      }
      Serial2.println();
      

    }

    betrieb="nichts";
    Serial2.println("Export finished");




  }





  if (betrieb == "schreiben")
  {

    int time_since_record = millis() - last_record;
    if (time_since_record > t_cycle_recorder * 1000)
    {
      

      byte payload[amount_data * 2];


      payload[0] = highByte(int16_t (time_since_record));
      payload[1] = lowByte(int16_t (time_since_record));

      payload[2] = highByte(int16_t (heading_sp));
      payload[3] = lowByte(int16_t (heading_sp));

      payload[4] = highByte(int16_t (heading_filter));
      payload[5] = lowByte(int16_t (heading_filter));

      payload[6] = highByte(int16_t (velocity));
      payload[7] = lowByte(int16_t (velocity));

      payload[8] = highByte(int16_t (roll));
      payload[9] = lowByte(int16_t (roll));


      payload[10] = highByte(int16_t (tilt));
      payload[11] = lowByte(int16_t (tilt));

      payload[12] = highByte(int16_t (cv_rudder));
      payload[13] = lowByte(int16_t (cv_rudder));

      payload[14] = highByte(int16_t (error));
      payload[15] = lowByte(int16_t (error));


      payload[16] = highByte(int16_t (int_error));
      payload[17] = lowByte(int16_t (int_error));


      if ((no_records * amount_data * 2 + 2) > EEPROM_BYTES)
      {
        no_records = 0;
        current_address = 0;

      }
      

      for (int i = 0; i < amount_data * 2; i++)
      {


        i2c_eeprom.write(current_address + 2, payload[i]);
        current_address++;


      }
      no_records++;
      i2c_eeprom.write(0, highByte(no_records));
      i2c_eeprom.write(1, lowByte(no_records));
     
      




      last_record = millis();
    }

  }


}