using Microsoft.ServiceBus.Messaging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace IoTTemperatureDemo
{
    /// <summary>
    /// Description of data needed to populate Index.cshtml
    /// </summary>
    public class TemperatureDisplayViewModel
    {
        public TemperatureEntity CurrentTemperature { get; set; }

        public List<TemperatureEntity> TemperatureHistory {get; set;}

        public List<string> AlertHistory { get; set; }
    }
}