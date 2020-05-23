using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Configuration;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using DirectX.Capture;
using DShowNET;
using System.Windows.Input;
using GMap.NET.MapProviders;
using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;

namespace Boat_New
{
    static class Methoden
    {

        public class kontrolle
        {

            

            public static void check_key(TextBox text)
            {
                

                bool key_vor = Keyboard.IsKeyDown(Key.Up);
                bool key_zuruck = Keyboard.IsKeyDown(Key.Down);
                bool key_links = Keyboard.IsKeyDown(Key.Left);
                bool key_rechts = Keyboard.IsKeyDown(Key.Right);
                bool key_schnell = Keyboard.IsKeyDown(Key.RightShift);
                bool key_langsam = Keyboard.IsKeyDown(Key.RightCtrl);

                string order = "N";
                //Zuordnen
                if (key_vor)
                {
                    order = "V";

                }

                if (key_vor && key_schnell)
                {
                    order = "S";

                }
                if (key_vor && key_langsam)
                {
                    order = "L";

                }

                if (key_vor && key_links)
                {
                    order = "B";

                }

                if (key_vor && key_rechts)
                {
                    order = "E";

                }

                if (key_vor && key_langsam)
                {
                    order = "L";

                }


                if (key_vor && key_links && key_schnell)
                {
                    order = "A";

                }



                if (key_vor && key_links && key_langsam)
                {
                    order = "C";

                }

                if (key_vor && key_rechts && key_schnell)
                {
                    order = "D";

                }


                if (key_vor && key_rechts && key_langsam)
                {
                    order = "F";

                }





                if (key_zuruck)
                {
                    order = "K";

                }

                if (key_zuruck && key_rechts)
                {
                    order = "H";

                }

                if (key_zuruck && key_links)
                {
                    order = "G";

                }

                if (key_links && key_vor == false && key_zuruck == false)
                {
                    order = "O";

                }

                if (key_rechts && key_vor == false && key_zuruck == false)
                {
                    order = "P";

                }




                text.Text = order;
       
                
                Methoden.Serielles.serial_write(order);
                


            }






        }

        public static Capture cam;


        

        public  class Camera
        {



            




            public static void create_camera(PictureBox pic)
            {
                /*
                int camera_id=Convert.ToInt32(ConfigurationManager.AppSettings["WEBCAM_ID"]);
                Filters filters = new Filters();
                if ( filters.VideoInputDevices!=null)
                { 
                var temp_camera = new Capture(filters.VideoInputDevices[camera_id],filters.AudioInputDevices[camera_id]);
                
                temp_camera.PreviewWindow = pic;
                cam = temp_camera;
                cam.FrameRate = 20;
                }
                */
            }
            

            


          
        }

        public class GPS_stuff
        {

            public static void karte_initialisieren(GMap.NET.WindowsForms.GMapControl karte)
            {
                
                
                karte.ShowCenter = true;
                karte.MapProvider = BingMapProvider.Instance;
                karte.Zoom = Convert.ToInt16(ConfigurationManager.AppSettings["Zoom"]);
                karte.Manager.Mode = AccessMode.ServerAndCache;
                karte.SetPositionByKeywords(ConfigurationManager.AppSettings["map_location"]);
                


            }


            void display_current_position(double lat, double lon, GMap.NET.WindowsForms.GMapControl karte, GMapOverlay own_positions)
            {



                karte.Invoke(new Action(() => own_positions.Markers.Clear()));
                GMapMarker marker = new GMap.NET.WindowsForms.Markers.GMarkerGoogle(
                new PointLatLng(lat, lon),
                GMarkerGoogleType.red_small);
                karte.Invoke(new Action(() => karte.Overlays.Add(own_positions)));
                own_positions.Markers.Add(marker);



            }

        }

        

        public class Serielles  //??Static
        {
            public static List<SerialPort> Serielle_ports = new List<SerialPort>(Serial_ports_einlesen());

            public static SerialPort selected_port = new SerialPort();

         
            public static bool selected = false;

            public static void serial_write(string command)
            {




                if (selected_port.IsOpen)
                {

                    try
                    {
                        selected_port.Write("X");


                        selected_port.Write(command);


                        selected_port.Write("J");








                    }
                    catch (Exception ex)
                    {

                    }



                }

            }

            


            public static void DataReceivedHandler(
                    object sender,
                    SerialDataReceivedEventArgs e) 
            {
                int c = selected_port.ReadChar();
                if (c == 82)
                {
                    var received = selected_port.ReadTo("E");
                                    if (received.Contains("C")&& received.Contains("S")&& received.Contains("L") && received.Contains("V"))
                {
                    try
                    { 
                    var test= received.Split('L');
                    
                    var lat = Convert.ToDouble(test[0]) / 10000000;
                    var test2 = test[1].Split('S');
                    var lng = Convert.ToDouble(test2[0]) / 10000000;
                    var test3 = test2[1].Split('C');
                    var speed = Convert.ToDouble(test3[0]) / 100;
                    var test4 = test3[1].Split('V');
                    var movement_gps_angle = Convert.ToDouble(test4[0])/100;
                    var voltage = Convert.ToDouble(test4[1])/100;


                        

                        }
                    catch
                    { }


                    }
            }

               

            }

            public static List<SerialPort> Serial_ports_einlesen()
            {
                var temp_serial_ports = new List<SerialPort>();
                var list = SerialPort.GetPortNames();
                foreach (string item in SerialPort.GetPortNames())
                {
                    var temp_serialport = new SerialPort();
                    temp_serialport.BaudRate = Convert.ToInt32(ConfigurationManager.AppSettings["seriel_speed"]);
                    temp_serialport.PortName = item;

                    temp_serialport.DataReceived+=new SerialDataReceivedEventHandler(DataReceivedHandler);
                    
                    temp_serial_ports.Add(temp_serialport);
                    
                }

                

                return temp_serial_ports;
            }

        }

    }
}
