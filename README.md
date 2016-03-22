---
services: iot-hub, stream-analytics, event-hubs, web-apps, documentdb, storage-accounts
platforms: csharp,raspberry-pi
author: AttunixCorp
---

# Get Started with Microsoft Azure IoT Kit - Raspberry Pi

Welcome to the Get Started page for your Azure IoT Starter Kit - Raspberry Pi 2. In this page, you will find two tutorials that will walk you through different scenarios. You can choose to do whichever you want to, based on your preferences. The first tutorial will show you how to connect your Azure IoT Starter kit to our Remote Monitoring preconfigured solution from Azure IoT Suite. In the second tutorial, you will leverage Azure IoT services to create your own IoT architecture. 

- [Running a Simple Remote Monitoring Solution on Raspberry Pi2 with Raspbian](#running-a-simple-remote-monitoring-solution-on-raspberry-pi2-with-raspbian)
- [Using Microsoft Azure IoT to Process and Use Sensor Data to Indicate Abnormal Temperatures](#using-microsoft-azure-iot-to-process-and-use-sensor-data-to-indicate-abnormal-temperatures)

# Running a Simple Remote Monitoring Solution on Raspberry Pi2 with Raspbian

This tutorial describes the process of taking your Microsoft Azure IoT Starter Kit for Raspberry Pi 2, and using it to develop a temperature and humidity reader that can communicate with the cloud using the Raspbian OS and Microsoft Azure IoT SDK. For Windows 10 IoT Core samples, please visit [windowsondevices.com](http://www.windowsondevices.com/).

## Table of Contents

- [Tutorial Overview](#tutorial-overview)
- [Before Starting](#before-starting)
  - [Required Software](#required-software)
  - [Required Hardware](#required-hardware)
- [Create a New Azure IoT Suite Remote Monitoring solution and Add Device](#create-a-new-azure-iot-suite-remote-monitoring-solution-and-add-device)
- [Preparing the Device](#preparing-the-device)
  - [Logging in and Accessing the Terminal](#logging-in-and-accessing-the-terminal)
  - [Logging in Using PuTTY](#logging-in-using-putty)
  - [Logging in using Linux or Mac OS](#logging-in-using-linux-or-mac-os)
- [Update Solution Connection Data](#update-solution-connection-data)
  - [Updating the Device Data](#updating-the-device-data)
  - [Updating the Connection String](#updating-the-connection-string)
- [Build the Modified Sample](#build-the-modified-sample)
- [Viewing the Sensor Data from the IoT Suite Portal](#viewing-the-sensor-data-from-the-iot-suite-portal)

## Tutorial Overview

- To begin the tutorial we will be adding setting up your environment on Azure using the Microsoft Azure IoT Remote Monitoring solution, getting a large portion of the set-up that would be required done in one step.
- Next we will set your device up so that it can communicate with both your computer, and Azure. 
- We will then build a test application to ensure everything we’ve done so far is working properly. A check-point, if you will.
- Next, we will build the prototype for the temperature and humidity sensor.
- Lastly, we will update the test code to include our connection data and send it to Azure to be viewed remotely.

## Before Starting

### Required Software:

- An SSH client – This makes it so you can remotely access the Raspberry Pi’s command line remotely from your computer
  - Windows doesn’t have a built-in SSH client. We recommend using [PuTTY](http://www.putty.org/)
  - Many Linux distributions and Mac OS has SSH built into their terminal. If yours does not, we recommend OpenSSH
  - See also: [SSH Using Linux of Mac OS](https://www.raspberrypi.org/documentation/remote-access/ssh/unix.md)

### Required Hardware:

- Microsoft Azure IoT Starter Kit
  - 8GB MicroSD Card (Comes with the kit, flashed with Windows 10 IoT Core)
  - A USB Mini cable (Comes with the kit)
  - An Ethernet cable or Wi-Fi dongle (Comes with the kit)

## Create a New Azure IoT Suite Remote Monitoring solution and Add Device

- Log in to [Azure IoT Suite](https://www.azureiotsuite.com/)  with your Microsoft account and click **Create a New Preconfigured Solution** 

***
**Note:** Creating an account is free. Click here to get your [Azure free trial](https://azure.microsoft.com/en-us/pricing/free-trial/)
***

- Click select in the **Remote Monitoring** option
- Type in a solution name. For this tutorial we’ll be using “Pi2Suite”
- Choose your subscription plan and geographic region, then click **Create Solution**
- Wait for Azure to finish provisioning your IoT suite, and then click **Launch**

***
**Note:** You may be asked to log back in. This is to ensure your solution has proper permissions associated with your account. 
***

- Open the link to your IoT Suite’s “Solution Dashboard.” You may have been redirected there already. 
- This opens your personal remote monitoring site at the URL &lt;Your IoT Hub suite name>.azurewebsites.net (e.g. Pi2Suite.azurewebsites.net)
- Click **Add a Device** at the lower left hand corner of your screen
- Add a new **custom device**
- Enter your desired device ID. In this case we’ll use “RaspPi”, and then click **Create**
- Make note of your device ID, Device Key, and IoT Hub Hostname to enter into the code you’ll run on your device later 

***
**Note:** Make sure to **stop** your remote monitoring solution once you have completed this to avoid unnecesary Azure consumption! 
***

## Preparing the Device

If this is the first time you are using Raspberry Pi 2, now it’s the time to set it up. If you’ll be using Windows, please use [windowsondevices.com](http://www.windowsondevices.com/) for detail
guidelines on how to get started with the Raspberry Pi. If you’re using Linux, Raspberry Pi and Adafruit have a set of tutorials and videos to help you get started.

Please visit the following links:

- [Raspberry Pi NOOBS setup page](https://www.raspberrypi.org/help/noobs-setup/)
- [Adafruit BME280 sensor setup](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test)
  - Use the section labeled "SPI Wiring" for seting this up.
  - For sensor pins, we will use the following
    - CS -> CE0 (Pin 24)
    - SCK -> SCK (Pin 23)
    - MISO -> MISO (Pin 21)
    - MOSI -> MOSI (Pin 19)

**At the end of your work, your Raspberry Pi should be connected with a working sensor.**

### Logging in and Accessing the Terminal

The default login for Raspbian is username `pi` with password `raspberry`. In the task bar up top, you can launch the Terminal using the 3rd icon from the left – The one that looks like a monitor. This is where you will be entering commands if you use the Raspbian interface directly.

### Logging in Using PuTTY

- You need to discover the IP address of your Raspberry Pi before you can connect using PuTTY. For more information see: [https://www.raspberrypi.org/documentation/remote-access/ip-address.md](https://www.raspberrypi.org/documentation/remote-access/ip-address.md)
- With the Raspberry Pi on and running, open an SSH terminal program such as [PuTTY](http://www.putty.org/) on your desktop machine.
- Use the IP address from step 4 as the Host name, Port=22, and Connection type=SSH to complete the connection.
- When prompted, log in with username `pi`, and password `raspberry`.

#### Logging in using Linux or Mac OS

- See: [SSH Using Linux of Mac OS](https://www.raspberrypi.org/documentation/remote-access/ssh/unix.md)

## Update Solution Connection Data

Now we can download the Microsoft Azure IoT SDK Remote Monitoring device sample solution and incorporate the data from the sensors and send it up to our Azure Remote Monitoring solution.

Clone the repository by entering the following commands on your Pi:
```
cd ~
git clone https://github.com/Azure-Samples/iot-hub-c-raspberrypi-getstartedkit.git
```
From here, we only need to update three major things:

- Include the required libraries (included in the git repository)
- Update the Device Data
- Update the connection string to use the string we saved above


### Updating the Device Data

To update the device data, we need to update the source file. You can edit the file by entering the following command:

```
nano ~/azure-iot-sdks/c/serializer/samples/remote_monitoring/remote_monitoring.c
```

Use the arrow keys to navigate to the portion where you see:

```
if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity))
```

Before this, you will need to update the values of your `DeviceID`. Simply set the value for each of those as follows:

```
Thermostat->DeviceId = "your-device-id";
```

### Updating the Connection String

- Use the arrow keys to navigate to the following lines:

```
static const char* deviceId = "[Device Id]";
static const char* deviceKey = "[Device Key]";
static const char* hubName = "[IoTHub Name]";
static const char* hubSuffix = "[IoTHub Suffix, i.e. azure-devices.net]";
```
- Replace the placeholder with your device and IoT Hub information you created and saved at the beginning of this tutorial. 
- Save and exit with `Control-o, Enter, Control-x`

##  Build the Modified Sample

Now that you’re exited out of
Nano, go ahead and build the updated sample solution by entering the following:
```
sudo build.sh
```
- Now that everything is compiled, it’s time to run the program. Enter the command:

```
sudo azure_rm_sample
```
If all goes well, you will begin to see data streaming!
Press `ctrl-c` to exit at any time.

```
IoTHubClient accepted the message for delivery
Humidity = 48.4% Temperature = 23.9*C
Sending sensor value Temperature = 2, Humidity = 1610612736
IoTHubClient accepted the message for delivery
```
## Viewing the Sensor Data from the IoT Suite Portal

- Once you have the sample running, visit your dashboard by visiting azureiotsuite.com and clicking “Launch” on your solution
- Make sure the “Device to View” in the upper right is set to your device
- If the demo is running, you should see the graph change as your data updates in real time!

***
**Note:** Make sure to **stop** your remote monitoring solution once you have completed this to avoid unnecesary Azure consumption! 
***



# Using Microsoft Azure IoT to Process and Use Sensor Data to Indicate Abnormal Temperatures

This tutorial describes the process of taking your Microsoft Azure IoT Starter Kit for Raspberry Pi 2, and using it to develop a temperature and humidity reader that can communicate with Microsoft Azure IoT Services, process the data,
detects abnormal data, and sends that back to the Pi for use. We will be using
the Raspbian OS and Microsoft IoT SDK. For Windows 10 IoT Core samples,
please visit [windowsondevices.com](http://www.windowsondevices.com/).

## Table of Contents

- [Tutorial Overview](#tutorial-overview)
  - [Required Software](#required-software)
  - [Required Hardware](#required-hardware)
- [Create a New Microsoft Azure IoT Hub and Add Device](#create-a-new-microsoft-azure-iot-hub-and-add-device)
- [Preparing the Device](#preparing-the-device)
  - [Logging in and Accessing the Terminal](#logging-in-and-accessing-the-terminal)
  - [Logging in Using PuTTY](#logging-in-using-putty)
- [Create an Event Hub](#create-an-event-hub)
- [Create a Storage Account for Table Storage](#create-a-storage-account-for-table-storage)
- [Create a Stream Analytics job to Save IoT Data in Table Storage and Raise Alerts](#create-a-stream-analytics-job-to-save-iot-data-in-table-storage-and-raise-alerts)
- [Web Project Setup](#web-project-setup)
- [Update Solution Connection Data](#update-solution-connection-data)
  - [Updating the Device Data](#updating-the-device-data)
  - [Updating the Connection String](#updating-the-connection-string)
- [Build the Modified Sample](#build-the-modified-sample)

## Tutorial Overview

- To begin the tutorial, we will be start with setting up an IoT Hub resource on Microsoft Azure and adding your device. 
- Then we will prepare the device, get connected to the device, and set it up so that it can read sensor data.
- We will then continue to configure your Microsoft Azure environment by adding Event Hub, Storage Account, and Stream Analytics resources.
- Next we will prepare your local web solution for monitoring and sending commands to your device.
- Last, we will update the sample code to respond to commands and include the data from our sensors, sending it to Microsoft Azure to be viewed remotely.

## Requirements

### Required Software:

- Visual Studio 2015
- [Git](https://git-scm.com/downloads) - For cloning the required repositories
- An SSH client – This makes it so you can remotely access the Raspberry Pi’s command line remotely from your computer
  - Windows doesn’t have a built-in SSH client. We recommend using [PuTTY](http://www.putty.org/)
  - Many Linux distributions and Mac OS has SSH built into their terminal. If yours does not, we recommend OpenSSH
  - See also: [SSH Using Linux of Mac OS](https://www.raspberrypi.org/documentation/remote-access/ssh/unix.md)
  
### Required Hardware:

- Microsoft Azure IoT Starter Kit
  - 8GB MicroSD Card (comes with the kit, flashed with Windows 10 IoT Core)
  - A USB Mini cable
  - An Ethernet cable or Wi-Fi dongle

## Create a New Microsoft Azure IoT Hub and Add Device

- To create your Microsoft Azure IoT Hub and add a device, follow the instructions outlined in the [Setup IoT Hub Microsoft Azure Iot SDK page](https://github.com/Azure/azure-iot-sdks/blob/master/doc/setup_iothub.md).
- After creating your device, make note of your connection string to enter into the code you’ll run on your device later

## Preparing the Device

- [Raspberry Pi NOOBS setup page](https://www.raspberrypi.org/help/noobs-setup/)
- [Adafruit BME280 sensor setup](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test)
  - Use the section labeled "SPI Wiring" for seting this up.
  - For sensor pins, we will use the following
    - CS -> CE0 (Pin 24)
    - SCK -> SCK (Pin 23)
    - MISO -> MISO (Pin 21)
    - MOSI -> MOSI (Pin 19)

**At the end of your work, your Raspberry Pi should be connected with a working sensor.**

### Logging in and Accessing the Terminal

The default login for Raspbian is username `pi` with password `raspberry`. In the task bar up top, you can launch the Terminal using the 3rd icon from the left – The one that looks like a monitor. This is where you will be entering commands if you use the Raspbian interface directly.

### Logging in Using PuTTY

- You need to discover the IP address of your Raspberry Pi before you can connect using PuTTY. For more information see: [https://www.raspberrypi.org/documentation/remote-access/ip-address.md](https://www.raspberrypi.org/documentation/remote-access/ip-address.md)
- With the Raspberry Pi on and running, open an SSH terminal program such as [PuTTY](http://www.putty.org/) on your desktop machine.
- Use the IP address from step 4 as the Host name, Port=22, and Connection type=SSH to complete the connection.
- When prompted, log in with username `pi`, and password `raspberry`.



## Create an Event Hub

- Log on to the [Microsoft Azure Portal](https://portal.azure.com/)
- Click on **New** -&gt; **Internet of Things**-&gt; **Event Hub**
- After being redirected, click "Custom Create", Enter the following settings for the Event Hub (use a name of your choice for the event hub and the namespace):
    - Event Hub Name: `piEventHub`
    - Region: `Your choice`
    - Subscription: `Your choice`
    - Namespace Name: `Your Project Namespace, in our case “Pi2Suite”`
- Click the **arrow** to continue.
- Choose to create **4** partitions and retain messages for **7** days.
- Click the **check** at the bottom right hand corner to create your event hub.
- Click on your `Pi2Suite` service bus (what you named your service bus)
- Click on the **Event Hubs** tab
- Select the `piEventHub` eventhub and go in the **Configure** tab in the **Shared Access Policies** section, add a new policy:
    - Name = `readwrite`
    - Permissions = `Send, Listen`
- Click **Save** at the bottomof the page, then click the **Dashboard** tab near the top and click on **Connection Information** at the bottom
- _Copy down the connection string for the `readwrite` policy you created._
- From the your IoT Hub Settings (The Resource that has connected dots) on the [Microsoft Azure Portal](https://portal.azure.com/), click the **Messaging blade** (found in your settings), write down the _Event Hub-compatible name_
- Look at the _Event-hub-compatible Endpoint_, and write down this part: sb://**thispart**.servicebus.windows.net/ we will call this one the _IoTHub EventHub-compatible namespace_

## Create a Storage Account for Table Storage

- Log on to the [Microsoft Azure Portal](https://portal.azure.com/)
- In the menu, click **New** and select **Data + Storage** then **Storage Account**
- Choose **Classic** for the deployment model and click on **Create**
- Enter the name of your choice (We chose `pistorage`) for the account name, `Standard-RAGRS` for the type, choose your subscription, select the resource group you created earlier, then click on **Create**
- Once the account is created, find it in the **resources blade** or click on the **pinned tile**, go to **Settings**, **Keys**, and write down the _primary connection string_.

## Create a Stream Analytics job to Save IoT Data in Table Storage and Raise Alerts

- Log on to the [Microsoft Azure Portal](https://portal.azure.com/)
- In the menu, click **New**, then click **Internet of Things**, and then click **Stream Analytics Job**
- Enter a name for the job (We chose “PiStorageJob”), a preferred region, then choose your subscription. At this stage you are also offered to create a new or to use an existing resource group. Choose the resource group you created earlier (In our case, `Pi2Suite`).
- Once the job is created, open your **Job’s blade** or click on the **pinned tile**, and find the section titled _“Job Topology”_ and click the **Inputs** tile. In the Inputs blade, click on **Add**
- Enter the following settings:
    - Input Alias = _`TempSensors`_
    - Source Type = _`Data Stream`_
    - Source = _`IoT Hub`_
    - IoT Hub = _`Pi2Suite`_ (use the name for the IoT Hub you create before)
    - Shared Access Policy Name = _`device`_
    - Shared Access Policy Key = _The `device` primary key saved from earlier_
    - IoT Hub Consumer Group = "" (leave it to the default empty value)
    - Event serialization format = _`JSON`_
    - Encoding = _`UTF-8`_

- Back to the **Stream Analytics Job blade**, click on the **Query tile** (next to the Inputs tile). In the Query settings blade, type in the below query and click **Save**:

```
SELECT
    DeviceId,
    EventTime,
    MTemperature as TemperatureReading
INTO
    TemperatureTableStorage
from TempSensors
WHERE
   DeviceId is not null
   and EventTime is not null

SELECT
    DeviceId,
    EventTime,
    MTemperature as TemperatureReading
INTO   
    TemperatureAlertToEventHub
FROM
    TempSensors
WHERE TemperatureReading>30 
```
***
**Note:** You can change the `30` to `0` when you're ready to generate alerts to look at. This number represents the temperature in degrees celsius to check for when creating alerts.
***

- Back to the **Stream Analytics Job blade**, click on the **Outputs** tile and in the **Outputs blade**, click on **Add**
- Enter the following settings then click on **Create**:
    - Output Alias = _`TemperatureTableStorage`_
    - Sink = _`Table Storage`_
    - Storage account = _`pistorage`_ (The storage you made earlier)
    - Storage account key = _(The primary key for the storage account made earlier, can be found in Settings -&gt; Keys -&gt; Primary Access Key)_
    - Table Name = _`TemperatureRecords`_*Your choice - If the table doesn’t already exist, Local Storage will create it
    - Partition Key = _`DeviceId`_
    - Row Key = _`EventTime`_
    - Batch size = _`1`_
- Back to the **Stream Analytics Job blade**, click on the **Outputs tile**, and in the **Outputs blade**, click on **Add**
- Enter the following settings then click on **Create**:
    - Output Alias = _`TemperatureAlertToEventHub`_
    - Source = _`Event Hub`_
    - Service Bus Namespace = _`Pi2Suite`_
    - Event Hub Name = _`pieventhub`_ (The Event Hub you made earlier)
    - Event Hub Policy Name = _`readwrite`_
    - Event Hub Policy Key = _Primary Key for readwrite Policy name (That's the one you wrote down after creating the event hub)_
    - Partition Key Column = _`0`_
    - Event Serialization format = _`JSON`_
    - Encoding = _`UTF-8`_
    - Format = _`Line separated`_
- Back in the** Stream Analytics blade**, start the job by clicking on the **Start **button at the top

***
**Note:** Make sure to **stop** your Command Center jobs once you have completed this to avoid unnecesary Azure consumption! 
***

## Web Project Setup

 - Clone the web project repository on your local machine by entering the following commands in your command prompt/terminal:
```
git clone https://github.com/Azure-Samples/iot-hub-c-raspberrypi-getstartedkit.git
```
 - Open your `IoTTemperatureDemo` project and open the project’s `web.config`
 - Copy/paste the provided connection strings from
your target Event Hub, Table Storage account, and IoT Hub into `web.config`, named “AlertEventHub”, “TableStorage”,
and “IoTHubService” respectively. (For reference, you can go to each respective
section in the Microsoft Azure Portal, and look at the properties / keys for this
information, but it is primarily replacing the information with your storage
account name and your shared access key.)

```
<connectionStrings>
 <add name="AlertEventHub" connectionString="Endpoint=sb://your-eventhub-namespace.servicebus.windows.net/;SharedAccessKeyName=readwrite;SharedAccessKey=your-key" />
 <add name="TableStorage" connectionString="DefaultEndpointsProtocol=https;AccountName=your-storage-account-name;AccountKey=your-storage-key;BlobEndpoint=https://your-storage-account-name.blob.core.windows.net/;TableEndpoint=https://your-storage-account-name.table.core.windows.net/;QueueEndpoint=https://your-storage-account-name.queue.core.windows.net/;FileEndpoint=https://your-storage-account-name.file.core.windows.net/" />
 <add name="IoTHubService" connectionString="HostName=your-host-name.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=your-shared-access-key"/>
</connectionStrings>
```
Open `Models/TableStorageAccess.cs` and
set your device ID (From the previous tutorial) and table name (Table Storage).

```
public const string DEVICE_ID = "your-device-id";
public const string TABLE_NAME = "your-table-name";
```
 - Open `Models/AlertEventHubAccess.cs` and set your event hub alerts name. 

```
public const string EVENTHUB_NAME = "your-eventhub-name";  
```

 - Open `Models/TableStorageAccess.cs` and set your device ID and table name. 

```
public const string DEVICE_ID = "your-device-id";
public const string TABLE_NAME = "TemperatureRecords";
```

 - Now it is time to build the project and run it. Click the **Build** menu button and click **Build** (or press F6). Then click the **Debug** menu button and click **Start Debugging** (or press F5) to run the code.
    - You may be asked to choose a browser, if so, navigate to the directory where your browser is located and choose that.
 - If you are asked to find an “Executor” file, go ahead and click **cancel**.

Now you will be able to view your device’s temperature data,
view alerts if temperature gets too high, and send commands back to your device.

To deploy this project to the cloud using Azure, you can reference [Deploying WebJobs using Visual Studio](https://azure.microsoft.com/en-us/documentation/articles/websites-dotnet-deploy-webjobs/).

Next, we will update your device so that it can interact with all the things you just created.

## Update Solution Connection Data

Now we can download the Microsoft Azure
SDK sample solution and incorporate the data from the sensors and send it up to our Microsoft Azure Remote Monitoring solution.

Clone the repository by entering the following commands on your Pi:

```
cd ~
git clone https://github.com/Azure-Samples/iot-hub-c-raspberrypi-getstartedkit.git
```
From here, we only need to update three major things:

- Include the required libraries (included in the git repository)
- Update the Device Data
- Update the connection string to use the string we saved above

### Updating the Device Data

To update the device data, we need to update the source file. You can edit the file by entering the following command:

```
nano ~/azure-iot-sdks/c/serializer/samples/remote_monitoring/remote_monitoring.c
```

 - Use the arrow keys to navigate to the portion where you see:

```
if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity))
```
### Updating the Connection String

- Use the arrow keys to navigate to the following lines:

```
static const char* connectionString = "HostName=<host_name>.azure-devices.net;DeviceId=<device_id>;SharedAccessKey=<device_key>";
```
- Replace the placeholder with your device connection string you created at the beginning of this tutorial. 
- Save and exit with `Control-o, Enter, Control-x`

## Build the Modified Sample

- Go ahead and run the build process:

```
sudo build.sh
```
- Now that everything is compiled, it’s time to run the program. Enter the command:

```
sudo azure_sample
```
You will now see data being sent off at regular intervals to
Microsoft Azure. When it detects something out of range, you will see the LED you’ve set
up turn from green to red! 

Head back and run your Visual Studio solution and you will see the most
recent updates and any alerts show up there.

***
**Note:** Make sure to **stop** your Command Center jobs once you have completed this to avoid unnecesary Azure consumption! 
***