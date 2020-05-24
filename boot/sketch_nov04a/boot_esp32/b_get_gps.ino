void get_GPS()
{

  while (Serial1.available() > 0)
  {
    if (gps.encode(Serial1.read()))
    {



      if (gps.location.isValid())
      {


        //Serial.println("FIX");

        lati = gps.location.lat();
        lon = gps.location.lng();
        heading = gps.course.deg();
        heading_filter= (1 - T_accel) * heading + T_accel * heading_filter;
        velocity = gps.speed.mps();
        no_sat = gps.satellites.value();
        fix = true;


      }
      else
      {

        no_sat = gps.satellites.value();
        fix = false;
        //Serial.println(String(no_sat,0));
        lati = 0;
        lon = 0;
        heading = 0;
        velocity = 0;

      }
    }


  }
}

