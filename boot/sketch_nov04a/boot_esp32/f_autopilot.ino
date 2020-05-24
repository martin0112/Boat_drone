void autopilot()
{
  
  if (autopilot_mode == 'M')
  {
    //Rudder
    if (commands[1] == 'L')
    {
      ledcWrite(0, u_rudder_max);
      ledcWrite(1, 0);


    }


    if (commands[1] == 'R')
    {
      ledcWrite(1, u_rudder_max);
      ledcWrite(0, 0);
    }

    if (commands[1] == 'N')
    {
      ledcWrite(1, 0);
      ledcWrite(0, 0);
    }
    //Motor
    int speed_percent = 0;
    int vorwarts = 0;
    int rueckwaerts = 0;

    int schnell = 0;
    int langsam = 0;


    if (commands[0] == 'V')
    {
      vorwarts = 1;
      schnell = 0;
      langsam = 0;
    }

    if (commands[0] == 'N')
    {
      vorwarts = 0;
      schnell = 0;
      langsam = 0;



    }

    if (commands[0] == 'Z')
    {
      vorwarts = -1;
      schnell = 0;
      langsam = 0;

    }



    if (commands[2] == 'S')
    {

      schnell = 1;
      langsam = 0;

    }

    if (commands[2] == 'L')
    {

      schnell = 0;
      langsam = 1;

    }



    speed_percent = vorwarts * (50 + schnell * 50 - langsam * 25) ;


    int geschwindigkeit = abs((int)(speed_percent / 100.0 * v_motor_max));


    if (speed_percent > 0)
    {
      ledcWrite(2, geschwindigkeit);
      ledcWrite(3, 0);

    }
    if (speed_percent < 0)
    {
      
      ledcWrite(3, geschwindigkeit);
      ledcWrite(2, 0);
         
    }
    if (speed_percent == 0)
    {
      ledcWrite(2, 0);
      ledcWrite(3, 0);
    }


  }

  

  if (autopilot_mode == 'C'&&gps.location.isValid())
  {
    
    error =(int)(heading_sp - heading_filter);
    if (error > 180)
    {
      error = error - 360;
    }

    else if (error < -180)
    {
      error = 360 + error;
    }
    float derror_dt = (error - last_error) / loop_time;
    int_error = int_error + error * loop_time;

    last_error = error;

    cv_rudder =(int)(kp * error + ki * int_error + td * derror_dt) ;
    int u_cv_rudder=cv_rudder<u_rudder_max?cv_rudder:u_rudder_max;
    Serial2.print("Data;");
    Serial2.print(heading_sp);
    Serial2.print(";");
    Serial2.print(heading_filter);
    Serial2.print(";");
    Serial2.print(error);
    Serial2.print(";");
    Serial2.print(cv_rudder);
    Serial2.print(";");
    Serial2.println(u_cv_rudder);
   
    
    //Rudder
    if (cv_rudder>=0)
    {
      ledcWrite(0, u_cv_rudder);
      ledcWrite(1, 0);


    }
      if (cv_rudder<0)
    {
      ledcWrite(1, u_cv_rudder);
      ledcWrite(0, 0);


    }

    float speed_percent = 50.0;
    int geschwindigkeit = abs((int)(speed_percent / 100.0 * v_motor_max));
    ledcWrite(2, geschwindigkeit);
    ledcWrite(3, 0);




  }



}
