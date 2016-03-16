using Microsoft.Azure.Devices;
using Microsoft.ServiceBus;
using Microsoft.ServiceBus.Messaging;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web;

namespace IoTTemperatureDemo
{
    public class AlertEventHubAccess
    {

        public static DateTime ALERT_HISTORY_START_DATE = DateTime.Parse("01/01/2012");
        public static int MAX_ALERTS_TO_DISPLAY = 1000;
        public const string EVENTHUB_NAME = "pieventhub";
        public static async Task<IEnumerable<EventData>> ReceiveAlertsFromDeviceAsync(string partition)
        {
            //It's strongly advised to include the name of your Event Hub explicitly as an argument
            //as well as including it in your connection string.
            var eventHubClient = EventHubClient.CreateFromConnectionString(ConfigurationManager.ConnectionStrings["AlertEventHub"].ConnectionString, EVENTHUB_NAME);
           
            //Create a receiver to read alerts sent to our event hub, starting from the given date.
            var eventHubReceiver = eventHubClient.GetDefaultConsumerGroup().CreateReceiver(partition);

            //Retrieve alerts up to our limit.
            var alertEntries = eventHubReceiver.Receive(MAX_ALERTS_TO_DISPLAY);

            //Always close your event hub receiver when you're done using it. 
            //Each consumer group is allocated a limited number of receiver connections.
            eventHubReceiver.Close();
            return alertEntries;
        }

       
    }
}