//Boot, Telegramm mit 
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using System.IO.Ports;
using System.Windows.Input;
using AForge.Video;
using AForge.Video.DirectShow;
using System.Diagnostics;
using System.Configuration;
using GMap.NET.MapProviders;
using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace Aervo_2_axis_hmi
{


    public partial class Form1 : Form
    {


        public Form1()
        {
            InitializeComponent();
            initialisierung();
            Worker workerObject = new Worker();
            Thread workerThread = new Thread(workerObject.DoWork);
            workerThread.SetApartmentState(ApartmentState.STA);
            workerThread.IsBackground = true;
            workerThread.Start(this);







            
            FilterInfoCollection videosources = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            //Überprüfen, ob mindestens eine Aufnahmequelle vorhanden ist
            if (videosources.Count>0)
            {
                //Die erste Aufnahmequelle an unser Webcam Objekt binden
                //(habt ihr mehrere Quellen, muss nicht immer die erste Quelle die
                //gewünschte Webcam sein!)
                int videosource = Convert.ToInt32(ConfigurationManager.AppSettings["WEBCAM_ID"]);
                videoSource = new VideoCaptureDevice(videosources[videosource].MonikerString);
                


                try
                {
                    //Überprüfen ob die Aufnahmequelle eine Liste mit möglichen Aufnahme-
                    //Auflösungen mitliefert.
                    if (videoSource.VideoCapabilities.Length > 0)
                    {
                        string highestSolution = "0;0";
                        //Das Profil mit der höchsten Auflösung suchen
                        for (int i = 0; i < videoSource.VideoCapabilities.Length; i++)
                        {
                            if (videoSource.VideoCapabilities[i].FrameSize.Width > Convert.ToInt32(highestSolution.Split(';')[0]))
                                highestSolution = videoSource.VideoCapabilities[i].FrameSize.Width.ToString() + ";" + i.ToString();
                        }
                        //Dem Webcam Objekt ermittelte Auflösung übergeben
                        videoSource.VideoResolution = videoSource.VideoCapabilities[Convert.ToInt32(highestSolution.Split(';')[1])];
                    }
                }
                catch { }

                //NewFrame Eventhandler zuweisen anlegen.
                //(Dieser registriert jeden neuen Frame der Webcam)
                videoSource.NewFrame += new AForge.Video.NewFrameEventHandler(videoSource_NewFrame);

                //Das Aufnahmegerät aktivieren
                videoSource.Start();
            }




        }

        VideoCaptureDevice videoSource;

        public class Worker
        {
            private volatile bool _shouldStop;

            public void DoWork(Object parameter)
            {
                Form1 form = parameter as Form1;

                bool neue_daten = false;

                //Stopwatch_timer Neue Messdaten         

                Stopwatch timer_global = new Stopwatch();
                timer_global.Start();

                if (form == null) return;

                while (!_shouldStop)
                {

                    

                    string order;


                    //Abfrage keyboard
                    bool key_vor = Keyboard.IsKeyDown(Key.Up);
                    bool key_zuruck = Keyboard.IsKeyDown(Key.Down);
                    bool key_links = Keyboard.IsKeyDown(Key.Left);
                    bool key_rechts = Keyboard.IsKeyDown(Key.Right);
                    bool key_schnell = Keyboard.IsKeyDown(Key.RightShift);
                    bool key_langsam = Keyboard.IsKeyDown(Key.RightCtrl);

                    //Funktionstasten abfragen
                    bool key_function1 = Keyboard.IsKeyDown(Key.A);

                    /*
                    0. S- Schnell Vor
                    0. V- Normal Vor
                    0. L- Langsam Vor
                    0. N- Motor Aus
                    0. Z- Normal Zurück
                    0. R- Schnell Zurück
                    1. B- links
                    1. N- Ruder Neutral
                    1. S- Rechts
                    2. H- Handshake Anfordern
                    3. P- Position Anfoderdern

                     * */

                    order = "N";
                    //Zuordnen
                    if (key_vor)
                    {
                        order = "V";

                    }

                    if (key_vor&& key_schnell)
                    {
                        order ="S";

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


                    if (key_vor&& key_links&&key_schnell)
                    {
                        order ="A";

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

                    if (key_zuruck&&key_rechts)
                    {
                        order = "H";

                    }

                    if (key_zuruck&&key_links)
                    {
                        order = "G";

                    }

                    if ( key_links && key_vor == false && key_zuruck == false)
                    {
                        order = "O";

                    }

                    if (key_rechts&&key_vor==false&&key_zuruck==false)
                    {
                        order = "P";

                    }





                    if (heading_follower == true&&heading_sp<=360&& heading_sp>=0)
                    {
                        string heading_string = heading_sp.ToString();
                        if (heading_sp < 10)
                        {
                            heading_string = "0" + heading_string;

                        }
                        if (heading_sp < 100)
                        {
                            heading_string = "0" + heading_string;

                        }

                        order = "H"+ heading_string[0].ToString()+ heading_string[1].ToString()+ heading_string[2].ToString();
                    
                        
                    }

                    if (parameter_update)
                    {
                        order = "P" + Kp.ToString()+";" + Kp.ToString() + ";" + Kp.ToString();


                        parameter_update = false;


                    }

                    if (serialPort1.IsOpen==true)
                    { 
                    serial_write(order);

                        Thread.Sleep(500);


                        if (serialPort1.BytesToRead > 0)
                        {
                            string empfangen = serialPort1.ReadExisting();

                        }


                    }

                   
                    


                    //Neue Daten empfangen
                    // var time=timer_global.ElapsedMilliseconds;







                }

            }

            public void RequestStop()
            {
                _shouldStop = true;

            }

        }


        



        public void SetTextBox_received(String text)
        {



            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker)delegate () { SetTextBox_received(text); });
                    return;
            }

            label1.Text = text;

        }


        void initialisierung()
        {
            serialPort1 = new SerialPort();
            string[] ports = SerialPort.GetPortNames();
            listBox1.Items.AddRange(ports);
            this.FormBorderStyle = FormBorderStyle.None;
            this.WindowState = FormWindowState.Maximized;
            var pos = this.PointToScreen(label1.Location);
            pos = pictureBox1.PointToClient(pos);
            label1.BackColor = Color.Transparent;
            label1.Location = pos;
            label1.Parent = pictureBox1;
            pictureBox1.SizeMode=PictureBoxSizeMode.StretchImage;


            gmap.CacheLocation = ConfigurationManager.AppSettings["cache_folder"];
            WindowState = FormWindowState.Maximized;
            

        }

        public static int heading_sp;
        public static bool heading_follower;
        public static bool parameter_update;
        public static double Kp,Ki,Kd;


        public GMapOverlay markers_own_position = new GMapOverlay("eigene position");
    

        


        public class haversin_const
        {
            public const double radius_earth = 6399.594;
            public const double convertsion_degrees_radiant = (double)(Math.PI / 180);
            public const double convertsion_radiant_degrees = (double)(180 / Math.PI);


        }


        double[] haver_position(double lat1, double lng1, double heading, double distance)
        {
            //Haversin projektion auf neue Position
            double[] destination = new double[2];
            double lat1_rad = lat1 * haversin_const.convertsion_degrees_radiant;
            double lng1_rad = lng1 * haversin_const.convertsion_degrees_radiant;
            double heading_rad = heading * haversin_const.convertsion_degrees_radiant;
            double angular_distance = (double)(distance / haversin_const.radius_earth);
            //latitude destination
            destination[0] = Math.Asin(Math.Sin(lat1_rad) * Math.Cos(angular_distance) + Math.Cos(lat1_rad) * Math.Sin(angular_distance) * Math.Cos(heading_rad)) * haversin_const.convertsion_radiant_degrees;
            destination[1] = (lng1_rad + Math.Atan2(Math.Sin(heading_rad) * Math.Sin(angular_distance) * Math.Cos(lat1_rad), Math.Cos(angular_distance) - Math.Sin(lat1_rad) * Math.Sin(destination[0] * haversin_const.convertsion_degrees_radiant))) * haversin_const.convertsion_radiant_degrees;
            return destination;

        }



        public class seriel_gps
        {
            public static double lat;
            public static double lng;
            public static double height;
            public static DateTime time;
            public static double movement_gps_angle;
            public static double speed;
            public static bool valid_gps;
            public static bool new_position;
            public static double voltage;
        }

        

        void display_current_position(double lat,double lon)
        {
            

            
                gmap.Invoke(new Action(() => markers_own_position.Markers.Clear()));
                GMapMarker marker = new GMarkerGoogle(
                new PointLatLng(lat, lon),
                GMarkerGoogleType.red_small);
                gmap.Invoke(new Action(() => gmap.Overlays.Add(markers_own_position)));
                markers_own_position.Markers.Add(marker);
                

            
        }





        static SerialPort serialPort1;

        public static void serial_write(string control_string)
        {



            if (serialPort1.IsOpen)
            {

                try
                {
                    serialPort1.Write("X");

                 
                        serialPort1.Write(control_string);
                    

                    serialPort1.Write("J");








                }
                catch (Exception ex)
                {

                }

            }




        }

        

        
       









        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }



        private void textBox1_TextChanged_1(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {


        }


        void videoSource_NewFrame(object sender, AForge.Video.NewFrameEventArgs eventArgs)
        {
            //Jedes ankommende Objekt als Bitmap casten und der Picturebox zuweisen
            //(Denkt an das ".Clone()", um Zugriffsverletzungen aus dem Weg zu gehen.)
            pictureBox1.BackgroundImage = (Bitmap)eventArgs.Frame.Clone();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (videoSource != null && videoSource.IsRunning)
            {
                videoSource.SignalToStop();
                videoSource = null;
            }

            Environment.Exit(Environment.ExitCode);

        }

        private void button1_Click(object sender, EventArgs e)
        {
           
            serialPort1.BaudRate = Convert.ToInt32(ConfigurationManager.AppSettings["seriel_speed"]);
            try
            {
                serialPort1.PortName = listBox1.GetItemText(listBox1.SelectedItem); ; // Set in Windows
 

                serialPort1.Open();

                if (serialPort1.IsOpen)
                serialPort1.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

                MessageBox.Show("Verbindung erstellt " + serialPort1.PortName);

              


            }
            catch (Exception ex)
            {
                MessageBox.Show("Fehler" + ex);
            }
        }

        public void DataReceivedHandler(
                    object sender,
                    SerialDataReceivedEventArgs e)
        {
            string received = "";

            SerialPort sp = (SerialPort)sender;
            
            if (sp.BytesToRead>0)
            { 
            int c = sp.ReadChar();
            if (c=='T')
            {
                received = sp.ReadTo("T");
              
                if (received.Length >35 && received.Contains("H") && received.Contains("V") && received.Contains("L") && received.Contains("P"))
                {
                        try
                        {

                    String[] lat = received.Split('L');
                    String[] lon = lat[1].Split('V');
                    String[] vel = lon[1].Split('H');
                    String[] heading = vel[1].Split('V');
                    String[] voltage = lon[2].Split('P');
                    String[] power = voltage[1].Split('A');
                    String autopilot_info = power[1];
                    SetTextBox_received("Lat:"+lat[0]+ " Lon:"+lon[0]+ " \nVelocity:" + vel[0]+" km/h"+ "\nHeading:" + heading[0]+ " Autopilot: "+autopilot_info+" ° \nVoltage: " +voltage[0]+ " V" );

                    if (lat[0].Length==8&& lon[0].Length==8)
                        { 
                    seriel_gps.lat =Convert.ToDouble(lat[0],System.Globalization.CultureInfo.InvariantCulture) ;
                    seriel_gps.lng = Convert.ToDouble(lon[0], System.Globalization.CultureInfo.InvariantCulture);
                    display_current_position(Convert.ToDouble(lat[0], System.Globalization.CultureInfo.InvariantCulture), Convert.ToDouble(lon[0], System.Globalization.CultureInfo.InvariantCulture));


                        }
                    }
                    catch
                    { }
                }
                }
            }


        }


        

        public static Stopwatch timer_global = new Stopwatch();

        private void gmap_Load(object sender, EventArgs e)
        {

            gmap.ShowCenter = true;
            gmap.MapProvider = BingMapProvider.Instance;
            gmap.Zoom = Convert.ToInt16(ConfigurationManager.AppSettings["Zoom"]);
            gmap.Manager.Mode = AccessMode.ServerAndCache;
            gmap.SetPositionByKeywords(ConfigurationManager.AppSettings["map_location"]); 
            GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
            
        }

        private void button3_Click(object sender, EventArgs e)
        {
                try {
                   
                    if (markers_own_position.Markers.Count>0)
                {
                    try { 
                    gmap.Invoke(new Action(() => gmap.Position = markers_own_position.Markers[0].Position));
                    }
                    catch
                    { }
                }
            }
                catch
                { }
            
        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {
               if (textBox4.Text.Count()>0) 
                heading_sp = Convert.ToInt16(textBox4.Text);
            
        
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            
        }

        private void radioButton1_Click(object sender, EventArgs e)
        {
            


        }

        private void button4_Click(object sender, EventArgs e)
        {
            if (heading_follower == false) 
            {
                heading_follower = true;
                button4.BackColor = Color.Green;
                heading_sp = Convert.ToInt16(textBox4.Text);
            }
            else
            {
                heading_follower = false;
                button4.BackColor = Color.Red;



            }

        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void button5_Click(object sender, EventArgs e)
        {
            Kp = Convert.ToDouble(textBox1.Text);
            Ki = Convert.ToDouble(textBox2.Text);
            Kd = Convert.ToDouble(textBox3.Text);
            parameter_update = true;

        }
    }
}
