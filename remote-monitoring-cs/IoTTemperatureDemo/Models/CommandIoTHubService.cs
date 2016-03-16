using Microsoft.Azure.Devices;
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
    /// <summary>
    /// Sends commands from your .Net application to the IoT Hub.
    /// </summary>
    public class CommandIoTHubService
    {
        public const string OFF_COMMAND_STRING = "TurnFanOff";

        public const string ON_COMMAND_STRING = "TurnFanOn";

        public static ServiceClient ioTHubClient = ServiceClient.CreateFromConnectionString(ConfigurationManager.ConnectionStrings["IoTHubService"].ConnectionString);
       
     
        public static async Task RespondWithDeviceOnAsync(string deviceId)
        {
            await RespondToDeviceWithMessage(deviceId, ON_COMMAND_STRING);
        }

        public static async Task RespondWithDeviceOffAsync(string deviceId)
        {
            await RespondToDeviceWithMessage(deviceId, OFF_COMMAND_STRING);
        }

private static async Task RespondToDeviceWithMessage(string deviceId, string message)
{
    var commandEvent = new
    {
        Name = message,
        Parameters = ""
    };
    var messageString = JsonConvert.SerializeObject(commandEvent);
    var payload = new Message(Encoding.UTF8.GetBytes(messageString));
    var ioTHubClient = ServiceClient.CreateFromConnectionString(ConfigurationManager.ConnectionStrings["IoTHubService"].ConnectionString);

    //Asynchronously send the command to your device.
    await  ioTHubClient.SendAsync(deviceId, payload);
}

    }
}