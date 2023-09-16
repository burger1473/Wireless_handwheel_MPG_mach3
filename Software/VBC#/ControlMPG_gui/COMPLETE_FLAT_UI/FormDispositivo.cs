using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;


namespace COMPLETE_FLAT_UI
{
    public partial class FormDispositivo : Form
    {
        public FormDispositivo()
        {
            InitializeComponent();
        }

        private void BtnCerrar_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void FormDispositivo_Load(object sender, EventArgs e)
        {
            string path = Directory.GetCurrentDirectory() + "/config.conf";
            string[] lines = System.IO.File.ReadAllLines(@"" + path);
            txtid.Text = lines[0];
            comboBox1.Text = lines[1];
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string path = Directory.GetCurrentDirectory() + "/config.conf";
            string[] lines = { txtid.Text, comboBox1.Text };
            System.IO.File.WriteAllLines(path, lines);
            Application.Restart();
        }
    }
}
