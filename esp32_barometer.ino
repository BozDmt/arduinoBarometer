#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <bmp3.h>
#include <bmp3_defs.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPI.h>
// #include <dummy.h>
#include <ESPAsyncWebServer.h>
#include <esp_http_client.h>

#define CALIBRATE_ABSOLUTE_DIFFERENCE
#define SEALEVELPRESSURE_HPA (1013.25)
#define BMP_SCK 18
#define BMP_MISO 19
#define BMP_MOSI 23
#define BMP_CS 5

AsyncWebServer server(80);

float pressure, temp, altitude;

char ssid[11] = "VIVACOM_A1";
int wl_status = WL_IDLE_STATUS;
const char pass[9] = "dc8d34a5";

Adafruit_BMP3XX bmp;
WiFiClient client;
IPAddress gateway;

const char index_html[] PROGMEM= "{\"msg\": \"We just got a connection!\"}";

const char index_html1[] PROGMEM= R"rawliteral(
    <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Arduino barometer</title>
  </head>
  <body>
      <ul>
          <li>
              <p id="temp">
                  Temperature         
              </p>
          </li>
          <li>
              <p id="pressure">
                  Pressure
              </p>
          </li>
          <li>
              <p id="altitude">
                  Altitude
              </p>
          </li>
      </ul>
  </body>
  
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *req){
  req->send(404,"text/plain","Not Found MAN");
}

void setup() {
  Serial.begin(115200);

  if(!bmp.begin_I2C()){
    Serial.println("Couldn't find a valid sensor, check wiring");
    while(1);
  }

  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Authentication for VIVACOM_A1 failed. Attempting A2.");
    strcpy(ssid,"VIVACOM_A2");
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println('Issue with connection');
      return;
    }
  }
  WiFi.begin(ssid,pass);
  Serial.print("Connected to: ");
  Serial.println(WiFi.localIP());

  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200,"application/json",index_html);
  });

  server.onNotFound(notFound);

  server.begin();
  gateway = WiFi.gatewayIP();
}

void loop() {
  // if(!client.connected()){
  //   client.stop();
  //   while(true);
  // }
  // while(client.available()){
  //   // client.
  // }
  if(!bmp.performReading()){
    Serial.println("Failed to perform reading: (");
    return;
  }
  pressure = bmp.pressure / 100.0;
  temp = bmp.temperature;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  bmp.readPressure();
  bmp.readTemperature();
}
