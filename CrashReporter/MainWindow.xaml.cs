//  Vista Metadata Project
//  Copyright (C) 2007-2008  Eric Schultz

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; specifically version 2 of the License.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Interop; // WindowInteropHelper
using System.Runtime.InteropServices; // DllImportAttribute
using ICSharpCode.SharpZipLib;
using ICSharpCode.SharpZipLib.Tar;
using ICSharpCode.SharpZipLib.GZip;
using System.IO;
using System.Xml;
using ICSharpCode.SharpZipLib.Core;
using System.Diagnostics;
using Microsoft.Win32;


namespace CrashReporter
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private string miniDumpId;
        private string productName;
        private string dumpDir;
        private string version;
        private sendEnum sendChoice;
        string tarDest;
        string xmlDest;

        public MainWindow(string dumpDir, string miniDumpId, string productName, string version, sendEnum sendChoice)
        {
            this.miniDumpId = miniDumpId;
            this.productName = productName;
            this.dumpDir = dumpDir;
            this.sendChoice = sendChoice;
            this.version = version;
            if (this.sendChoice == sendEnum.SendAutomatically)
            {
                this.sendError();
                /*
                Process explorer = new Process();
                explorer.StartInfo.FileName = "explorer.exe";
                explorer.Start();
                */
                App.Current.Shutdown();
            }

            InitializeComponent();
            
        }
        
        [DllImport("user32.dll")]
        static extern int GetWindowLong(IntPtr hwnd, int index);

        [DllImport("user32.dll")]
        static extern int SetWindowLong(IntPtr hwnd, int index, int newStyle);

        [DllImport("user32.dll")]
        static extern bool SetWindowPos(IntPtr hwnd, IntPtr hwndInsertAfter, int x, int y, int width, int height, uint flags);

        [DllImport("user32.dll")]
        static extern IntPtr SendMessage(IntPtr hwnd, uint msg, IntPtr wParam, IntPtr lParam);

        const int GWL_EXSTYLE = -20;
        const int WS_EX_DLGMODALFRAME = 0x0001;

        const int SWP_NOSIZE = 0x0001;
        const int SWP_NOMOVE = 0x0002;
        const int SWP_NOZORDER = 0x0004;
        const int SWP_FRAMECHANGED = 0x0020;

        const uint WM_SETICON = 0x0080;
               
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            // Get this window's handle
            IntPtr hwnd = new WindowInteropHelper(this).Handle;
 
            // Change the extended window style to not show a window icon
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_DLGMODALFRAME);
 
            // Update the window's non-client area to reflect the changes
            SetWindowPos(hwnd, IntPtr.Zero, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        private bool CreateReport()
        {
            xmlDest = System.IO.Path.GetTempPath() + "\\" + miniDumpId + ".xml";
            tarDest = System.IO.Path.GetTempPath() + "\\" + miniDumpId + ".tar";
            
            XmlCreation();
            TarCreation();
            GZipCreation();
            SendReport(tarDest + ".gz");
            return true;
            

        }

        private void SendReport(string filename)
        {
          
            System.Net.WebClient wc = new System.Net.WebClient();
            try
            {
                byte[] body = wc.UploadFile("http://vistaprophand.sourceforge.net/errorreport.php", filename);
                string s = Encoding.ASCII.GetString(body);
            }
            catch (Exception e){ }
        }

        private void GZipCreation()
        {
            byte[] dataBuffer = new byte[4096];

            Stream s = new GZipOutputStream(File.Create(tarDest + ".gz"));
            ((GZipOutputStream)s).SetLevel(9);
            using (FileStream fs = File.OpenRead(tarDest))
            {
                StreamUtils.Copy(fs, s, dataBuffer);
            }
            s.Close();
        }

        private void XmlCreation()
        {
            
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<errorreport></errorreport>");

            XmlElement elem = doc.CreateElement("ProductName");
            elem.InnerText = productName;
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("GUID");
            elem.InnerText = miniDumpId;
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("Version");
            elem.InnerText = version;
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("DateTime");
            elem.InnerText = DateTime.Now.ToString();
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("OS");
            elem.InnerText = Environment.OSVersion.ToString();
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("CLR");
            elem.InnerText = Environment.Version.ToString();
            doc.LastChild.AppendChild(elem);

            elem = doc.CreateElement("MemoryAvailable");
            PerformanceCounter ramCounter = new PerformanceCounter("Memory", "Available MBytes");
            elem.InnerText = ramCounter.NextValue().ToString();
            doc.LastChild.AppendChild(elem);

            XmlWriter write = XmlWriter.Create(xmlDest);
            doc.WriteTo(write);
            write.Close();
           
        }

        private void TarCreation()
        {

            
            Stream outStream = File.Create(tarDest);

            TarArchive tar = TarArchive.CreateOutputTarArchive(outStream);
            System.IO.Directory.SetCurrentDirectory(System.IO.Path.GetTempPath());
            TarEntry xmlEntry = TarEntry.CreateEntryFromFile(miniDumpId+".xml");
            tar.WriteEntry(xmlEntry, false);

            System.IO.Directory.SetCurrentDirectory(dumpDir);
            TarEntry dump = TarEntry.CreateEntryFromFile(miniDumpId +".dmp");
            
            tar.WriteEntry(dump, false);
            tar.Close();

        }

        public enum sendEnum : int
        {
            NoSend = 1,
            GetPermission = 2,
            SendAutomatically = 4
        }

        private void cancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void sendErrorButton_Checked(object sender, RoutedEventArgs e)
        {
            if (!permanentCheck.IsEnabled)
                permanentCheck.IsEnabled = true;
            if (!okButton.IsEnabled)
                okButton.IsEnabled = true;
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            if (sendErrorButton.IsChecked.Value)
            {
                sendError();
                if (permanentCheck.IsChecked.Value)
                {
                    registerPref(sendEnum.SendAutomatically);
                }
            }
            else if (noSendErrorButton.IsChecked.Value && permanentCheck.IsChecked.Value)
            {
                registerPref(sendEnum.NoSend);
            }
            /*
            Process explorer = new Process();
            explorer.StartInfo.FileName = "explorer.exe";
            explorer.Start();
            */
            this.Close();
        }

        private void sendError()
        {
            CreateReport();

        }

        private void registerPref(sendEnum pref)
        {
            Registry.SetValue("HKEY_CURRENT_USER\\Software\\VistaMetadataProject", "AutoCrashReporting", (int)pref, RegistryValueKind.DWord);
        }
    }
}
