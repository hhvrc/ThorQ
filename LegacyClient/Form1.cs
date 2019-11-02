using System;
using System.IO;
using System.Net;
using System.Linq;
using System.Drawing;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;
using System.Collections.Generic;
using Newtonsoft.Json;

namespace CollarControl
{
    public partial class MainForm : Form
    {
        [Serializable]
        struct InqMessage
        {
            public string Signature;
            public string SessionID;
            public string Username;
            public string Message;
            public DateTime TimeStamp;
        }
        class InqUser
        {
            public string Username;
            public string SessionID;
            public DateTime LastSeenTime;

            public override string ToString()
            {
                return Username;
            }
        }

        const int updateFrequency = 60;
        const int updateSleepTimeMs = 1000 / updateFrequency;

        Thread netHandler = null;
        private volatile bool _isRunning = true;
        private Color _defaultColor;
        private List<InqUser> _activeUsers = null;
        private string _activeMasterSessionId = "";
        private DateTime lastDateTime = DateTime.UtcNow;

        private delegate void TextboxDelegate(string text);
        private delegate void ComboboxDelegate(InqUser user);

        public string ActiveMasterSessionId
        {
            get
            {
                if (_activeMasterSessionId == null)
                {
                    return null;
                }
                string result;
                lock (_activeMasterSessionId)
                {
                    result = String.Copy(_activeMasterSessionId);
                }
                return result;
            }
            set
            {
                if (_activeMasterSessionId == null || value == null)
                {
                    return;
                }
                lock (_activeMasterSessionId)
                {
                    _activeMasterSessionId = String.Copy(value);
                }
            }
        }

        private void SetOutSessionID(string text)
        {
            if (OutSessionID.InvokeRequired)
            {
                var d = new TextboxDelegate(SetOutSessionID);
                OutSessionID.Invoke(d, new object[] { text });
            }
            else
            {
                OutSessionID.Text = text;
            }
        }

        private void SetOutUsername(string text)
        {
            if (OutUsername.InvokeRequired)
            {
                var d = new TextboxDelegate(SetOutUsername);
                OutUsername.Invoke(d, new object[] { text });
            }
            else
            {
                OutUsername.Text = text;
            }
        }

        private void AddToComboboxActiveUsers(InqUser user)
        {
            if (ComboboxActiveUsers.InvokeRequired)
            {
                var d = new ComboboxDelegate(AddToComboboxActiveUsers);
                ComboboxActiveUsers.Invoke(d, new object[] { user });
            }
            else
            {
                ComboboxActiveUsers.Items.Add(user);
            }
        }

        private void RemoveFromComboboxActiveUsers(InqUser user)
        {
            if (ComboboxActiveUsers.InvokeRequired)
            {
                var d = new ComboboxDelegate(RemoveFromComboboxActiveUsers);
                ComboboxActiveUsers.Invoke(d, new object[] { user });
            }
            else
            {
                if (user != null)
				{
                    if (((InqUser)ComboboxActiveUsers.SelectedItem) == user)
                    {
                        ComboboxActiveUsers.SelectedIndex = -1;
                        SetOutUsername("");
                        SetOutSessionID("");
                        ActiveMasterSessionId = "";
                    }
					ComboboxActiveUsers.Items.Remove(user);
				}
                ComboboxActiveUsers.Refresh();
            }
        }

        public MainForm()
        {
            InitializeComponent();
            _activeUsers = new List<InqUser>();
            netHandler = new Thread(NetHandler);
            netHandler.Start();

            lock (MasterInfoBox)
                _defaultColor = MasterInfoBox.BackColor;
        }

        public void Cleanup()
        {
            _isRunning = false;
            netHandler.Join();
            netHandler = null;
        }

        private void NetHandler()
        {
            byte[] WARNING_ENABLE_KEY = new byte[] { (byte)'+' };
            byte[] WARNING_DISABLE_KEY = new byte[] { (byte)'-' };
            byte[] PUNISH_ENABLE_KEY = new byte[] { (byte)'!' };
            byte[] PUNISH_DISABLE_KEY = new byte[] { (byte)'_' };
            byte[] ALL_STOP_KEY = new byte[] { (byte)'#' };
            byte[] STREAM_WARNING_KEY = new byte[] { (byte)'W' };
            byte[] STREAM_PUNISH_KEY = new byte[] { (byte)'P' };

            ActiveMasterSessionId = "";

            SerialPort arduino = new SerialPort("COM5", 9600);
            arduino.Open();
            while (_isRunning)
            {
                lock (_activeUsers)
                {
                    List<InqUser> users = _activeUsers.FindAll(u => u.LastSeenTime < (DateTime.UtcNow - TimeSpan.FromSeconds(20)));
                    foreach (InqUser user in users)
                    {
                        Console.WriteLine("[{0}] User {1} timed out!", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), user.Username);
                        RemoveFromComboboxActiveUsers(user);
                        _activeUsers.Remove(user);
                    }
                }

                if (!arduino.IsOpen)
                {
                    arduino.Dispose();
                    arduino = null;
                    arduino = new SerialPort("COM5", 9600);
                    arduino.Open();
                }

                string html = "";
                string url = "http://hh.heavenvr.tech/api/inputs/" + lastDateTime.ToString("yyyy-MM-ddTHH:mm:ss.fff").Replace(":", "i").Replace(".", "d");

                try
                {
                    HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
                    request.AutomaticDecompression = DecompressionMethods.None;

                    using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                    using (Stream stream = response.GetResponseStream())
                    using (StreamReader reader = new StreamReader(stream))
                    {
                        html = reader.ReadToEnd();
                    }
                    if (html == "null")
                    {
                        Console.WriteLine("Datetime not correct format: " + lastDateTime);
                        continue;
                    }

                    InqMessage[] inqs = JsonConvert.DeserializeObject<InqMessage[]>(html);

                    if (inqs == null || inqs.Length == 0)
                    {
                        continue;
                    }
                    
                    for (int i = inqs.Length - 1; i >= 0; i--)
                    {
                        InqMessage inq = inqs[i];

                        lock (_activeUsers)
                        {
                            int index = _activeUsers.FindIndex(u => u.SessionID == inq.SessionID);
                            if (index >= 0)
                            {
                                _activeUsers[index].LastSeenTime = inq.TimeStamp;
                                if (inq.Message == "LoggedOut")
                                {
                                    InqUser user = _activeUsers[index];
                                    RemoveFromComboboxActiveUsers(user);
                                    _activeUsers.Remove(user);
                                    Console.WriteLine("[{0}] User {1} logged off!", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), inq.Username);
                                }
                            }
                            else
                            {
                                InqUser inqUser = new InqUser() { Username = inq.Username, SessionID = inq.SessionID, LastSeenTime = inq.TimeStamp };
                                _activeUsers.Add(inqUser);
                                Console.WriteLine("[{0}] User {1} is online!", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), inq.Username);
                                AddToComboboxActiveUsers(inqUser);
                            }
                        }
                        if (arduino.IsOpen && inq.SessionID == ActiveMasterSessionId)
                        {
                            switch (inq.Message)
                            {
                                case "PunishmentStart":
                                    lock (MasterInfoBox)
                                        MasterInfoBox.BackColor = Color.Red;
                                    Console.WriteLine("[{0}] Master {1} began punishment", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), inq.Username);
                                    arduino.Write(PUNISH_ENABLE_KEY, 0, PUNISH_ENABLE_KEY.Length);
                                    break;
                                case "WarningStart":
                                    lock (MasterInfoBox)
                                        MasterInfoBox.BackColor = Color.Orange;
                                    Console.WriteLine("[{0}] Master {1} sent a warning", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), inq.Username);
                                    arduino.Write(WARNING_ENABLE_KEY, 0, WARNING_ENABLE_KEY.Length);
                                    break;
                                case "AllStop":
                                    lock (MasterInfoBox)
                                        MasterInfoBox.BackColor = _defaultColor;
                                    Console.WriteLine("[{0}] Master {1} stopped", DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff"), inq.Username);
                                    arduino.Write(ALL_STOP_KEY, 0, ALL_STOP_KEY.Length);
                                    break;
                                default:
                                    break;
                            }
                        }

                        lastDateTime = inq.TimeStamp;
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine("Error: " + e.Message);
                }
                Thread.Sleep(updateSleepTimeMs);
            }
            if (!arduino.IsOpen)
            {
                arduino.Dispose();
                arduino = null;
                arduino = new SerialPort("COM5", 9600);
                arduino.Open();
            }
            arduino.Write(ALL_STOP_KEY, 0, ALL_STOP_KEY.Length);
        }

        private void BtnSetMaster_Click(object sender, EventArgs e)
        {
            InqUser user = (InqUser)ComboboxActiveUsers.SelectedItem;
            ActiveMasterSessionId = user.SessionID;
            SetOutSessionID(user.SessionID);
            SetOutUsername(user.Username);
        }
    }
}
