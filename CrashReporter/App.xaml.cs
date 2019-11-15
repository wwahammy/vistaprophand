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
using System.Configuration;
using System.Data;
using System.Windows;
using Microsoft.Win32;
using System.Diagnostics;

namespace CrashReporter
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            string[] args = e.Args;

            if (args.Length != 4)
                App.Current.Shutdown();
            else
            {
                CrashReporter.MainWindow.sendEnum send = getPref();

                if (send == CrashReporter.MainWindow.sendEnum.GetPermission || send == CrashReporter.MainWindow.sendEnum.SendAutomatically)
                {
                    CrashReporter.MainWindow window = new CrashReporter.MainWindow(args[0], args[1], args[2], args[3], send);
                    window.Show();
                }
                else
                {
                    /*
                    Process explorer = new Process();
                    explorer.StartInfo.FileName = "explorer.exe";
                    explorer.Start();
                    */
                    App.Current.Shutdown();
                }
            }
        }



        private static CrashReporter.MainWindow.sendEnum getPref()
        {
            CrashReporter.MainWindow.sendEnum send;

            object val = Registry.GetValue("HKEY_CURRENT_USER\\Software\\VistaMetadataProject", "AutoCrashReporting", CrashReporter.MainWindow.sendEnum.GetPermission);

            if (val == null)
            {
                send = CrashReporter.MainWindow.sendEnum.GetPermission;
            }
            else if ((CrashReporter.MainWindow.sendEnum)val != CrashReporter.MainWindow.sendEnum.GetPermission)
            {
                send = (CrashReporter.MainWindow.sendEnum)Enum.Parse(typeof(CrashReporter.MainWindow.sendEnum), ((int)val).ToString());
            }
            else
                send = (CrashReporter.MainWindow.sendEnum)val;

            return send;
        }
    }
}
