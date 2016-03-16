using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Web;

namespace IoTTemperatureDemo
{
    /// <summary>
    /// Allows temperature data to be read from Azure Table Storage.
    /// </summary>
    public class TableStorageAccess
    {
        public static string iotHubD2cEndpoint = "messages/events";
        
        public const string DEVICE_ID = "raspy";
        public const string TABLE_NAME = "TemperatureRecords2";
        public static IEnumerable<TemperatureEntity> LoadTemperatureDataForDemoDevice()
        {
            return LoadTemperatureDataForDevice(DEVICE_ID);
        }

        public static IEnumerable<TemperatureEntity> LoadTemperatureDataForDevice(string deviceId)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(
     ConfigurationManager.ConnectionStrings["TableStorage"].ConnectionString);
            // Create the table client.
            CloudTableClient tableClient = storageAccount.CreateCloudTableClient();

            // Create the table if it doesn't exist.
            CloudTable table = tableClient.GetTableReference(TABLE_NAME);

            //Create a query to get all temperature entities for our device. 
            //Records have been partitioned by DeviceId for efficient bulk access.
            TableQuery<TemperatureEntity> query = new TableQuery<TemperatureEntity>()
             .Where(TableQuery.GenerateFilterCondition("PartitionKey", QueryComparisons.Equal, deviceId));

            var entries = table.ExecuteQuery(query);
            return entries;
        }
    }
}