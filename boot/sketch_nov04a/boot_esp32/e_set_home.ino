void set_home()
{
  if ((switch_state && gps.location.isValid()) || (home_set == false && gps.location.isValid()))
  {
    lat_home = lati;
    lng_home = lon;
    int lati_digits = (lati - int(lati)) * 100000;
    int longi_digits = (lon - int(lon)) * 100000;


    String temp_radio = "H" + String(lati_digits) + "L" + String(longi_digits) + "E";
    
    Serial2.println(temp_radio);

    home_set = true;

    switch_state_float = 0;
  }


}
