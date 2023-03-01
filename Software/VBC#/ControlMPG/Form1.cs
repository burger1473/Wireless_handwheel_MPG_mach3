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

namespace ControlMPG
{
    public partial class Form1 : Form
    {
        //Defgino variable connect Mach3
        private IMach4 _mach;
        private IMyScriptObject _mInst;

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
            
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            UpdateDro();
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
                //_mInst.DoOEMButton(310); //Genera un pulso en eje y ++
                //_mInst.DoOEMButton(311); //Genera un pulso en eje z ++
                //_mInst.DoOEMButton(312); //Genera un pulso en eje z ++
                //_mInst.DoOEMButton(313); //Genera un pulso en eje a ++
                //_mInst.DoOEMButton(314); //Genera un pulso en eje a ++
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

        }
    }
}
