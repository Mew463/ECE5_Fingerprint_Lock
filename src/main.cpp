#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include "SPIFFS.h"
#include "USB.h"
#include <ArduinoJson.h>

#include "fingerprint.h"
#include "Database_Handler.h"

const char *ssid = "RESNET-BROTECTED"; // Name of the WIFI network hosted by the device
const char *password = "marbry2025";   // Password

// const char* ssid = "EnVision-Local"; //Name of the WIFI network hosted by the device
// const char* password =  "thinkmakebreak";               //Password

#include <FastLED.h>
CRGB leds[6];

AsyncWebServer server(80);

const int overrideButton = 13; // (SW2 on colin's dev board)
const int unlockPin = 21;

enum states
{
  ENROLL,
  IDLE,
  UNLOCK
};

void setLeds(CRGB color)
{
  fill_solid(leds, 6, color);
  FastLED.show();
}

void flashColor(CRGB color)
{
  for (int i = 0; i < 3; i++)
  {
    setLeds(color);
    delay(125);
    setLeds(CRGB::Black);
    delay(125);
  }
}

states curState = IDLE;

database_handler myDataBase = database_handler();
fingerprint fpsensor = fingerprint();
int openId = 1;
String enrollingUser;

void webServerSetup()
{

  // This is a super simple page that will be served up any time the root location is requested.  Get here intentionally by typing in the IP address.
  //  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //    request->send(200, "text/html", "<!DOCTYPE html><html><head><title>Success</title></head><body><p>Hooray</p></body>");
  //    USBSerial.println("requested /");
  //  });

  // Default webpage for android devices
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    request->send(SPIFFS, "/index.html");
    USBSerial.println("requested /generate_204"); });

  // Route to load style.css file SUPER IMPORTANT!!
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });

  // Route to load java script file SUPER IMPORTANT!!
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/script.js", "text/javascript"); });

  server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    USBSerial.println("received an unlock!");
    request->redirect("/generate_204");
    curState = UNLOCK; });

  server.on("/enroll/users/", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String userToEnroll = request->getParam(0)->value();
    USBSerial.println("got request to enroll " + userToEnroll);
    openId = myDataBase.getNextAvailableLine();
    enrollingUser = userToEnroll;
    request->send(200, "text/plain", "Request to enroll received");
    curState = ENROLL; });

  server.on("/database_info", HTTP_GET, [](AsyncWebServerRequest *request) { // Return list of users
    DynamicJsonDocument doc(1024);

    JsonArray jsonArray = doc.to<JsonArray>();
    for (String user : myDataBase.users)
    {
      user.remove(user.length() - 1); // remove the carriage return character
      jsonArray.add(user);
    }

    String jsonString; // Serialize JSON to string
    serializeJson(doc, jsonString);

    request->send(200, "application/json", jsonString);
    request->redirect("/generate_204");
  });

  server.on("/delete/users/", HTTP_DELETE, [](AsyncWebServerRequest *request) { // Delete a user
    String userToDelete = request->getParam(0)->value();
    USBSerial.println("got request to delete " + userToDelete);
    fpsensor.deleteFingerprint(myDataBase.findLine(userToDelete));
    myDataBase.modifyEntry(myDataBase.findLine(userToDelete), "*");

    request->send(200, "text/plain", "User deleted successfully");
  });

  server.on("/deleteall", HTTP_DELETE, [](AsyncWebServerRequest *request) { // Delete a user
    USBSerial.println("got request to delete entire database!");
    fpsensor.emptyDatabase();
    myDataBase.clearDatabase();
    request->send(200, "text/plain", "Cleared database successfully!");
  });

  server.begin(); // Starts the server process
}

void setup()
{
  USBSerial.begin(115200);
  SPIFFS.begin(true);

  FastLED.addLeds<WS2812B, 33, GRB>(leds, 6).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(100);
  setLeds(CRGB::Green);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    USBSerial.print(".");
  }
  USBSerial.println("IP address: ");
  USBSerial.println(WiFi.localIP());
  server.begin();

  webServerSetup(); // Configures the behavior of the web server

  pinMode(unlockPin, OUTPUT);

  // File file = SPIFFS.open("/database.txt", "w");
  // file.println("id 1");

  // file.close();

  fpsensor.initialize();
  myDataBase.updateLines();
}

void loop()
{
  switch (curState)
  {
  case UNLOCK:
    digitalWrite(unlockPin, HIGH);
    for (int i = 0; i < 3; i++)
    {
      leds[i] = CRGB::Green;
      leds[5 - i] = CRGB::Green;
      FastLED.show();
      delay(100);
    }
    delay(500);
    for (int i = 2; i >= 0; i--)
    {
      leds[i] = CRGB::Black;
      leds[5 - i] = CRGB::Black;
      FastLED.show();
      delay(100);
    }
    digitalWrite(unlockPin, LOW);
    curState = IDLE;
    break;
  case IDLE:
    setLeds(CRGB::Black);
    switch (fpsensor.verify())
    {
    case SUCCESS:
      curState = UNLOCK;
      break;
    case FAILED:
      flashColor(CRGB::Red);
      break;
    case WAITING:
      break;
    }

    break;
  case ENROLL:
    static int currentEnrollStep = 1;
    static int retries = 0;
    static unsigned long lastStep = millis();
    if (millis() - lastStep > 11000) // Crude way to make sure lastStep get initialized to current time when ENROLL is first called
      lastStep = millis();

    int enrollStatus = fpsensor.enroll(currentEnrollStep, openId);
    if (enrollStatus == SUCCESS)
    {
      if (currentEnrollStep == 1 || currentEnrollStep == 3)
      { // Only flash green for some steps
        for (int i = 0; i < 3; i++)
        {
          setLeds(CRGB::Green);
          delay(125);
          setLeds(CRGB::Black);
          delay(125);
        }
      }
      lastStep = millis();
      currentEnrollStep++;
    }
    else if (enrollStatus == FAILED)
    {
      currentEnrollStep = 1;
      retries++;
      lastStep = millis();
      setLeds(CRGB::Red);
      delay(500);
    }
    else if (enrollStatus == WAITING)
    {
      if (currentEnrollStep == 2)
      { // The step where user has to remove finger
        setLeds(CRGB::Black);
      }
      else
      {
        static int displayIndex = 0;
        EVERY_N_MILLISECONDS(175)
        {
          displayIndex++;
          if (displayIndex > 5)
            displayIndex = 0;
        }
        setLeds(CRGB::Black);
        leds[displayIndex] = CRGB::Orange;
        FastLED.show();
      }
    }
    if (retries > 3 || currentEnrollStep == 5 || millis() - lastStep > 10000)
    { // Timeout triggered after 10s
      if (currentEnrollStep == 5)
        myDataBase.modifyEntry(openId, enrollingUser); // Success!! We went through the gauntlet and can finally add it into our database
      else
        flashColor(CRGB::Red);
      lastStep = millis() - 10000;
      curState = IDLE;
      currentEnrollStep = 1;
      retries = 0;
    }
    break;
  }

  // delay(250); // so I can actually read the terminal holy crap
}