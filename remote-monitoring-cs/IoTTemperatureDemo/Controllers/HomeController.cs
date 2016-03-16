using Microsoft.ServiceBus.Messaging;
using Microsoft.WindowsAzure.Storage;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using System.Web.Mvc;

namespace IoTTemperatureDemo.Controllers
{
    public class HomeController : Controller
    {
        [HttpGet]
        public ActionResult Index()
        {
            IEnumerable<TemperatureEntity> allTemperatures = null;
            TemperatureEntity currentTemperature = null;
            try
            {
                // Load temperature data through your table storage access code, sorted by most recently generated datum first.
                allTemperatures = TableStorageAccess.LoadTemperatureDataForDemoDevice()
                      .OrderByDescending(t => t.eventtime);

                currentTemperature = allTemperatures.FirstOrDefault();
            }
            // A 404 exception in response to a table storage query execution can mean the table doesn't exist in your Storage Account. Your table will be automatically created as soon as an Azure Stream Analytics service attempts to add a piece a data to it.
            catch (StorageException e)
            {
                allTemperatures = null;
                currentTemperature = null;
            }
            // Seperate the current temperature and past temperatures. Get only the first 10
            IEnumerable<String> alertReceived = null;
            var temperatureHistory = allTemperatures?.Skip(1)?.ToList();
            var tempCount = (temperatureHistory.Count() < 10) ? temperatureHistory.Count : 10;
            temperatureHistory = temperatureHistory.GetRange(0, tempCount);
            List<string> alertHistory = null;

            try
            {
                // Load alert history from your event hub.
                alertReceived = AlertEventHubAccess.ReceiveAlertsFromDeviceAsync("0").Result
                    .Select(e => Encoding.UTF8.GetString(e.GetBytes()));
                alertHistory = alertReceived.ToList();
            }
            // A 404 exception in response to an event hub query execution can mean the hub contains no events. Your hub will start to respond as soon as an Azure Stream Analytics service adds an event to it.
            catch (AggregateException e)
            {
                alertHistory = null;
            }

            var viewModel = new TemperatureDisplayViewModel()

            {
                AlertHistory = alertHistory ?? new List<string> { "No alerts found." },
                CurrentTemperature = currentTemperature ?? null,
                TemperatureHistory = temperatureHistory ?? new List<TemperatureEntity> { new TemperatureEntity("No temperature history found.", -404, DateTime.UtcNow.Subtract(new DateTime(1970,1,1,0,0,0)).ToString()) },
            };
            return View(viewModel);
        }



        // Turn your green LED on remotely.
        public async Task<ActionResult> ToggleDeviceOn()
        {
            ViewBag.Message = "Sent Command -- Turn Green LED On";
            await CommandIoTHubService.RespondWithDeviceOnAsync(TableStorageAccess.DEVICE_ID);
            return Index();
        }

        // Turn your red LED off remotely.
        public async Task<ActionResult> ToggleDeviceOff()
        {
            ViewBag.Message = "Sent Command -- Turn Red LED On";
            await CommandIoTHubService.RespondWithDeviceOffAsync(TableStorageAccess.DEVICE_ID);
            return Index();
        }

    }
}