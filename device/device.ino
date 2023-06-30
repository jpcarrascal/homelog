// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/devkit-translator/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "OledDisplay.h"
#include "SystemTickCounter.h"

#define AZURE_FUNCTION_URL "https://homelog-jp.azurewebsites.net/"
#include "http_client.h"

#define LANGUAGES_COUNT 9
#define MAX_RECORD_DURATION 1.5
#define MAX_UPLOAD_SIZE (64 * 1024)
#define LOOP_DELAY 10000
#define PULL_TIMEOUT 30000

#define APP_VERSION "ver=2.0"
#define ERROR_INFO "Ouch."

enum STATUS
{
  Idle,
  Recording,
  Recorded,
  WavReady,
  Uploaded,
  SelectLanguage
};

static char azureFunctionUri[192];

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

  sprintf(azureFunctionUri, "%sappend?%s", (char *)AZURE_FUNCTION_URL, content);
  HTTPClient client = HTTPClient(HTTP_GET, azureFunctionUri);
  client.set_header("append", "pepe=wf&papo=df");
  const Http_Response *response = client.send();

  if (response != NULL && response->status_code == 200)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback functions
static void ResultMessageCallback(const char *text, int length)
{
  Serial.printf("Enter message callback, text: %s\r\n", text);
  if (status != Uploaded)
  {
    return;
  }

  EnterIdleState();
  if (text == NULL)
  {
    Screen.print(1, ERROR_INFO);
    return;
  }

  char temp[33];
  int end = min(length, sizeof(temp) - 1);
  memcpy(temp, text, end);
  temp[end] = '\0';
  Screen.print(1, "Translation: ");
  Screen.print(2, temp, true);
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

static void DoUploader()
{
  if ((int)(SystemTickCounterRead() - result_timeout_ms) >= PULL_TIMEOUT)
  {
    // Timeout
    EnterIdleState();
    Screen.print(1, "Client Timeout");
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  Screen.print(0, "DevKitTranslator");

  if (strlen(AZURE_FUNCTION_URL) == 0)
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
      DoUploader();
      break;

    }
  }
  HttpTriggerTranslator("pepe=wf&papo=df");
  delay(LOOP_DELAY);
}
