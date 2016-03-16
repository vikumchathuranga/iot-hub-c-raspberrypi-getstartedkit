// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "serializer.h"
#include "threadapi.h"
#include "sastoken.h"
#include "iothub_client.h"
#include "iothubtransportamqp.h"
#include "iothub_client_ll.h"

// Sensor and Device Includes
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "bme280.h"
#include "locking.h"

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP

const int Spi_channel__i = 0;
const int Spi_clock__i = 1000000L;
int mcp3008Read(int Analog_in_channel__i);

const int Red_led_pin__i = 4;
const int Grn_led_pin__i = 5;

/*String containing Hostname, Device Id & Device Key in the format:             */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */
static const char* connectionString = "HostName=aaaaa.azure-devices.net;DeviceId=aaaaa;SharedAccessKey=aaaaaaaaaaaaaaaaaaaaaa==";

// Define the Model
BEGIN_NAMESPACE(WeatherStation);

DECLARE_MODEL(ContosoAnemometer,
WITH_DATA(ascii_char_ptr, DeviceId),
WITH_DATA(ascii_char_ptr, EventTime),
WITH_DATA(float, MTemperature),
WITH_ACTION(TurnFanOn),
WITH_ACTION(TurnFanOff),
WITH_ACTION(SetAirResistance, int, Position)
);

END_NAMESPACE(WeatherStation);

DEFINE_ENUM_STRINGS(IOTHUB_CLIENT_CONFIRMATION_RESULT, IOTHUB_CLIENT_CONFIRMATION_RESULT_VALUES)

EXECUTE_COMMAND_RESULT TurnFanOn(ContosoAnemometer* device)
{
	(void)device;
	(void)printf("Turning Green LED on.\r\n");
	digitalWrite(Red_led_pin__i, LOW);
	digitalWrite(Grn_led_pin__i, HIGH);
	return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT TurnFanOff(ContosoAnemometer* device)
{
	(void)device;
	(void)printf("Turning red LED on.\r\n");
	digitalWrite(Red_led_pin__i, HIGH);
	digitalWrite(Grn_led_pin__i, LOW);
	return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT SetAirResistance(ContosoAnemometer* device, int Position)
{
	(void)device;
	(void)printf("Setting Air Resistance Position to %d.\r\n", Position);
	return EXECUTE_COMMAND_SUCCESS;
}

void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	int messageTrackingId = (intptr_t)userContextCallback;

	(void)printf("Message Id: %d Received.\r\n", messageTrackingId);

	(void)printf("Result Call Back Called! Result is: %s \r\n", ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}


static void sendMessage(IOTHUB_CLIENT_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)
{
	static unsigned int messageTrackingId;
	IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
	if (messageHandle == NULL)
	{
		printf("unable to create a new IoTHubMessage\r\n");
	}
	else
	{
		if (IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void*)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
		{
			printf("failed to hand over the message to IoTHubClient");
		}
		else
		{
			printf("IoTHubClient accepted the message for delivery\r\n");
		}

		IoTHubMessage_Destroy(messageHandle);
	}
	free((void*)buffer);
	messageTrackingId++;
}

static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	IOTHUBMESSAGE_DISPOSITION_RESULT result;
	const unsigned char* buffer;
	size_t size;

	printf("Action Detected...\r\n");

	if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
	{
		printf("unable to IoTHubMessage_GetByteArray\r\n");
		result = EXECUTE_COMMAND_ERROR;
	}
	else
	{
		/*buffer is not zero terminated*/
		char* temp = malloc(size + 1);
		if (temp == NULL)
		{
			printf("failed to malloc\r\n");
			result = EXECUTE_COMMAND_ERROR;
		}
		else
		{
			memcpy(temp, buffer, size);
			temp[size] = '\0';
			EXECUTE_COMMAND_RESULT executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
			result =
				(executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
				(executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
				IOTHUBMESSAGE_REJECTED;
			free(temp);
		}
	}
	return result;
}


void simplesample_amqp_run(void)
{
	if (serializer_init(NULL) != SERIALIZER_OK)
	{
		(void)printf("Failed on serializer_init\r\n");
	}
	else
	{
		/* Setup IoTHub client configuration */
		IOTHUB_CLIENT_HANDLE iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, AMQP_Protocol);
		srand((unsigned int)time(NULL));
		int avgMTemperature = 10;

		if (iotHubClientHandle == NULL)
		{
			(void)printf("Failed on IoTHubClient_Create\r\n");
		}
		else
		{
			#ifdef MBED_BUILD_TIMESTAMP
			// For mbed add the certificate information
			if (IoTHubClient_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
			{
				(void)printf("failure to set option \"TrustedCerts\"\r\n");
			}
			#endif // MBED_BUILD_TIMESTAMP

			ContosoAnemometer* myWeather = CREATE_MODEL_INSTANCE(WeatherStation, ContosoAnemometer);
			if (myWeather == NULL)
			{
				(void)printf("Failed on CREATE_MODEL_INSTANCE\r\n");
			}
			else
			{
				unsigned char* destination;
				size_t destinationSize;

				if (IoTHubClient_SetMessageCallback(iotHubClientHandle, IoTHubMessage, myWeather) != IOTHUB_CLIENT_OK)
				{
					printf("unable to IoTHubClient_SetMessageCallback\r\n");
				}
				else
				{
					int Lock_fd__i = open_lockfile(LOCKFILE);
					if (setuid(getuid()) < 0)
					{
						perror("Dropping privileges failed. (did you use sudo?)\n");
						exit(EXIT_FAILURE);
					}

					int Result__i = wiringPiSetup();
					if (Result__i != 0) exit(Result__i);

					int Spi_fd__i = wiringPiSPISetup(Spi_channel__i, Spi_clock__i);
					if (Spi_fd__i < 0)
					{
						printf("Can't setup SPI, error %i calling wiringPiSPISetup(%i, %i)  %s\n",
							Spi_fd__i, Spi_channel__i, Spi_clock__i, strerror(Spi_fd__i));
						exit(Spi_fd__i);
					}

					int Init_result__i = bme280_init(Spi_channel__i);
					if (Init_result__i != 1)
					{
						printf("It appears that no BMP280 module on Chip Enable %i is attached. Aborting.\n", Spi_channel__i);
						exit(1);
					}

					pinMode(Red_led_pin__i, OUTPUT);
					pinMode(Grn_led_pin__i, OUTPUT);

					////////////////


					// Read the Temp & Pressure module.
					float Temp_C__f = -300.0;
					float Pressure_Pa__f = -300;
					float Humidity_pct__f = -300;
					Result__i = bme280_read_sensors(&Temp_C__f, &Pressure_Pa__f,
						&Humidity_pct__f);

					if (Result__i == 1)
					{
						printf("Temperature = %.1f *C  Pressure = %.1f Pa  Humidity = %1f %%\n",
							Temp_C__f, Pressure_Pa__f, Humidity_pct__f);
					}
					else
					{
						printf("Unable to read BME280 on pin %i\n", Spi_channel__i);
					}

					char buff[11];
					int timeNow = 0;

					int c;
					while (1)
					{
						timeNow = (int)time(NULL);

						sprintf(buff, "%d", timeNow);

						myWeather->DeviceId = "raspy";
						myWeather->EventTime = buff;

						if (Result__i == 1)
						{
							myWeather->MTemperature = Temp_C__f;
							printf("Humidity = %.1f%% Temperature = %.1f*C \n",
								Humidity_pct__f, Temp_C__f);
						}
						else
						{
							myWeather->MTemperature = 404.0;
							printf("Unable to read BME280 on pin %i\n", Spi_channel__i);
							pinMode(Red_led_pin__i, OUTPUT);
						}


						if (SERIALIZE(&destination, &destinationSize, myWeather->DeviceId, myWeather->EventTime, myWeather->MTemperature) != IOT_AGENT_OK)
						{
							(void)printf("Failed to serialize\r\n");
						}
						else
						{
							sendMessage(iotHubClientHandle, destination, destinationSize);
						}

						delay(5000);
					}
					/* wait for commands */
				  // (void)getchar();

					close_lockfile(Lock_fd__i);

				}
				DESTROY_MODEL_INSTANCE(myWeather);
			}
			IoTHubClient_Destroy(iotHubClientHandle);
		}
		serializer_deinit();
	}
}

int mcp3008Read(int Analog_in_channel__i)
{
	if ((Analog_in_channel__i > 7) || (Analog_in_channel__i < 0)) return -1;

	// Send the convert command and channel to the chip.
	// Simultaneously read back two bytes from the chip.
	const int Xfer_count__i = 3;
	unsigned char Buf__ca[Xfer_count__i];
	// Start bit.
	Buf__ca[0] = 0x01;
	// Bit 7 = single-ended, Bits 6-4 = channel.
	Buf__ca[1] = (0x80 + (Analog_in_channel__i << 4));
	Buf__ca[2] = 0;
	int Result__i = wiringPiSPIDataRW(Spi_channel__i, Buf__ca, Xfer_count__i);
	if (Result__i < 0) return Result__i;

	// Extract the relevant 10 bits.
	int Reading__i = ((Buf__ca[1] & 3) << 8) + Buf__ca[2];
	return Reading__i;
}

