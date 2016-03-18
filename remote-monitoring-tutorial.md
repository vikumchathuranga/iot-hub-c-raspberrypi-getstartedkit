# Running a Simple Remote Monitoring Solution on Raspberry Pi2 with Raspbian

This tutorial describes the process of taking your Microsoft IoT Pack for Raspberry Pi 2, and using it to develop a temperature and humidity reader that can communicate with the cloud using the Raspbian OS and Microsoft IoT SDK. For Windows 10 IoT Core samples, please visit [windowsondevices.com](http://www.windowsondevices.com/).

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
- [Introducing Sensors to your Device](#introducing-sensors-to-your-device)
- [Update Solution with Actual Sensor Data](#update-solution-with-actual-sensor-data)
- [Updating the Simulated Data with Actual Sensor Data](#updating-the-simulated-data-with-actual-sensor-data)
- [Updating the Connection String](#updating-the-connection-string)
- [Build the Modified Sample](#build-the-modified-sample)
- [Viewing the Sensor Data from the IoT Suite Portal](#viewing-the-sensor-data-from-the-iot-suite-portal)

## Tutorial Overview

- To begin the tutorial we will be adding setting up your environment on Azure using the Azure IoT Remote Monitoring solution, getting a large portion of the set-up that would be required done in one step.
- Next we will set your device up so that it can communicate with both your computer, and Azure. 
- We will then build a test application to ensure everything we’ve done so far is working properly. A check-point, if you will.
- Next, we will build the prototype for the temperature and humidity sensor.
- Lastly, we will update the test code to include the data from our sensors and send it to Azure to be viewed remotely.

## Before Starting

### Required Software:

- An SSH client – This makes it so you can remotely access the Raspberry Pi’s command line remotely from your computer
- Windows doesn’t have a built-in SSH client. We recommend using [PuTTY](http://www.putty.org/)
- Many Linux distributions and Mac OS has SSH built into their terminal. If yours does not, we recommend OpenSSH
- See also: [SSH Using Linux of Mac OS](https://www.raspberrypi.org/documentation/remote-access/ssh/unix.md)

### Required Hardware:

- Microsoft IoT Starter Kit, Raspberry Pi 2 Edition
- 8GB MicroSD Card (comes with the kit, flashed with Windows 10 IoT Core)
- A USB Mini cable
- A 5 Volt - 1 Amp USB power supply
- An Ethernet cable or Wi-Fi dongle

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
- This opens your personal remote monitoring site at the URL &lt; Your IoT Hub suite name >.azurewebsites.net (e.g. Pi2Suite.azurewebsites.net)
- Click **Add a Device** at the lower left hand corner of your screen
- Add a new **custom device**
- Enter your desired device ID. In this case we’ll use “RaspPi”, and then click **Create**
- Make note of your device ID, Device Key, and IoT Hub Hostname to enter into the code you’ll run on your device later 

## Preparing the Device

If this is the first time you are using Raspberry Pi 2, now
it’s the time to set it up. If you’ll be using Windows, please use [windowsondevices.com](http://www.windowsondevices.com/) for detail
guidelines on how to get started with the Raspberry Pi. If you’re using Linux,
Raspberry Pi and Adafruit have a set of tutorials and videos to help you get started.
Please visit the following links:

- [Raspberry Pi NOOBS setup page](https://www.raspberrypi.org/help/noobs-setup/)
- [Adafruit Raspberry Pi lesson](https://learn.adafruit.com/adafruit-raspberry-pi-lesson-1-preparing-and-sd-card-for-your-raspberry-pi/)
- [Adafruit BME280 sensor setup](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test)

**At the end of your work, your Raspberry Pi should be connected with a working sensor.**

### Logging in and Accessing the Terminal

The default login for Raspbian is username pi with password raspberry. In the task bar up top, you can launch the Terminal using the 3rd icon from the left – The one that looks like a monitor. This is where you will be entering commands if you use the Raspbian interface directly.

### Logging in Using PuTTY

- You need to discover the IP address of your Raspberry Pi before you can connect using PuTTY. For more information see: [https://www.raspberrypi.org/documentation/remote-access/ip-address.md](https://www.raspberrypi.org/documentation/remote-access/ip-address.md)
- With the Raspberry Pi on and running, open an SSH terminal program such as [PuTTY](http://www.putty.org/) on your desktop machine.
- Use the IP address from step 4 as the Host name, Port=22, and Connection type=SSH to complete the connection.
- When prompted, log in with username `pi`, and password `raspberry`.

#### Logging in using Linux or Mac OS

- See: [SSH Using Linux of Mac OS](https://www.raspberrypi.org/documentation/remote-access/ssh/unix.md)

## Introducing Sensors to your Device

If you haven’t already, you can head over to the [Adafruit BME280 sensor setup page](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test).

## Update Solution with Actual Sensor Data

Now we can download the Azure IoT SDK Remote Monitoring device sample solution and incorporate the data from the sensors and send it up to our Azure Remote Monitoring solution.

Clone the repository by entering the following commands on
your Pi:
```
cd ~
git clone git://<Azure SDK RM Sample URI>
cd <Sample Folder>
```
Using what we learned from the section titled “Introducing Sensors to your Device”, we only need to update three major things:

- Include the required libraries
- Update the Sample Data with the actual sensor data
- Update the connection string to use the string we saved above


### Updating the Simulated Data with Actual Sensor Data

As you did in the [Adafruit BME280 sensor setup page](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test),
there are portions where you received the sensor data and set them to
variables. In this case, you will be doing the exact same thing. Head to the
portion where you see:
```
if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity))
```
Before this, you will need to update the values of your `DeviceID`, `Temperature`, and `Humidity`. Simply set the value for each of those in a similar way as done in the previous tutorial (depending on implementation), for
example:
```
Result = bme280_read(&temp, &humidity);

Thermostat->DeviceId = "RaspPi";
thermostat->Temperature = temp;
thermostat->Humididty = humidity;
```
### Updating the Connection String

- Use the arrow keys to navigate to the
following lines:

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

