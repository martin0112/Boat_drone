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

                    for (int i=0; i<befehl.order_gesamt.Length;i++)
                    {
                        befehl.order_gesamt[i] = "N";

                    }

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


                    //Zuordnen
                    if (key_vor)
                    {
                        befehl.order_gesamt[0] = "V";

                    }

                    if (key_vor&& key_schnell)
                    {
                        befehl.order_gesamt[0] ="S";

                    }

                

                    if (key_vor&& key_langsam)
                    {
                        befehl.order_gesamt[0] = "L";

                    }


                    if (key_zuruck)
                    {
                        befehl.order_gesamt[0] = "Z"; 

                    }

                    if (key_zuruck&&key_schnell)
                    {
                        befehl.order_gesamt[0] = "R";

                    }

                    if (key_links)
                    {
                        befehl.order_gesamt[1] = "B";

                    }


                    if (key_rechts)
                    {
                        befehl.order_gesamt[1] = "S";

                    }


                    form.SetTextBox(string.Concat(befehl.order_gesamt));

                    if (serialPort1.IsOpen==true)
                    { 
                    serial_write(befehl.order_gesamt);
                    }

                    //Neue Daten empfangen
                    // var time=timer_global.ElapsedMilliseconds;

                    if (timer_global.ElapsedMilliseconds > 2000)
                    {
                        neue_daten = false;
                        form.SetTextcolor(neue_daten);
                    }
                    


                    Thread.Sleep(10);



                }

            }

            public void RequestStop()
            {
                _shouldStop = true;

            }

        }


        public void SetTextBox(String text)
        {
            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker)delegate () { SetTextBox(text); });
                return;
            }
            textBox4.Text = text;
        }

        public void SetTextcolor(bool neu)
        {
            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker)delegate () { SetTextcolor(neu); });
                return;
            }

            if (neu == true)
            {
                label1.ForeColor = Color.Red;
            }
            else
            {
                label1.ForeColor = Color.Gray;

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
            //Update GUI
            Thread gui_thread = new Thread(update_gui);
            gui_thread.IsBackground = true;
            gui_thread.Start();
            //Get GPS Position

            Thread gps_thread = new Thread(gps_position_serial);
            if (ConfigurationManager.AppSettings["GPS_source"] == "fake")
            {
                gps_thread = new Thread(gps_position_fake);
            }

            gps_thread.IsBackground = true;
            gps_thread.Start();

        }

        public static class my_position
        {
            public static List<double> lat = new List<double>();
            public static List<double> lng = new List<double>();
            public static List<double> height = new List<double>();
            public static List<DateTime> time = new List<DateTime>();
            public static List<int> heading_waypoint = new List<int>();
            public static List<double> heading_gps = new List<double>();
            public static List<double> compass_heading = new List<double>();
            public static List<double> gps_speed = new List<double>();
            public static List<double> voltage = new List<double>();
            
            

        }


        public GMapOverlay markers_own_position = new GMapOverlay("eigene position");
        public GMapRoute movement_gps = new GMapRoute("bewegungsrichtung gps");


        private Object variable_update_lock = new Object();

        public void gps_position_serial()
        {


            while (true)
            {

                if (seriel_gps.new_position == true)
                {
                    lock (variable_update_lock)
                    {

                        my_position.lat.Add(seriel_gps.lat);
                        my_position.lng.Add(seriel_gps.lng);
                        my_position.height.Add(seriel_gps.height);
                        my_position.time.Add(seriel_gps.time);
                        my_position.gps_speed.Add(seriel_gps.speed);
                        my_position.heading_gps.Add(seriel_gps.movement_gps_angle);

                        seriel_gps.new_position = false;
                    }

                }

                Thread.Sleep(100);
            }


        }

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


        public void gps_position_fake()
        {
            //Initial position
            my_position.lat.Add(53.551086);
            my_position.lng.Add(9.993682);

            int k = 0;
            while (true)

            {
                lock (variable_update_lock)
                {
                    k = k + 1;
                    my_position.gps_speed.Add((double)(50 * (1.5 + 0.5 * Math.Sin((double)k / 10))));
                    my_position.heading_gps.Add(Math.Sin(((double)k / 500) + 1) * 360);
                    double[] next_position = haver_position(my_position.lat[my_position.lat.Count - 1], my_position.lng[my_position.lat.Count - 1], my_position.heading_gps[my_position.lat.Count - 1], my_position.gps_speed[my_position.lat.Count - 1] * 0.0001);
                    my_position.lat.Add(next_position[0]);
                    my_position.lng.Add(next_position[1]);
                    my_position.height.Add(500);


                    my_position.height.Add(500);

                    my_position.time.Add(DateTime.Now);
                    seriel_gps.new_position = true;

                }
                Thread.Sleep(100);
            }


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

        void update_gui()
        {

            while (true)
            {


                //Display current position
                display_current_position();

                //Display movement direction
                display_movement_direct();






                Thread.Sleep(100);
            }


        }

        void display_movement_direct()
        {
            double max_velocity = 200;//kmph
            double max_arrow = 10; //km

            if (my_position.gps_speed.Count > 0)
            {
                double arrow_length = (double)(my_position.gps_speed[my_position.gps_speed.Count - 1] / max_velocity * max_arrow);
                //calculate theoretical waypoint for movement indication
                double lat1 = my_position.lat[my_position.gps_speed.Count - 1];
                double lng1 = my_position.lng[my_position.gps_speed.Count - 1];



                //Linie ziehen zur Indikation bewegungsrichtung
                try { 
                movement_gps.Points.Clear();
                GMapOverlay line_overlay = new GMapOverlay();
                movement_gps.Stroke = new Pen(Brushes.Red, 1); //width and color of line
                line_overlay.Routes.Add(movement_gps);
                gmap.Overlays.Add(line_overlay);
                movement_gps.Points.Add(new PointLatLng(lat1, lng1));
                gmap.UpdateRouteLocalPosition(movement_gps);
                }
                catch
                { }




            }
        }

        void display_current_position()
        {
            

            if (my_position.lat.Count > 0)
            {
                //Wegpunkt eintragen
                gmap.Invoke(new Action(() => markers_own_position.Markers.Clear()));
                GMapMarker marker = new GMarkerGoogle(
                new PointLatLng(my_position.lat[my_position.lat.Count - 1], my_position.lng[my_position.lat.Count - 1]),
                GMarkerGoogleType.red_dot);
                gmap.Invoke(new Action(() => gmap.Overlays.Add(markers_own_position)));
                markers_own_position.Markers.Add(marker);
                

            }
        }





        static SerialPort serialPort1;

        public static void serial_write(string[] control_array)
        {

            if (serialPort1.IsOpen)
            {

                try
                {
                    serialPort1.Write("X");

                    for (int i = 0; i < control_array.GetLength(0); i++)
                    {
                        serialPort1.Write(control_array[i]);
                    }

                    serialPort1.Write("E");








                }
                catch (Exception ex)
                {

                }

            }




        }

        

        
       







        public static class befehl
        {

           
            public static string[] order_gesamt = { "N", "N", "N", "N", "N" };

            /*
            0. S- Schnell Vor
            0. V- Normal Vor
            0. L- Langsam Vor
            0. N- Motor Aus
            0. Z- Normal Zurück
            0. R- Schnell Zurück
            1. L- Hart links 
            1. b- links
            1. N- Ruder Neutral
            1. R- Hart Rechts
            1. S- Rechts
            2. H- Handshake Anfordern
            3. P- Position Anfoderdern



             * */


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
                
                serialPort1.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                serialPort1.Open();
                if (serialPort1.IsOpen)
                MessageBox.Show("Verbindung erstellt " + serialPort1.PortName);

                //serial_write(befehl.order_gesamt);


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
            int c = sp.ReadChar();
            if (c==82)
            {
              
                received = sp.ReadTo("E");
                if (received.Length >35 && received.Contains("H") && received.Contains("V") && received.Contains("L"))
                {

                    SetTextBox_received(received);
                    String[] lat = received.Split('L');
                    String[] lon = lat[1].Split('V');

                    try {
                    if (lat[0].Length==8&& lon[0].Length==8)
                        { 
                    seriel_gps.lat =Convert.ToDouble(lat[0],System.Globalization.CultureInfo.InvariantCulture) ;
                    seriel_gps.lng = Convert.ToDouble(lon[0], System.Globalization.CultureInfo.InvariantCulture);
                    
                    seriel_gps.new_position = true;
                        }
                    }
                    catch
                    { }
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
            if(my_position.lat.Count>0)
            {
                try {
                    var lat = my_position.lat[my_position.lat.Count-1];
                    var lon = my_position.lng[my_position.lat.Count - 1];
                    gmap.Invoke(new Action(() => gmap.Position = new PointLatLng(lat, lon)));
                }
                catch
                { }
            }
        }
    }
}
