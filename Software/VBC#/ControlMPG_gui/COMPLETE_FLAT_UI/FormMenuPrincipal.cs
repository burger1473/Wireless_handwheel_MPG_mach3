using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;   //proporciona servicios de interoperabilidad para la comunicación entre componentes de software escritos en lenguajes de programación diferentes. Esto significa que puede usar el espacio de nombres para comunicarse con componentes de software escritos en lenguajes como C#, Visual Basic y C++. Esto se logra mediante la creación de una interfaz común entre los componentes de software, lo que permite que los componentes de software escritos en diferentes lenguajes de programación se comuniquen entre sí.
using Mach4;                            //DLL Interop.Mach4.dll  creado al referir el icono de Mach3. Antes se debia crear el registro.
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Globalization;


namespace COMPLETE_FLAT_UI
{
    public partial class FormMenuPrincipal : Form
    {
        //Constructor
        public FormMenuPrincipal()
        {
            InitializeComponent();
            //Estas lineas eliminan los parpadeos del formulario o controles en la interfaz grafica (Pero no en un 100%)
            this.SetStyle(ControlStyles.ResizeRedraw, true);
            this.DoubleBuffered = true;
        }

        //METODO PARA REDIMENCIONAR/CAMBIAR TAMAÑO A FORMULARIO  TIEMPO DE EJECUCION ----------------------------------------------------------
        private int tolerance = 15;
        private const int WM_NCHITTEST = 132;
        private const int HTBOTTOMRIGHT = 17;
        private Rectangle sizeGripRectangle;

        protected override void WndProc(ref Message m)
        {
            switch (m.Msg)
            {
                case WM_NCHITTEST:
                    base.WndProc(ref m);
                    var hitPoint = this.PointToClient(new Point(m.LParam.ToInt32() & 0xffff, m.LParam.ToInt32() >> 16));
                    if (sizeGripRectangle.Contains(hitPoint))
                        m.Result = new IntPtr(HTBOTTOMRIGHT);
                    break;
                default:
                    base.WndProc(ref m);
                    break;
            }
        }
        //----------------DIBUJAR RECTANGULO / EXCLUIR ESQUINA PANEL 
        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            var region = new Region(new Rectangle(0, 0, this.ClientRectangle.Width, this.ClientRectangle.Height));

            sizeGripRectangle = new Rectangle(this.ClientRectangle.Width - tolerance, this.ClientRectangle.Height - tolerance, tolerance, tolerance);

            region.Exclude(sizeGripRectangle);
            this.panelContenedorPrincipal.Region = region;
            this.Invalidate();
        }

        //----------------COLOR Y GRIP DE RECTANGULO INFERIOR
        protected override void OnPaint(PaintEventArgs e)
        {

            SolidBrush blueBrush = new SolidBrush(Color.FromArgb(55, 61, 69));
            e.Graphics.FillRectangle(blueBrush, sizeGripRectangle);

            base.OnPaint(e);
            ControlPaint.DrawSizeGrip(e.Graphics, Color.Transparent, sizeGripRectangle);
        }
       
        //METODO PARA ARRASTRAR EL FORMULARIO---------------------------------------------------------------------
        [DllImport("user32.DLL", EntryPoint = "ReleaseCapture")]
        private extern static void ReleaseCapture();

        [DllImport("user32.DLL", EntryPoint = "SendMessage")]
        private extern static void SendMessage(System.IntPtr hWnd, int wMsg, int wParam, int lParam);

        private void PanelBarraTitulo_MouseDown(object sender, MouseEventArgs e)
        {
            ReleaseCapture();
            SendMessage(this.Handle, 0x112, 0xf012, 0);
        }

        //METODOS PARA CERRAR,MAXIMIZAR, MINIMIZAR FORMULARIO------------------------------------------------------
        int lx, ly;
        int sw, sh;
        private void btnMaximizar_Click(object sender, EventArgs e)
        {
            lx = this.Location.X;
            ly = this.Location.Y;
            sw = this.Size.Width;
            sh = this.Size.Height;
            this.Size = Screen.PrimaryScreen.WorkingArea.Size;
            this.Location = Screen.PrimaryScreen.WorkingArea.Location;
            btnMaximizar.Visible = false;
            btnNormal.Visible = true;

        }

        private void btnNormal_Click(object sender, EventArgs e)
        {
            this.Size = new Size(sw, sh);
            this.Location = new Point(lx, ly);
            btnNormal.Visible = false;
            btnMaximizar.Visible = true;
        }

        private void btnMinimizar_Click(object sender, EventArgs e)
        {
            this.WindowState = FormWindowState.Minimized;
        }

        private void btnCerrar_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("¿Está seguro de cerrar?", "Alerta¡¡", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                if (tcp_conected == true)
                {
                    try
                    {
                       // clientSocket.Disconnect(true);
                        clientSocket.Close();
                    }
                    catch (Exception)
                    {

                    }
                }
                Application.Exit();
            }
        }

        private void btnSalir_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("¿Está seguro de cerrar?", "Alerta¡¡", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                Application.Exit();
            }
        }

        //METODOS PARA ANIMACION DE MENU SLIDING--
        private void btnMenu_Click(object sender, EventArgs e)
        {
            //-------CON EFECTO SLIDING
            if (panelMenu.Width == 230)
            {
                this.tmContraerMenu.Start();
            }
            else if (panelMenu.Width == 55)
            {
                this.tmExpandirMenu.Start();
            }

            //-------SIN EFECTO 
            //if (panelMenu.Width == 55)
            //{
            //    panelMenu.Width = 230;
            //}
            //else

            //    panelMenu.Width = 55;
        }

        private void tmExpandirMenu_Tick(object sender, EventArgs e)
        {
            if (panelMenu.Width >= 230)
                this.tmExpandirMenu.Stop();
            else
                panelMenu.Width = panelMenu.Width + 5;
            
        }

        private void tmContraerMenu_Tick(object sender, EventArgs e)
        {
            if (panelMenu.Width <= 55)
                this.tmContraerMenu.Stop();
            else
                panelMenu.Width = panelMenu.Width - 5;
        }

        //METODO PARA ABRIR FORM DENTRO DE PANEL-----------------------------------------------------
        private void AbrirFormEnPanel(object formHijo)
        {
            if (this.panelContenedorForm.Controls.Count > 0)
                this.panelContenedorForm.Controls.RemoveAt(0);
            Form fh = formHijo as Form;
            fh.TopLevel = false;
            fh.FormBorderStyle = FormBorderStyle.None;
            fh.Dock = DockStyle.Fill;            
            this.panelContenedorForm.Controls.Add(fh);
            this.panelContenedorForm.Tag = fh;
            fh.Show();
        }

        //METODO PARA MOSTRAR FORMULARIO DE LOGO Al INICIAR ----------------------------------------------------------
        private void MostrarFormLogo()
        {
            AbrirFormEnPanel(new FormLogo());
        }

        private void FormMenuPrincipal_Load(object sender, EventArgs e)
        {
            string path = Directory.GetCurrentDirectory() + "/config.conf";
            string[] lines = System.IO.File.ReadAllLines(@"" + path);

            if (lines[1]=="SI")
            {
                ip = "192.168.4.1";
            }
            else
            {
                try
                {
                    IPAddress[] IpInHostAddress = Dns.GetHostAddresses(lines[0] + ".local");
                    ip = IpInHostAddress[0].ToString();
                }
                catch (Exception)
                {
                    ip = "192.168.4.1";
                }
                
            }
            MostrarFormLogo();
        }

        //METODO PARA MOSTRAR FORMULARIO DE LOGO Al CERRAR OTROS FORM ----------------------------------------------------------
        private void MostrarFormLogoAlCerrarForms(object sender, FormClosedEventArgs e)
        {
            MostrarFormLogo();
        }


        //METODOS PARA ABRIR OTROS FORMULARIOS Y MOSTRAR FORM DE LOGO Al CERRAR ----------------------------------------------------------

        private void button6_Click(object sender, EventArgs e)
        {
            FormDispositivo fm = new FormDispositivo();
            fm.FormClosed += new FormClosedEventHandler(MostrarFormLogoAlCerrarForms);
            AbrirFormEnPanel(fm);

        }

        private void button5_Click(object sender, EventArgs e)
        {
            //AbrirFormEnPanel(new Form1());
            FormUsuario fm = new FormUsuario();
            fm.FormClosed += new FormClosedEventHandler(MostrarFormLogoAlCerrarForms);
            AbrirFormEnPanel(fm);

        }

        private void button3_Click(object sender, EventArgs e)
        {
            FormListaTareas fm = new FormListaTareas();
            fm.FormClosed += new FormClosedEventHandler(MostrarFormLogoAlCerrarForms);
            AbrirFormEnPanel(fm);
        }

        private void button1_Click(object sender, EventArgs e)
        {

        }


        //METODO PARA HORA Y FECHA ACTUAL ----------------------------------------------------------
        private void tmFechaHora_Tick(object sender, EventArgs e)
        {
            lbFecha.Text = DateTime.Now.ToLongDateString();
            lblHora.Text = DateTime.Now.ToString("HH:mm:ssss");
        }
        



        //APLICACION MPG


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
        int msg_tcp_enviando = 0;
        String M51 = "N***************************************************F";
        String M52 = "N***************************************************F";
        int lineas_agregadas = 0;
        string path = Directory.GetCurrentDirectory() + "/tempgcode.gcode";

        private void GetMachInstance()
        {
            try
            {
                _mach = (IMach4)Marshal.GetActiveObject("Mach4.Document");
                _mInst = (IMyScriptObject)_mach.GetScriptDispatch();
            }
            catch
            {
                _mach = null;
                _mInst = null;
            }
        }

        private void UpdateDro()
        {
            GetMachInstance();
            if (_mInst != null)
            {
                //String elapsed_time = _mInst.GetOEMDRO(814).ToString();
                String elapsed_time = _mInst.GetDRO(14).ToString();
                String estimated_time = _mInst.GetOEMDRO(815).ToString();
                String x = _mInst.GetOEMDRO(800).ToString();
                String y = _mInst.GetOEMDRO(801).ToString();
                String z = _mInst.GetOEMDRO(802).ToString();
                String a = _mInst.GetOEMDRO(803).ToString();
                String numero_linea_actual = _mInst.GetOEMDRO(816).ToString();

                
                String signox, signoy, signoz, signoa;
                if (float.Parse(x) >= 0) { signox = "+"; } else { signox = "-"; }
                if (float.Parse(y) >= 0) { signoy = "+"; } else { signoy = "-"; }
                if (float.Parse(z) >= 0) { signoz = "+"; } else { signoz = "-"; }
                if (float.Parse(a) >= 0) { signoa = "+"; } else { signoa = "-"; }

                x = string.Format("{0:00000.0000}", float.Parse(x, CultureInfo.InvariantCulture.NumberFormat));
                y = string.Format("{0:00000.0000}", float.Parse(y, CultureInfo.InvariantCulture.NumberFormat));
                z = string.Format("{0:00000.0000}", float.Parse(z, CultureInfo.InvariantCulture.NumberFormat));
                a = string.Format("{0:00000.0000}", float.Parse(a, CultureInfo.InvariantCulture.NumberFormat));
                numero_linea_actual= string.Format("{0:0000}", int.Parse(numero_linea_actual));


                M51 = "N51E"+ signox+ x +"x"+ signoy + y + "y"+ signoz + z + "z"+ signoa + a + "aF";
                //String M01 = "N01E+00000.0000x+00000.0000y+00000.0000z+00000.0000aF";
                //label2.Text = numero_linea_actual + " * " + elapsed_time + " * " + estimated_time;
                M52 = "N52EL"+ elapsed_time + "ES"+ estimated_time + "NL0000LA"+ numero_linea_actual + "***********************F";
                //String M02 = "N02EL00:00ES00:00NL0000LA0000***********************F";
            }
        }


        private void Update_pro_Tick(object sender, EventArgs e)
        {
            UpdateDro();
            try
            {
                if (clientSocket != null)
                {
                    if (clientSocket.Connected == false)
                    {
                        tcp_conected = false;
                    }
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
            if (tcp_conected==true)
            {
                button2.Text = "  MPG Conectado";
                button2.ForeColor = Color.Green;
            }
            else
            {
                button2.Text = "  MPG Desconectado";
                button2.ForeColor = Color.Red;
            }
        }

        //Server tcp/ip
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

        private void enviar_tcp(string mensaje)
        {
            byte[] enviar_info = new byte[100];
            enviar_info = Encoding.Default.GetBytes(mensaje);
            try
            {
                if (tcp_conected == true)
                {
                    if (clientSocket != null)
                    {
                        if (msg_tcp_enviando == 0)
                        {
                            msg_tcp_enviando = 1;
                            clientSocket.BeginSend(enviar_info, 0, enviar_info.Length, SocketFlags.None, SendCallback, null);
                        }
                        
                    }

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
        private static void ShowErrorDialog(string message)
        {
            MessageBox.Show(message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        void mach3_send_OEMDRO(int OEMDRO)
        {
            GetMachInstance();
            if (_mInst != null)
            {
                _mInst.DoOEMButton(Convert.ToInt16(OEMDRO));

            }
        }
        void mach3_SetOEMDRO(int OEMDRO, float valor)
        {
            GetMachInstance();
            if (_mInst != null)
            {
                _mInst.SetOEMDRO(Convert.ToInt16(OEMDRO), valor);
            }
        }

        void mach3_Loadgcodefile(String ruta)
        {
            GetMachInstance();
            if (_mInst != null)
            {
                _mInst.LoadFile(ruta);
            }
            
        }

        void mach3_Sencode(String code)
        {
            GetMachInstance();
            if (_mInst != null)
            {
                _mInst.Code(code);
            }
        }

        private void ReceiveCallback(IAsyncResult AR)
        {
            try
            {
                int received = clientSocket.EndReceive(AR);

                string message = Encoding.ASCII.GetString(buffer);

                char[] chars = message.ToCharArray();
                if (message.Length == 53)
                {
                    if (chars[0] == 'N' && chars[52] == 'F')                             //Si el mensaje es correcto
                    {                                                                  //N01S0.0001X+1000************************************F
                        if (message == "N0451*********************************************51F") { enviar_tcp(M51); }
                        if (message == "N0452*********************************************52F") { enviar_tcp(M52); }
                        if (message == "N02Zero*****************************************ZeroF") { mach3_send_OEMDRO(1007); } //Zero all
                        if (message == "N02ZeroX***************************************ZeroXF") { mach3_send_OEMDRO(1008); } //Zero x
                        if (message == "N02ZeroY***************************************ZeroYF") { mach3_send_OEMDRO(1009); } //Zero y
                        if (message == "N02ZeroZ***************************************ZeroZF") { mach3_send_OEMDRO(1010); } //Zero z
                        if (message == "N02ZeroA***************************************ZeroAF") { mach3_send_OEMDRO(1011); } //Zero a
                        if (message == "N02Reset***************************************ResetF") { mach3_send_OEMDRO(1021); } //Reset
                        if (message == "N02Stop*****************************************StopF") { mach3_send_OEMDRO(1003); } //Stop
                        if (message == "N02Rewind*************************************RewindF") { mach3_send_OEMDRO(1002); } //Rewind
                        if (message == "N02Start***************************************StartF") { mach3_send_OEMDRO(1000); } //Cycle start
                        if (message == "N02Pause***************************************PauseF") { mach3_send_OEMDRO(1001); } //Pause (Feed Hold)
                        if (message == "N02GOTOZ***************************************GOTOZF") { mach3_send_OEMDRO(1017); } //Goto zero

                        if (chars[1] == '0' && chars[2] == '1' && chars[3] == 'S')    //Mensaje de movimiento de eje
                        {
                            string res = ""; res += chars[4]; res += chars[5]; res += chars[6]; res += chars[7]; res += chars[8]; res += chars[9];
                            string step = ""; step += chars[12]; step += chars[13]; step += chars[14]; step += chars[15];

                            float pasos = Int16.Parse(step) * float.Parse(res, CultureInfo.InvariantCulture.NumberFormat);

                            String gcode_tosend = "G91 " + chars[10].ToString() + chars[11].ToString() + pasos.ToString();
                            label2.Text = gcode_tosend;
                            mach3_Sencode(gcode_tosend);

                            /*
                            mach3_send_OEMDRO(275);  //Set Jog mode STEP (same as button 205)
                            mach3_send_OEMDRO(305);  //Set MPG jogging mode = do single eaxct steps

                            mach3_SetOEMDRO(828, float.Parse(res, CultureInfo.InvariantCulture.NumberFormat)); //Set cycle jog step to 0.01

                            int pasos = Int16.Parse(step);
                            label2.Text = pasos.ToString();
                            */
                            //if (chars[10] == 'X' && chars[11] == '+') { for (int i = 0; i < pasos; i++) { mach3_send_OEMDRO(307); Thread.Sleep(100); /*Genera un pulso en eje X ++*/ } }
                            //if (chars[10] == 'X' && chars[11] == '-') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(308); Thread.Sleep(1); /*Genera un pulso en eje X --*/ } }
                            //if (chars[10] == 'Y' && chars[11] == '+') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(309); Thread.Sleep(1); /*Genera un pulso en eje Y ++*/ } }
                            //if (chars[10] == 'Y' && chars[11] == '-') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(310); Thread.Sleep(1); /*Genera un pulso en eje Y --*/ } }
                            //if (chars[10] == 'Z' && chars[11] == '+') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(311); Thread.Sleep(1); /*Genera un pulso en eje Z ++*/ } }
                            //if (chars[10] == 'Z' && chars[11] == '-') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(312); Thread.Sleep(1); /*Genera un pulso en eje Z --*/ } }
                            //if (chars[10] == 'A' && chars[11] == '+') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(313); Thread.Sleep(1); /*Genera un pulso en eje A ++*/ } }
                            //if (chars[10] == 'A' && chars[11] == '-') { for (int i = 0; i < Int16.Parse(step); i++) { mach3_send_OEMDRO(314); Thread.Sleep(1); /*Genera un pulso en eje A -*/ } }

                        }

                        if (chars[1] == '0' && chars[2] == '3')    //Mensaje GCOE
                        {
                            string text_codigo = message.Substring(8, 39);
                            string text_codigo_sin_asterisco = text_codigo.Replace('*', ' ');
                            int linea_Actual = Int16.Parse(chars[3].ToString() + chars[4].ToString() + chars[5].ToString() + chars[6].ToString());
                            try
                            {
                                if (chars[3] == chars[48] && chars[4] == chars[49] && chars[5] == chars[50] && chars[6] == chars[51])                                  //Si es la ultima linea
                                {
                                    File.AppendAllText(path, text_codigo_sin_asterisco);
                                    lineas_agregadas++;

                                    if (linea_Actual == lineas_agregadas)  //en este cado linea_Actual es el valor final
                                    {
                                        lineas_agregadas = 0;
                                        mach3_Loadgcodefile(path);
                                        enviar_tcp("N53Confirm:Gcode-load*******************************F");
                                        //label2.Text = "Fin gcode";
                                    }
                                    else
                                    {
                                        //label2.Text = "Numeros de lineas diferentes";
                                    }
                                }
                                else                                   //Sino
                                {
                                    if (chars[3] == '0' && chars[4] == '0' && chars[5] == '0' && chars[6] == '1')  //Si es el inicio del codigo, primero borro el archivo completo 
                                    {
                                        lineas_agregadas = 0;
                                        // Create a file to write to.
                                        try
                                        {
                                            path = Directory.GetCurrentDirectory() + "/tempgcode.gcode";
                                            using (StreamWriter sw = File.CreateText(path))
                                            {
                                                sw.WriteLine(text_codigo_sin_asterisco);
                                            }
                                        }
                                        catch
                                        {
                                            path = Directory.GetCurrentDirectory() + "/tempgcode2.gcode";
                                            using (StreamWriter sw = File.CreateText(path))
                                            {
                                                sw.WriteLine(text_codigo_sin_asterisco);
                                            }
                                        }

                                        //File.AppendAllText(path, text_codigo_sin_asterisco);
                                        lineas_agregadas++;
                                    }
                                    else
                                    {
                                        if (linea_Actual == (lineas_agregadas + 1))
                                        {
                                            File.AppendAllText(path, text_codigo_sin_asterisco + "\n");
                                            lineas_agregadas++;
                                        }

                                    }
                                }
                            }
                            catch (Exception)
                            {

                            }

                        }

                    }
                    else
                    {
                        //label2.Text = "error";
                    }
                }
                else
                {
                    //label2.Text = "error";
                }


                //_mInst.LoadFile("C:/Users/fabia/OneDrive/Desktop/gcode.gcode");
                // SetDRO(801, 20.1);


                // Comienzo a recibir datos nuevamente.
                clientSocket.BeginReceive(buffer, 0, 53, SocketFlags.None, ReceiveCallback, null);
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

        void Conexion_tcp_automatica_Tick(object sender, EventArgs e)
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

        private void PanelBarraTitulo_Paint(object sender, PaintEventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            enviar_tcp("N05Vivo0000000**************************************F");
        }

        private void pictureBox7_Click(object sender, EventArgs e)
        {
            
        }

        private void button4_Click(object sender, EventArgs e)
        {
        }

        private void ConnectCallback(IAsyncResult AR)
        {
            try
            {
                //clientSocket = (Socket)AR.AsyncState;
                clientSocket.EndConnect(AR);
                buffer = new byte[clientSocket.ReceiveBufferSize];
                clientSocket.BeginReceive(buffer, 0, 53, SocketFlags.None, ReceiveCallback, null);

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
            msg_tcp_enviando = 0;
        }

    }
}
