// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef WINCE
#include "iothubtransportamqp.h"
#else
#include "iothubtransporthttp.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "sastoken.h"
#include "bme280.h"
#include "locking.h"

#include "schemalib.h"
#include "iothub_client.h"
#include "serializer.h"
#include "schemaserializer.h"
#include "threadapi.h"

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP

static const char* deviceId = "08578f25-777e-438b-876b-e2cfed911ab0";
static const char* deviceKey = "Vg7u5edztBGW98KwHWvsHw==";
static const char* hubName = "AuricIoTHubPiC";
static const char* hubSuffix = "azure-devices.net";

const int Spi_channel__i = 0;
const int Spi_clock__i = 1000000L;

const int Red_led_pin__i = 4;
const int Grn_led_pin__i = 5;


// Define the Model
BEGIN_NAMESPACE(Contoso);

DECLARE_STRUCT(SystemProperties,
ascii_char_ptr, DeviceID,
_Bool, Enabled
);

DECLARE_STRUCT(DeviceProperties,
ascii_char_ptr, DeviceID,
_Bool, HubEnabledState
);

DECLARE_MODEL(Thermostat,

/* Event data (temperature, external temperature and humidity) */
WITH_DATA(float, Temperature),
WITH_DATA(float, ExternalTemperature),
WITH_DATA(float, Humidity),
WITH_DATA(ascii_char_ptr, DeviceId),

/* Device Info - This is command metadata + some extra fields */
WITH_DATA(ascii_char_ptr, ObjectType),
WITH_DATA(_Bool, IsSimulatedDevice),
WITH_DATA(ascii_char_ptr, Version),
WITH_DATA(DeviceProperties, DeviceProperties),
WITH_DATA(ascii_char_ptr_no_quotes, Commands),

/* Commands implemented by the device */
WITH_ACTION(SetTemperature, int, temperature),
WITH_ACTION(SetHumidity, int, humidity)
);

END_NAMESPACE(Contoso);

EXECUTE_COMMAND_RESULT SetTemperature(Thermostat* thermostat, int temperature)

{
    (void)printf("Received temperature %d\r\n", temperature);
    thermostat->Temperature = temperature;
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT SetHumidity(Thermostat* thermostat, int humidity)

{
    (void)printf("Received humidity %d\r\n", humidity);
    thermostat->Humidity = humidity;
    return EXECUTE_COMMAND_SUCCESS;
}

static void sendMessage(IOTHUB_CLIENT_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)

{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    
    {
        printf("unable to create a new IoTHubMessage\r\n");
    }
    else
    
    {
        if (IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK)
        
        {
            printf("failed to hand over the message to IoTHubClient\n");
        }
        else
        
        {
            printf("IoTHubClient accepted the message for delivery\r\n");
        }
        
        IoTHubMessage_Destroy(messageHandle);
    }
    free((void*)buffer);
}

/*this function "links" IoTHub to the serialization library*/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)

{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    
    {
        printf("unable to IoTHubMessage_GetByteArrayrn");
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
            EXECUTE_COMMAND_RESULT executeCommandResult;
            
            memcpy(temp, buffer, size);
            temp[size] = '\0';
            executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result =
            (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
            (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
            IOTHUBMESSAGE_REJECTED;
            free(temp);
        }
    }
    return result;
}



void remote_monitoring_run(void)

{
    
    int Lock_fd__i = open_lockfile(LOCKFILE);
    if (setuid(getuid()) < 0)
    
    {
        perror("Dropping privileges failed. (did you use sudo?)n");
        exit(EXIT_FAILURE);
    }
    
    int Result__i = wiringPiSetup();
    if (Result__i != 0) exit(Result__i);
    
    int Spi_fd__i = wiringPiSPISetup(Spi_channel__i, Spi_clock__i);
    if (Spi_fd__i < 0)
    
    {
        printf("Can't setup SPI, error %i calling wiringPiSPISetup(%i, %i)  %sn",
        Spi_fd__i, Spi_channel__i, Spi_clock__i, strerror(Spi_fd__i));
        exit(Spi_fd__i);
    }
    

  int Init_result__i = bme280_init(Spi_channel__i);
  if (Init_result__i != 1)
  {
    printf("It appears that no BMP280 module on Chip Enable %i is attached. Aborting.\n", Spi_channel__i);
    exit(1);
  }

// Read the Temp & Pressure module.
float Temp_C__f = -300.0;
float Pressure_Pa__f = -300;
float Humidity_pct__f = -300;
Result__i = bme280_read_sensors(&Temp_C__f, &Pressure_Pa__f, &Humidity_pct__f);

if (Result__i == 1)
{
  printf("Temperature = %.1f *C  Pressure = %.1f Pa  Humidity = %1f %%\n",
    Temp_C__f, Pressure_Pa__f, Humidity_pct__f);
}
else
{
  printf("Unable to read BME280 on pin %i. Aborting.\n", Spi_channel__i);
  exit(1);
}

    if (serializer_init(NULL) != SERIALIZER_OK)
    {
        printf("Failed on serializer_initrn");
    }
    else
    {
        IOTHUB_CLIENT_CONFIG config;
        IOTHUB_CLIENT_HANDLE iotHubClientHandle;
        
        config.deviceId = deviceId;
        config.deviceKey = deviceKey;
        config.iotHubName = hubName;
        config.iotHubSuffix = hubSuffix;
        #ifndef WINCE
        config.protocol = AMQP_Protocol;
        #else
        config.protocol = HTTP_Protocol;
        #endif
        iotHubClientHandle = IoTHubClient_Create(&config);
        if (iotHubClientHandle == NULL)
        
        {
            (void)printf("Failed on IoTHubClient_CreateFromConnectionStringrn");
        }
        else
        
        {
            #ifdef MBED_BUILD_TIMESTAMP
            // For mbed add the certificate information
            if (IoTHubClient_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
            
            {
                printf("failure to set option "TrustedCerts"rn");
            }
            #endif // MBED_BUILD_TIMESTAMP
            
            Thermostat* thermostat = CREATE_MODEL_INSTANCE(Contoso, Thermostat);
            if (thermostat == NULL)
            
            {
                (void)printf("Failed on CREATE_MODEL_INSTANCErn");
            }
            else
            
            {
                STRING_HANDLE commandsMetadata;
                
                if (IoTHubClient_SetMessageCallback(iotHubClientHandle, IoTHubMessage, thermostat) != IOTHUB_CLIENT_OK)
                
                {
                    printf("unable to IoTHubClient_SetMessageCallbackrn");
                }
                else
                
                {
                    
                    /* send the device info upon startup so that the cloud app knows
                    what commands are available and the fact that the device is up */
                    thermostat->ObjectType = "DeviceInfo";
                    thermostat->IsSimulatedDevice = false;
                    thermostat->Version = "1.0";
                    thermostat->DeviceProperties.HubEnabledState = true;
                    thermostat->DeviceProperties.DeviceID = (char*)deviceId;
                    
                    commandsMetadata = STRING_new();
                    if (commandsMetadata == NULL)
                    
                    {
                        (void)printf("Failed on creating string for commands metadata\r\n");
                    }
                    else
                    
                    {
                        /* Serialize the commands metadata as a JSON string before sending */
                        if (SchemaSerializer_SerializeCommandMetadata(GET_MODEL_HANDLE(Contoso, Thermostat), commandsMetadata) != SCHEMA_SERIALIZER_OK)
                        
                        {
                            (void)printf("Failed serializing commands metadata\r\n");
                        }
                        else
                        
                        {
                            unsigned char* buffer;
                            size_t bufferSize;
                            thermostat->Commands = (char*)STRING_c_str(commandsMetadata);
                            
                            /* Here is the actual send of the Device Info */
                            if (SERIALIZE(&buffer, &bufferSize, thermostat->ObjectType, thermostat->Version, thermostat->IsSimulatedDevice, thermostat->DeviceProperties, thermostat->Commands) != IOT_AGENT_OK)
                            
                            {
                                (void)printf("Failed serializing\r\n");
                            }
                            else
                            
                            {
                                sendMessage(iotHubClientHandle, buffer, bufferSize);
                            }
                            
                        }
                        
                        STRING_delete(commandsMetadata);
                    }
                    
                    thermostat->Temperature = 50;
                    thermostat->ExternalTemperature = 55;
                    thermostat->Humidity = 50;
                    thermostat->DeviceId = (char*)deviceId;

                    while (1)
                    {
                        unsigned char*buffer;
                        size_t bufferSize;

			float Temp_C__f = -300.0;
			float Pressure_Pa__f = -300;
			float Humidity_pct__f = -300;

                        Result__i = bme280_read_sensors(&Temp_C__f, &Pressure_Pa__f, &Humidity_pct__f);

                        if (Result__i == 1)
                        {
                            thermostat->Temperature = Temp_C__f;
                            thermostat->Humidity = Humidity_pct__f;
                            printf("Humidity = %.1f%% Temperature = %.1f*C \n",
                            Humidity_pct__f, Temp_C__f);
                            pinMode(Grn_led_pin__i, OUTPUT);
                        }
                        else
                        {
                            thermostat->Temperature = 404.0;
                            thermostat->Humidity = 404.0;
			    printf("Unable to read BME280 on pin %i\n", Spi_channel__i);
                            pinMode(Red_led_pin__i, OUTPUT);
                        }

                        (void)printf("Sending sensor value Temperature = %.1f*C, Humidity = %.1f%%\r\n", thermostat->Temperature, thermostat->Humidity);

                        if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity, thermostat->ExternalTemperature) != IOT_AGENT_OK)
                        {
                            (void)printf("Failed sending sensor valuer\n");
                        }
                        else
                        {
                            sendMessage(iotHubClientHandle, buffer, bufferSize);
                        }

                        ThreadAPI_Sleep(1000);
                    }
                }
                close_lockfile(Lock_fd__i);
                DESTROY_MODEL_INSTANCE(thermostat);
            }
            IoTHubClient_Destroy(iotHubClientHandle);
        }
        serializer_deinit();
    }
}
