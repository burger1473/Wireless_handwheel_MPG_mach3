using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;   //proporciona servicios de interoperabilidad para la comunicación entre componentes de software escritos en lenguajes de programación diferentes. Esto significa que puede usar el espacio de nombres para comunicarse con componentes de software escritos en lenguajes como C#, Visual Basic y C++. Esto se logra mediante la creación de una interfaz común entre los componentes de software, lo que permite que los componentes de software escritos en diferentes lenguajes de programación se comuniquen entre sí.
using Mach4;                            //DLL Interop.Mach4.dll  creado al referir el icono de Mach3. Antes se debia crear el registro.
using System.Net;
using System.Net.Sockets;
using System.Text;


namespace ControlMPG
{
    public partial class Form1 : Form
    {
        //Defgino variable connect Mach3
        private IMach4 _mach;
        private IMyScriptObject _mInst;
        //string ip = "192.168.4.1";
        //string ip = "192.168.1.113";
        string ip;
        int puerto_server1 = 8001;
        private Socket clientSocket;
        private byte[] buffer;
        bool tcp_conected = false;

        public Form1()
        {
            InitializeComponent();
        }

        private void GetMachInstance()
        {
            try
            {
                _mach = (IMach4) Marshal.GetActiveObject("Mach4.Document");
                _mInst = (IMyScriptObject)_mach.GetScriptDispatch();
            }
            catch {
                _mach = null;
                _mInst = null;
            }
        }

        private void UpdateDro()
        {
            GetMachInstance();
            if (_mInst != null)
            {
                textBox1.Text = _mInst.GetOEMDRO(Convert.ToInt16(textBox2.Text)).ToString();
                //_mInst.GetOEMDRO(814).ToString();   //elapsed time
                //_mInst.GetOEMDRO(815).ToString();   //estimated time
                //_mInst.GetOEMDRO(800).ToString();   //Axis x value
                //_mInst.GetOEMDRO(801).ToString();   //Axis y value
                //_mInst.GetOEMDRO(802).ToString();   //Axis z value
                //_mInst.GetOEMDRO(803).ToString();   //Axis a value
                //_mInst.GetOEMDRO(816).ToString();   //Numero de linea actual

            }

            //_mInst.LoadFile("C:/Users/fabia/OneDrive/Desktop/gcode.gcode");
            // SetDRO(801, 20.1);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            IPAddress[] IpInHostAddress = Dns.GetHostAddresses("MPG000001.local");
            ip = IpInHostAddress[0].ToString();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            UpdateDro();
            textBox1.Text = tcp_conected.ToString();
            richTextBox1.Text = Conexion_tcp_automatica.Enabled.ToString();
            try
            {
                if (clientSocket.Connected==false) {
                    tcp_conected=false;
                }
            }
            catch (Exception) { 
            }
        }

        private void Load_Gcode_Click(object sender, EventArgs e)
        {
            GetMachInstance();
            if (_mInst != null)
            {
                //_mInst.LoadFile("C:/Users/fabia/OneDrive/Desktop/gcode.gcode");

                //_mInst.DoOEMButton(307); //Genera un pulso en eje x ++
                //_mInst.DoOEMButton(308); //Genera un pulso en eje x --
                //_mInst.DoOEMButton(309); //Genera un pulso en eje y ++
                //_mInst.DoOEMButton(310); //Genera un pulso en eje y --
                //_mInst.DoOEMButton(311); //Genera un pulso en eje z ++
                //_mInst.DoOEMButton(312); //Genera un pulso en eje z --
                //_mInst.DoOEMButton(313); //Genera un pulso en eje a ++
                //_mInst.DoOEMButton(314); //Genera un pulso en eje a --
                //_mInst.DoOEMButton(1007); //Zero all
                //_mInst.DoOEMButton(1008); //Zero x
                //_mInst.DoOEMButton(1009); //Zero y
                //_mInst.DoOEMButton(1010); //Zero z
                //_mInst.DoOEMButton(1011); //Zero a
                //_mInst.DoOEMButton(1001); //Pause (Feed Hold)
                //_mInst.DoOEMButton(1000); //Cycle start
                //_mInst.DoOEMButton(1002); //Rewind
                //_mInst.DoOEMButton(1003); //Stop
                //_mInst.DoOEMButton(1021); //Reset
                //_mInst.DoOEMButton(1017); //Goto zero
                _mInst.DoOEMButton(275);//Set Jog mode STEP (same as button 205)
                _mInst.DoOEMButton(305); //Set MPG jogging mode = do single eaxct steps

                _mInst.SetOEMDRO(828, 0.01); //Set cycle jog step to 0.01
                for (int i=0; i<124; i++)
                {
                    _mInst.DoOEMButton(307); //Genera un pulso en eje x ++
                }

            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            enviar_tcp("HOLAfabi");
        }

    
        void desconectar()
        {
            try
            {
                if (clientSocket.Connected == true)
                {
                    tcp_conected = false;
                    Conexion_tcp_automatica.Enabled = true;
                    //clientSocket.Disconnect(true);
                    clientSocket.Close();
                }
            }
            catch (Exception)
            {

            }
        }

        private void enviar_tcp(string mensaje)
        {
            byte[] enviar_info = new byte[100];
            enviar_info = Encoding.Default.GetBytes(mensaje);
            try
            {
                clientSocket.BeginSend(enviar_info, 0, enviar_info.Length, SocketFlags.None, SendCallback, null);
            }
            catch (SocketException ex)
            {

            }
            catch (ObjectDisposedException ex)
            {

            }
            catch (Exception)
            {

            }
        }
        private static void ShowErrorDialog(string message)
        {
            MessageBox.Show(message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void ReceiveCallback(IAsyncResult AR)
        {
            try
            {
                int received = clientSocket.EndReceive(AR);

                string message = Encoding.ASCII.GetString(buffer);
                textBox1.Text = message;
                richTextBox1.Text = richTextBox1.Text + "/n" + message;

                // Comienzo a recibir datos nuevamente.
                clientSocket.BeginReceive(buffer, 0, 2, SocketFlags.None, ReceiveCallback, null);
            }
            catch (SocketException ex)
            {
                desconectar();
            }
            catch (ObjectDisposedException ex)
            {

            }
            catch (Exception)
            {

            }

        }


        private void ConnectCallback(IAsyncResult AR)
        {
            try
            {
                //clientSocket = (Socket)AR.AsyncState;
                clientSocket.EndConnect(AR);
                buffer = new byte[clientSocket.ReceiveBufferSize];
                clientSocket.BeginReceive(buffer, 0, 2, SocketFlags.None, ReceiveCallback, null);

            }
            catch (SocketException ex)
            {
                desconectar();
            }
            catch (ObjectDisposedException ex)
            {
            
            }
            catch (Exception)
            {
            }
        }

        private void SendCallback(IAsyncResult AR)
        {
            try
            {
                clientSocket.EndSend(AR);
            }
            catch (SocketException ex)
            {  
                desconectar();
            }
            catch (ObjectDisposedException ex)
            {
            }
            catch (Exception)
            {
            }
        }


    

        private void button2_Click(object sender, EventArgs e)
        {
            
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
           if (tcp_conected == true) {
                try
                {
                        clientSocket.Disconnect(true);
                        clientSocket.Close();
                }
                catch (Exception)
                {

                }
            }
        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            if (tcp_conected == false)
            {
                try
                {
                    
                    clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                   
                    // Connect to the specified host.
                    if (clientSocket.Connected == false)
                    {
                        var endPoint = new IPEndPoint(IPAddress.Parse(ip), puerto_server1);
                        clientSocket.BeginConnect(endPoint, new AsyncCallback(ConnectCallback), null);
                        tcp_conected = true;
                        byte[] enviar_info = new byte[100];
                        enviar_info = Encoding.Default.GetBytes("N04Conected*****************************************F");
                        clientSocket.BeginSend(enviar_info, 0, enviar_info.Length, SocketFlags.None, SendCallback, null);
                        //Conexion_tcp_automatica.Enabled = false;
                    }
                }
                catch (SocketException ex)
                {

                }
                catch (ObjectDisposedException ex)
                {

                }
                catch (Exception)
                {

                }
            }
        }

        private void timer_tcpvivo_Tick(object sender, EventArgs e)
        {
            enviar_tcp("N05Vivo0000000**************************************F");
        }
    }
}
