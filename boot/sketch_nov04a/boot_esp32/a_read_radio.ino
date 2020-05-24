void read_radio()
{
  while (Serial2.available() > 0)
  {
    char first_byte=Serial2.read();

    
    

    //Get manual commands
    if (first_byte == 'O')
    {
      char commands_temp[5];
      commands_temp[0] = Serial2.read();
      commands_temp[1] = Serial2.read();
      commands_temp[2] = Serial2.read();
      commands_temp[3] = Serial2.read();
      commands_temp[4] = Serial2.read();

      if (Serial2.read() == 'X')
      {
        strncpy(commands, commands_temp, 5);
        autopilot_mode = 'M';
        last_received = millis();

      }


    }

    //flight recorder
    if (first_byte == 'R')
    {
      char second_byte=Serial2.read();
      
        

      if (second_byte == 'W')
      {
        betrieb = "schreiben";

        last_received = millis();

      }

      if (second_byte == 'R')
      {
        betrieb = "lesen";
        last_received = millis();

      }
      if (second_byte == 'N')
      {
        betrieb = "nichts";
        last_received = millis();

      }


    }

   //New configuration
    if (first_byte == 'C')
    {
       char second_byte=Serial2.read();
       //Configure Kp of PnID
    if (second_byte == 'P')
      {
          char PID_KP_char[4];
          char sign=Serial2.read();          
          PID_KP_char[0]=Serial2.read();
          PID_KP_char[1]=Serial2.read();
          PID_KP_char[2]=Serial2.read();
          PID_KP_char[3]=Serial2.read();

          
          if (isDigit(PID_KP_char[0])&&isDigit(PID_KP_char[1])&&isDigit(PID_KP_char[2])&&isDigit(PID_KP_char[3]))
          {
          if (sign=='-')
          {
          kp=-atoi(PID_KP_char)/10;
          }
          if (sign=='+')
          {
          kp=atoi(PID_KP_char)/10;
          }
            

          }

          Serial2.print("Kp set:");
          Serial2.println(kp);
          

        
      }

      
    }


   //Set new home command
    if (first_byte == 'H')
    {
      char second_byte=Serial2.read();


      if (second_byte == 'S')
      {
        home_set=false;
        
      }
    //Return home
     if (second_byte == 'H')
      {
        autopilot_mode = 'C';
        heading_sp = gps.courseTo((double)lati, (double)lon, lat_home, (double)lng_home);
        Serial2.print("SReturnHomeX");
        last_received = millis();
        
      }


    //autopilot
     if (second_byte == 'A')
      {
        autopilot_mode = 'A';
        heading_sp = gps.courseTo((double)lati, (double)lon, lat_home, (double)lng_home);
        Serial2.print("SReturnHomeX");
        last_received = millis();
        
      }
      
      //Set autopilot to follow course mode
      if (second_byte == 'C')
      {  
          char heading_sp_char[3];
          heading_sp_char[0]=Serial2.read();
          heading_sp_char[1]=Serial2.read();
          heading_sp_char[2]=Serial2.read();
          if (isDigit(heading_sp_char[0])&&isDigit(heading_sp_char[1])&&isDigit(heading_sp_char[2]))
          {
          heading_sp=atoi(heading_sp_char);

          autopilot_mode = 'C';

          Serial2.print("Heading_sp: ");          
          Serial2.print(heading_sp);          
          }
            
      }



    }
    




  }

}
