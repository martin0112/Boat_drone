using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;
using GMap.NET;

namespace Boat_New
{



    public partial class Form1 : Form 
    {

        public GMapOverlay markers_own_position = new GMapOverlay("eigene position");



        public Form1()
        {
            InitializeComponent();
            
           
        }

        
        private void Form1_Load(object sender, EventArgs e)
        {
            //GPS Karte
            Methoden.GPS_stuff.karte_initialisieren(gMapControl1);
            


            //Listbox mit com 
            listBox2.DataSource = Methoden.Serielles.Serielle_ports;
            listBox2.DisplayMember="PortName";
            listBox2.ClearSelected();


            //Kamera
            Methoden.Camera.create_camera(pictureBox1);

            //Keyboard
            this.KeyPreview = true;
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            
        }

        public void AddDataMethod(String myString)
        {
            textBox2.AppendText(myString);
        }

        private void button1_Click(object sender, EventArgs e)
        {


            if (Methoden.Serielles.selected == true && Methoden.Serielles.selected_port.IsOpen==false)
            {
                    Methoden.Serielles.selected_port = (SerialPort)listBox2.SelectedItem;
                    

                    Methoden.Serielles.selected_port.Open();
                
                    if (Methoden.Serielles.selected_port.IsOpen)
                        button1.BackColor = System.Drawing.Color.PowderBlue;





            }

            else
                
            {
                Methoden.Serielles.selected_port.Close();
                listBox2.SelectedItems.Clear();
                Methoden.Serielles.selected = false;
                button1.BackColor = System.Drawing.Color.Red;
                button1.Enabled = false;



            }



            
        }

        private void listBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
       


        }

    




        private void listBox2_MouseClick(object sender, MouseEventArgs e)
        {
            if (listBox2.SelectedItems.Count>0)
            {
            Methoden.Serielles.selected=true;
            button1.BackColor = System.Drawing.Color.White;
            button1.Enabled = true;
            
            }

        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (Methoden.cam!=null)
            Methoden.cam.Stop();
            Application.Exit();
        }

     

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            //MessageBox.Show("Form.KeyPress");
            Methoden.kontrolle.check_key(textBox1);
      

        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {

                if (markers_own_position.Markers.Count > 0)
                {
                    try
                    {
                        gMapControl1.Invoke(new Action(() => gMapControl1.Position = markers_own_position.Markers[0].Position));
                    }
                    catch
                    { }
                }
            }
            catch
            { }

        }
    }
}
