using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using AForge;
using AForge.Video;
using AForge.Video.DirectShow;
using System.Configuration;
using System.IO.Ports;
using System.Windows.Threading;
using System.ComponentModel;
using System.Collections.ObjectModel;



namespace remote_II
{





    public partial class MainWindow : Window
    {
        static public List<FilterInfo> Cameras = new List<FilterInfo>();
        static public List<SerialPort> Serial_ports = new List<SerialPort>();
        static public SerialPort Port_selected;
        static public DispatcherTimer map_update_timer = new DispatcherTimer();
        public static Dictionary<string, bool> commands = new Dictionary<string, bool>() { };
        static public DispatcherTimer key_timer = new DispatcherTimer();



        public static class telemetry
        {

            public static double LAT;
            public static double LNG;
            public static double LAT_old;
            public static double LNG_old;

            public static double LAT_HOME;
            public static double LNG_HOME;

            public static double vel;
            public static double heading;

            
            public static string status_0;
            public static string status_1;
            public static bool received_data = false;

            public static String debug_text;

            public static DateTime last_update = DateTime.Now;
            public static void tel_update(double lati, double longi, double velo = 0, string status0 = "", string status1 = "", double head = 0)
            {

                double lat_com = Convert.ToDouble(ConfigurationManager.AppSettings["lat_comp"]);
                double lon_com = Convert.ToDouble(ConfigurationManager.AppSettings["lng_comp"]);
                if (lati == 0 || longi == 0)
                {
                    status_1 = "No GPS";

                }
                else
                {
                    LAT_old = LAT;
                    LNG_old = LNG;
                    LAT = lat_com + lati / 100000;
                    LNG = lon_com + longi / 100000;
                    vel = velo;
                    heading = head;
                    status_1 = "GPS FIX";


                }

                status_0 = status0;

                last_update = DateTime.Now;


            }
            public static string telemetry_text()
            {
                string ausgabe = "";
                if ((DateTime.Now - last_update).TotalSeconds > 10)
                {
                    ausgabe = Convert.ToString(last_update) + "\n" + Convert.ToString(LAT) + "\n" + Convert.ToString(LNG) + "\n" + Convert.ToString(vel) + " m/s" + "\n" + Convert.ToString(heading) + " °" + "\n" + status_0 + "\n" + status_1 + "\n" + "No Downlink: " + Convert.ToString(Math.Round((DateTime.Now - last_update).TotalSeconds)) + " s";

                }
                else
                {
                    ausgabe = Convert.ToString(last_update) + "\n" + Convert.ToString(LAT) + "\n" + Convert.ToString(LNG) + "\n" + Convert.ToString(vel) + " m/s" + "\n" + Convert.ToString(heading) + " °" + "\n" + status_0 + "\n" + status_1;


                }
                return ausgabe;
            }

            public static void home_set(double lati, double longi)
            {
                double lat_com = Convert.ToDouble(ConfigurationManager.AppSettings["lat_comp"]);
                double lon_com = Convert.ToDouble(ConfigurationManager.AppSettings["lng_comp"]);
                if (lati == 0 || longi == 0)
                {
                    status_1 = "No GPS";

                }
                else
                {
                    LAT_HOME = lat_com + lati / 100000;
                    LNG_HOME = lon_com + longi / 100000;
                    status_0 = "HOME SET";


                }
            }
        }



        public static class Map_state
        {
            public static double zoom = 15;

        }

        void Cam_NewFrame(object sender, NewFrameEventArgs eventArgs)
        {
            try
            {
                System.Drawing.Image img = (Bitmap)eventArgs.Frame.Clone();

                MemoryStream ms = new MemoryStream();
                img.Save(ms, ImageFormat.Bmp);
                ms.Seek(0, SeekOrigin.Begin);
                BitmapImage bi = new BitmapImage();
                bi.BeginInit();
                bi.StreamSource = ms;
                bi.EndInit();

                bi.Freeze();

                Dispatcher.BeginInvoke(new ThreadStart(delegate
                {

                    frameHolder.Source = bi;
                }));
            }
            catch (Exception ex)
            {
            }
        }


        public MainWindow()
        {
            InitializeComponent();
            scan_cameras();
            scan_ports();

            //Update map timer
            map_update_timer.Tick += new EventHandler(Update_map);
            map_update_timer.Interval = TimeSpan.FromSeconds(0.5);

            //telemetry
            map_update_timer.Tick += new EventHandler(Update_telemetry);
            map_update_timer.Start();

            //Initialize commands
            commands["Vor"] = false;
            commands["Links"] = false;
            commands["Rechts"] = false;
            commands["Zurueck"] = false;
            commands["Schnell"] = false;
            commands["Langsam"] = false;
            commands["Function_0"] = false;
            commands["Manuell"] = true;
            commands["Home"] = false;
            commands["Autopilot"] = false;
            commands["Set_home"] = false;
            commands["Course"] = false;

            //Keys check timer
            key_timer.Interval = TimeSpan.FromSeconds(0.05);
            key_timer.Tick += new EventHandler(Check_keys);
            key_timer.Start();



        }

        private void send_commands()
        {
            char[] commands_send = new char[5] { 'N', 'N', 'N', 'N', 'N' };

            if (commands["Zurueck"] == true)
                commands_send[0] = 'Z';
            if (commands["Vor"] == true)
                commands_send[0] = 'V';
            if (commands["Zurueck"] == false && commands["Vor"] == false)
                commands_send[0] = 'N';
            if (commands["Links"] == true)
                commands_send[1] = 'L';
            if (commands["Rechts"] == true)
                commands_send[1] = 'R';
            if (commands["Links"] == false && commands["Rechts"] == false)
                commands_send[1] = 'N';
            if (commands["Schnell"] == true)
                commands_send[2] = 'S';
            if (commands["Langsam"] == true)
                commands_send[2] = 'L';
            if (commands["Schnell"] == false && commands["Langsam"] == false)
                commands_send[2] = 'N';
            if (commands["Function_0"] == true)
                commands_send[4] = 'F';

            if (commands["Manuell"] == true && Port_selected != null && Port_selected.IsOpen)
            {
                Port_selected.Write("O");
                Port_selected.Write(commands_send, 0, 5);
                Port_selected.Write("X");
            }
            if (commands["Home"] == true && Port_selected != null && Port_selected.IsOpen)
            {
                Port_selected.Write("H");
                Port_selected.Write("H");

            }

            if (commands["Course"] == true && Port_selected != null && Port_selected.IsOpen)
            {
                Port_selected.Write("H");
                Port_selected.Write("C");
                string course = "000";
                try { 
                course=Convert.ToInt32(Course_SP.Text).ToString().PadLeft(3, '0');
                }
                catch
                {
                    course = "000";
                
                }
                Port_selected.Write(course);
            }







        }
        private void Check_keys(object sender, EventArgs e)
        {

            commands["Vor"] = Keyboard.IsKeyDown(Key.W);
            commands["Links"] = Keyboard.IsKeyDown(Key.A);
            commands["Rechts"] = Keyboard.IsKeyDown(Key.D);
            commands["Zurueck"] = Keyboard.IsKeyDown(Key.S);
            commands["Schnell"] = Keyboard.IsKeyDown(Key.LeftShift);
            commands["Langsam"] = Keyboard.IsKeyDown(Key.LeftCtrl);
            commands["Function_0"] = Keyboard.IsKeyDown(Key.F);
            commands["Set_home"] = Keyboard.IsKeyDown(Key.H) && Keyboard.IsKeyDown(Key.LeftCtrl);


            if (commands["Vor"] || commands["Links"] || commands["Rechts"] || commands["Zurueck"] || commands["Schnell"] || commands["Langsam"])
            {
                commands["Home"] = false;
                commands["Autopilot"] = false;
                commands["Manuell"] = true;
                commands["Course"] = false;

            }


            if (Keyboard.IsKeyDown(Key.H))
            {
                commands["Home"] = true;
                commands["Autopilot"] = false;
                commands["Manuell"] = false;
                commands["Course"] = false;


            }
            if (Keyboard.IsKeyDown(Key.G))
            {
                commands["Autopilot"] = true;
                commands["Home"] = false;
                commands["Manuell"] = false;
                commands["Course"] = false;


            }
        


            /*
            String command_text = "";
            foreach (var item in commands) 
            {
                command_text += item.Key+" "+item.Value+";";
            }
            Orders.Text = command_text;*/
            send_commands();

        }

        private void mapView_Loaded(object sender, RoutedEventArgs e)
        {
            telemetry.tel_update(Convert.ToDouble(ConfigurationManager.AppSettings["init_lat"]), Convert.ToDouble(ConfigurationManager.AppSettings["init_lng"]));

            GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
            mapView.CacheLocation = @"C:\cache\";
            // choose your provider here
            mapView.MapProvider = GMap.NET.MapProviders.OpenStreetMapProvider.Instance;
            mapView.MinZoom = 2;
            mapView.MaxZoom = 22;
            mapView.ShowCenter = false;
            // whole world zoom
            mapView.Position = new GMap.NET.PointLatLng(telemetry.LAT, telemetry.LNG);
            mapView.Zoom = 15;
            // lets the map use the mousewheel to zoom
            mapView.MouseWheelZoomType = GMap.NET.MouseWheelZoomType.MousePositionAndCenter;
            // lets the user drag the map
            mapView.CanDragMap = true;
            // lets the user drag the map with the left mouse button
            mapView.DragButton = MouseButton.Left;
            var own_pos = new GMap.NET.WindowsPresentation.GMapMarker(new GMap.NET.PointLatLng(telemetry.LAT, telemetry.LNG));
            own_pos.Shape = new Ellipse
            {
                Width = 20,
                Height = 20,
                Stroke = System.Windows.Media.Brushes.Black,
                StrokeThickness = 4
            };
            mapView.Markers.Add(own_pos);

        }




        public void Update_telemetry(object sender, EventArgs e)
        {

            Telemetry_text.Text = telemetry.telemetry_text();

            Debug_output.Text = telemetry.debug_text;



        }


        public void scan_cameras()
        { var videoDevices = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            foreach (FilterInfo item in videoDevices)
            {
                Cameras.Add(item);

            }
            Kamera_Liste.ItemsSource = Cameras;
            Kamera_Liste.DisplayMemberPath = "Name";
            Kamera_Liste.SelectedIndex = 0;


        }

        public void scan_ports()
        {
            var ports = SerialPort.GetPortNames();
            foreach (String item in ports)
            {
                SerialPort new_port = new SerialPort();
                new_port.PortName = item;
                new_port.BaudRate = Convert.ToInt32(ConfigurationManager.AppSettings["Baud"]);
                Serial_ports.Add(new_port);


            }



            Port_Liste.ItemsSource = Serial_ports;
            Port_Liste.SelectedIndex = 0;
            Port_Liste.DisplayMemberPath = "PortName";


        }



        private void Button_Click(object sender, RoutedEventArgs e)
        {
            int index_selected = Kamera_Liste.SelectedIndex;

            var LocalWebCam = new VideoCaptureDevice(Cameras[index_selected].MonikerString);
            LocalWebCam.NewFrame += new AForge.Video.NewFrameEventHandler(Cam_NewFrame);
            LocalWebCam.Start();

        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            Environment.Exit(0);
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            mapView.Zoom += 0.5;
        }
        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            mapView.Position = new GMap.NET.PointLatLng(telemetry.LAT, telemetry.LNG);

        }
        private void Button_Click_4(object sender, RoutedEventArgs e)
        {
            mapView.Zoom -= 0.5;

        }

        public class boat_symbol
        {
            public System.Windows.Shapes.Polyline boot_symbol;

            public boat_symbol(double heading, double velocity)
            {
                double[] vector = new double[2]; //index 0 ist horizontal, 1 vertikal

                if (heading <= 90 && heading >= 0)
                {
                    vector[0] = Math.Sin(heading);
                    vector[1] = Math.Cos(heading);

                }

                if (heading > 90 && heading <= 180)
                {
                    vector[0] = Math.Cos(heading - 90);
                    vector[1] = -Math.Sin(heading - 90);


                }

                if (heading > 180 && heading <= 270)
                {
                    vector[0] = -Math.Sin(heading - 180);
                    vector[1] = -Math.Cos(heading - 180);


                }

                if (heading > 270 && heading <= 359)
                {
                    vector[0] = -Math.Cos(heading - 270);
                    vector[1] = Math.Sin(heading - 270);


                }

                boot_symbol.StrokeThickness = 2;
                boot_symbol.Stroke = System.Windows.Media.Brushes.Red;
                boot_symbol.Points.Add(new System.Windows.Point(0, 0));
                boot_symbol.Points.Add(new System.Windows.Point(vector[0] * 10, vector[1] * 10));
                boot_symbol.Points.Add(new System.Windows.Point((vector[0] + 2) * 10, (vector[0] + 2) * 10));
                boot_symbol.Points.Add(new System.Windows.Point((vector[0] - 2) * 10, (vector[0] - 2) * 10));

            }



        }

        public void Update_map(object sender, EventArgs e)
        {
            mapView.Markers.Clear();
            var own_pos = new GMap.NET.WindowsPresentation.GMapMarker(new GMap.NET.PointLatLng(telemetry.LAT, telemetry.LNG));
            var home_pos = new GMap.NET.WindowsPresentation.GMapMarker(new GMap.NET.PointLatLng(telemetry.LAT_HOME, telemetry.LNG_HOME));

            double direction = telemetry.heading;

            var myPolyline = new Polyline();
            myPolyline.Stroke = System.Windows.Media.Brushes.Red;
            myPolyline.StrokeThickness = 2;


            PointCollection myPointCollection2 = new PointCollection();
            myPointCollection2.Add(new System.Windows.Point(-10 - 10 * telemetry.vel / 1, -10 - 10 * telemetry.vel / 1));
            myPointCollection2.Add(new System.Windows.Point(-10, -10));
            myPointCollection2.Add(new System.Windows.Point(-10, 10));
            myPointCollection2.Add(new System.Windows.Point(10, 10));
            myPointCollection2.Add(new System.Windows.Point(10, -10));
            myPointCollection2.Add(new System.Windows.Point(-10, -10));


            myPolyline.Points = myPointCollection2;

            own_pos.Shape = myPolyline;



            own_pos.Shape.RenderTransform = new RotateTransform(direction + 45);





            home_pos.Shape = new System.Windows.Shapes.Rectangle
            {
                Width = 20,
                Height = 20,

                Stroke = System.Windows.Media.Brushes.Black,
                StrokeThickness = 4
            };

            mapView.Markers.Add(own_pos);
            mapView.Markers.Add(home_pos);





        }

        public void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            while (sp.BytesToRead > 0)
            {
                char temp = Convert.ToChar(sp.ReadByte());

                if (temp == 'T')
                {
                    try
                    {
                        string data = sp.ReadTo("E");
                        var pos_0 = data.Split('L');
                        var pos_1 = pos_0[1].Split('S');
                        var pos_2 = pos_1[1].Split('H');
                        var speed = pos_2[0];
                        var head = pos_2[1];

                        telemetry.tel_update(Convert.ToDouble(pos_0[0]), Convert.ToDouble(pos_1[0]), Convert.ToDouble(speed), "", "", Convert.ToDouble(head));

                    }
                    catch
                    {
                        telemetry.tel_update(0, 0, 0, "data corrupted");


                    }
                    sp.DiscardInBuffer();


                }

                if (temp == 'H')
                {
                    try
                    {
                        string data = sp.ReadTo("E");
                        var pos_0 = data.Split('L');
                        var pos_1 = pos_0[0];
                        var pos_2 = pos_0[1];

                        telemetry.home_set(Convert.ToDouble(pos_1), Convert.ToDouble(pos_2));

                    }
                    catch
                    {
                        telemetry.tel_update(0, 0, 0, "data corrupted");


                    }
                    sp.DiscardInBuffer();


                }
                else
                {
                    
                    telemetry.debug_text += sp.ReadExisting();
                    sp.DiscardInBuffer();


                }



            }



        }

        private void Button_Click_5(object sender, RoutedEventArgs e)
        {
            if (Port_Liste.Items.Count > 0)
            {
                var index = Port_Liste.SelectedIndex;
                Port_selected = Serial_ports[index];
                try {
                    Port_selected.Open();
                    Port_selected.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                    Connect_button.Background = System.Windows.Media.Brushes.BlueViolet;
                    Connect_button.Content = "Port Connected";
                    Port_selected.DiscardInBuffer();
                }
                catch
                {
                    Port_selected.Close();
                    Connect_button.Background = System.Windows.Media.Brushes.MediumVioletRed;
                    Connect_button.Content = "Connection failed";

                }
            }
        }

        private void Course_SP_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                commands["Autopilot"] = false;
                commands["Home"] = false;
                commands["Manuell"] = false;
                commands["Course"] = true;
                
            }
        }
    }
}
