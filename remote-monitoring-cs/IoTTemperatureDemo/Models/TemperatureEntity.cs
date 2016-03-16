using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTTemperatureDemo
{
    /// <summary>
    /// Definition of data types contained in temperature records sent through the IoT hub to Azure table storage.
    /// Note that Azure Table Storage normalizes all column names to lower case, so entity properties must be lower case too.
    /// </summary>
    public class TemperatureEntity : TableEntity
    {
        /// <summary>
        /// Id of the device that sent this event. Forms a unique identifier when combined with eventtime..
        /// </summary>
        public string deviceid { get; set; }

        /// <summary>
        /// Temperature recorded from sensor.
        /// </summary>
        private string tr;
        public double temperaturereading {
            get
            {
                if (tr != null)
                {
                    double x = double.Parse(tr);
                    return double.Parse(x.ToString("n2"));
                } else
                {
                    return -404;
                }
                
            }
            set
            {
                tr = value.ToString();
            }
        }

        /// <summary>
        /// Time the event was captured on the device. Forms a unique identifier when combined with deviceid.
        /// </summary>
        private string et;
        public string eventtime {
            
            get
            {
                int x;
                try
                {
                    x = Int32.Parse(et);
                } catch
                {
                    x = 0;
                }
                DateTime epoch = new DateTime(1970, 1, 1, 0, 0, 0);
                DateTime now = epoch.AddSeconds(x);
                return now.ToString("hh:mm:ss on yy/MM/dd");
            }
            set
            {
                et = value;
            }
        }
        public TemperatureEntity(string deviceId, double temperature, string eventTime)
        {
            deviceid = deviceId;
            temperaturereading = temperature;
            eventtime = eventTime;
        }
            
        public TemperatureEntity()
        {
        }
    }
}
