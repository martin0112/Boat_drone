void check_emergency() //Falls kein Funksignal
{
  if ((millis() - last_received) > home_after_not_received)
  {
    autopilot_mode = 'C';
    heading_sp = gps.courseTo((double)lati, (double)lon, lat_home, (double)lng_home);

  }
  


}

void loop_time_measurement()
{

  n_loop++;

  if (n_loop == loop_average)
  {
    n_loop = 0;
    loop_time = (millis() - previos_micros) / loop_average / 1000.0;
    //Serial.println(loop_time);
    previos_micros = millis();


  }
}
