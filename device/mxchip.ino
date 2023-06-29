/*
#include "AzureIotHub.h"
#include "AzureIotHubClient.h"
#include "AzureIotHubClientCore.h"
#include "AzureIotHubDirectMethods.h"
#include "AzureIotHubDeviceTwin.h"
*/
#include "AZ3166WiFi.h"
#include "eventloop_timer_utilities.h"
#include "http_client.h"
#include "parson.h"
#include "sntp.h"
#include "wifi.h"

#define TEMPERATURE_SENSOR_PIN 0 // GPIO0 pin on MXChip AZ3166

static const char* ssid = "your_wifi_ssid";
static const char* password = "your_wifi_password";
static const char* url = "http://ppp.com/append";

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

static void SendDataToServer(double temperature, double pressure)
{
    char payload[256];
    snprintf(payload, sizeof(payload), "temperature=%.2f&pressure=%.2f", temperature, pressure);

    HTTPCLIENT_HANDLE httpClientHandle = HTTPClient_Create();
    HTTPCLIENT_RESULT result = HTTPClient_ExecuteRequest(
        httpClientHandle,
        HTTPCLIENT_REQUEST_GET,
        url,
        NULL,
        payload,
        strlen(payload),
        NULL,
        NULL,
        0);
    if (result != HTTPCLIENT_OK)
    {
        LogError("Failed to send data to server (HTTP error code %d)", result);
    }

    HTTPClient_Destroy(httpClientHandle);
}

static void SendTelemetryData(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle)
{
    double temperature = 0.0;
    double pressure = 0.0;

    // Code to read temperature and pressure values from sensors on MXChip
    // Replace the placeholders with the actual code for reading the data

    SendDataToServer(temperature, pressure);
}

static void SendMessageCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    LogInfo("Message sent to Azure IoT Hub");
}

/*
static void SetupAzureIoTHub()
{
    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    if (iotHubClientHandle == NULL)
    {
        LogError("Failed to create IoT Hub client handle");
        return;
    }

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed to set trusted certificates");
        return;
    }

    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, NULL);
}
*/

static void ConnectToWifi()
{
    if (WiFi.begin(ssid, password) != WL_CONNECTED)
    {
        LogError("Failed to connect to Wi-Fi");
        return;
    }

    LogInfo("Connected to Wi-Fi");

    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // Wait for time synchronization
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
    {
        delay(500);
    }

    //SetupAzureIoTHub();
}

void setup()
{
    Serial.begin(115200);

    InitWiFi();
    ConnectToWifi();
}

void loop()
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);

    // Read and send temperature and pressure data every 5 seconds
    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime > 10000)
    {
        SendTelemetryData(iotHubClientHandle);
        lastSendTime = millis();
    }
}
