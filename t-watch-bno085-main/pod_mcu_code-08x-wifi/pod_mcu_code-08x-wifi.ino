

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;



#define LILYGO_WATCH_2019_WITH_TOUCH
#include <LilyGoWatch.h>
TTGOClass *watch;
TFT_eSPI *tft;



#include <Wire.h>

#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
BNO080 myIMU;

#include "wifilogin.h"

String mac_address;


//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.0.196:1234/setValue";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 10;

String response;


void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      //webSocket.sendTXT("Connected");
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);

      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}


void setup() {
  Serial.begin(115200);

  delay(100); //  Wait for BNO to boot
  // Start i2c and BNO080
  Wire.flush();   // Reset I2C
  myIMU.begin(BNO080_DEFAULT_ADDRESS, Wire);
  Wire.begin(22, 21);

  // Get TTGOClass instance
  watch = TTGOClass::getWatch();

  // Initialize the hardware, the BMA423 sensor has been initialized internally
  watch->begin();

  // Turn on the backlight
  watch->openBL();

  //Receive objects for easy writing
  tft = watch->tft;

  // Some display settings
  tft->setTextColor(random(0xFFFF));
  tft->drawString("BNO08x Quaternions",  5, 50, 4);
  tft->setTextFont(4);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);


  //   WiFi.begin(ssid, password);


  WiFiMulti.addAP(ssid, password);

  Serial.println("Connecting");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  mac_address = WiFi.macAddress();

  delay(500);
  // server address, port and URL
  webSocket.begin("172.20.10.4", 3000, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  // webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  webSocket.sendTXT(String(millis()).c_str());

//  delay(100); //  Wait for BNO to boot
//  // Start i2c and BNO080
//  Wire.flush();   // Reset I2C
//  myIMU.begin(BNO080_DEFAULT_ADDRESS, Wire);
//  Wire.begin(22, 21);

  if (myIMU.begin() == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  myIMU.enableRotationVector(50); //Send data update every 50ms
  myIMU.enableLinearAccelerometer(50);

  Serial.println(F("Rotation vector enabled"));
  Serial.println(F("Output in form i, j, k, real, accuracy"));

}

void loop() {
  webSocket.loop();
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {

      float quatI;
      float quatJ;
      float quatK;
      float quatReal;
      float linAccX;
      float linAccY;
      float linAccZ;

      if (myIMU.dataAvailable() == true)
      {
        quatI = myIMU.getQuatI();
        quatJ = myIMU.getQuatJ();
        quatK = myIMU.getQuatK();
        quatReal = myIMU.getQuatReal();
        linAccX = myIMU.getLinAccelX();
        linAccY = myIMU.getLinAccelY();
        linAccZ = myIMU.getLinAccelZ();
        byte quatAccuracy = myIMU.getQuatAccuracy();
        byte linAccelAccuracy = myIMU.getLinAccelAccuracy();

        Serial.print(quatI, 2);
        Serial.print(F(" "));
        Serial.print(quatK, 2);
        Serial.print(F(" "));
        Serial.print(quatJ, 2);
        Serial.print(F(" "));
        Serial.print(quatReal, 2);
        Serial.print(F(" "));
        printAccuracyLevel(quatAccuracy);

        Serial.println();

        Serial.print(linAccX, 2);
        Serial.print(F(" "));
        Serial.print(linAccY, 2);
        Serial.print(F(" "));
        Serial.print(linAccZ, 2);
        Serial.print(F(" "));
        printAccuracyLevel(linAccelAccuracy);

        Serial.println();

        String url = "{\"id\": \"" + mac_address + "\",\"x\":" + quatI + ",\"y\":" + quatJ + ",\"z\":" + quatK + ",\"w\":" + quatReal + ",\"xAcc\":" + linAccX + ",\"yAcc\":" + linAccY + ",\"zAcc\":" + linAccZ + "}";
        Serial.println(url);
        webSocket.sendTXT(url.c_str());
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

//Given a accuracy number, print what it means
void printAccuracyLevel(byte accuracyNumber)
{
  if (accuracyNumber == 0) Serial.print(F("Unreliable"));
  else if (accuracyNumber == 1) Serial.print(F("Low"));
  else if (accuracyNumber == 2) Serial.print(F("Medium"));
  else if (accuracyNumber == 3) Serial.print(F("High"));
}
