// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/devkit-translator/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "OledDisplay.h"
#include "HTS221Sensor.h"

#define CLOUD_APP_URL "https://homelog-jp.azurewebsites.net/"
#include "http_client.h"

#define LOOP_DELAY 10000

#define APP_VERSION "ver=2.0"
#define ERROR_INFO "Ouch."

DevI2C *i2c;
HTS221Sensor *sensor;
float humidity = 0;
float temperature = 0;

enum STATUS
{
  Idle,
  Recording,
  Recorded,
  WavReady,
  Uploaded,
  SelectLanguage
};

static char webAppUri[192];
static char sensorString[128];

// The timeout for retrieving the result
static uint64_t result_timeout_ms;

// Indicate the processing status
static STATUS status = Idle;

// Indicate whether WiFi is ready
static bool hasWifi = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWiFi()
{
  Screen.print(2, "Connecting...");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

static void EnterIdleState(bool clean = true)
{
  status = Idle;
  if (clean)
  {
    Screen.clean();
  }
  Screen.print(0, "Hold B to talk");
}

static int HttpTriggerTranslator(const char *content)
{
  if (content == NULL)
  {
    Serial.println("Content not valid");
    return -1;
  }

  sprintf(webAppUri, "%sappend?%s", (char *)CLOUD_APP_URL, content);
  HTTPClient client = HTTPClient(HTTP_GET, webAppUri);
  client.set_header("append", content);
  Screen.print(2, "Sending...");
  const Http_Response *response = client.send();
  if (response != NULL && response->status_code == 200)
  {
    Screen.print(2, "Success!");
    return 0;
  }
  else
  {
    Screen.print(2, "Error :(");
    Serial.println(response->status_message);
    return -1;
  }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actions
static void DoIdle()
{
  if (digitalRead(USER_BUTTON_A) == LOW)
  {
    // Enter Select Language mode
    status = SelectLanguage;
    Screen.clean();
    Screen.print(0, "Press B Scroll");
  }
  else if (digitalRead(USER_BUTTON_B) == LOW)
  {
    // Enter the Recording mode
    Screen.clean();
    Screen.print(0, "Recording...");
    Screen.print(1, "Release to send\r\nMax duraion: \r\n1.5 sec");
    status = Recording;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  Screen.print(0, "HomeLogger");

  if (strlen(CLOUD_APP_URL) == 0)
  {
    Screen.print(2, "No Azure Func");
    return;
  }

  Screen.print(2, "Initializing...");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);

  Screen.print(3, " > Serial");
  Serial.begin(115200);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }

  i2c = new DevI2C(D14, D15);
  sensor = new HTS221Sensor(*i2c);
  // init the sensor
  sensor -> init(NULL);


  Screen.print(1, "HomeLog Ready...");
}

void loop()
{
  if (hasWifi)
  {
    switch (status)
    {
    case Idle:
      DoIdle();
      break;

    case Uploaded:

      break;

    }
  }
  sensor -> getHumidity(&humidity);
  sensor -> getTemperature(&temperature);
  sprintf(sensorString, "temp=%f&humid=%f", temperature, humidity);
  Serial.println(sensorString);
  HttpTriggerTranslator("pepe=xx&papo=yy");
  delay(LOOP_DELAY);
}
