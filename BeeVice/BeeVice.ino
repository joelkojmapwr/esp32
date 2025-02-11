#include "DHT.h"

#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <json_parser.h>


#define DHTTYPE DHT11
#define SD_SCK_PIN 18
#define SD_MISO_PIN 19
#define SD_MOSI_PIN 23
#define SD_CS_PIN 5

#define DHTPIN 33

#define secondsToMicroSeconds 1000000


String wifi_ssid;
String wifi_password;
RTC_DATA_ATTR const char* apiHost = "srv77343.seohost.com.pl";  // Replace with your API domain
RTC_DATA_ATTR const int apiPort = 443;



RTC_DATA_ATTR const long utcOffsetInSeconds = 3600;
RTC_DATA_ATTR int counter = 0;
RTC_DATA_ATTR WiFiUDP ntpUDP;
RTC_DATA_ATTR NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000);

RTC_DATA_ATTR int wakeUpSeconds[50];
RTC_DATA_ATTR u_short wakeUpTableLength = 0;

void logError(String message) {
  Serial.println(message);
}

void afterWakeUpSetup() {
  
}

String newRequest(String requestType, String body = ""){
  WiFiClientSecure client;
  client.setInsecure();
  Serial.println("Connecting to API");


  if (client.connect(apiHost, apiPort)) {
    Serial.println("Connected to API");

    client.println(requestType + " HTTP/1.1");      // Replace "/login" with your API endpoint
    client.println(String("Host: ") + apiHost);        // Host header
    client.println("Content-Type: application/json");  // JSON content type
    client.println("Connection: close");               // Close connection after response
    if (body != ""){
      client.print("Content-Length: ");                  // Calculate and send content length
      client.println(body.length());
      client.println();    // End of headers
      client.print(body);  // Send the body
    }

    // Read the server response
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readString();
        response += line;
      }
    }
    client.stop();  // Disconnect after receiving
    return response;
  } else {
    logError("Failed to connect to API server");
  }
  return "err";
}

void postSensorReads() {
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin();
  WiFi.begin(wifi_ssid, wifi_password);
  timeClient.begin();
  Serial.print("Connecting to WiFi...");
  int waiting_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    waiting_counter++;
    if (waiting_counter > 30) {
      logError("Waited to long for WiFi to connect");
      return;
    }
  }
  Serial.println("Connected to WiFi");

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    logError("Failed to read temperature and/or humidity");
    return;
  }

  Serial.printf("Temp is %f and humidity is %f\n", t, h);  
  String body = "{\"beeViceId\": 1, \"sensorType\": 1, \"value\": " + String(t) + "}";
  String response = newRequest("POST /sensorReads", body);
  Serial.print(response);
  body = "{\"beeViceId\": 1, \"sensorType\": 2, \"value\": " + String(h) + "}";
  response = newRequest("POST /sensorReads", body);
  Serial.print(response);
}

void print_wakeup_touchpad() {
  touch_pad_t touchPin = esp_sleep_get_touchpad_wakeup_status();

  switch (touchPin) {
    case 0: Serial.println("Touch detected on GPIO 4"); break;
    case 1: Serial.println("Touch detected on GPIO 0"); break;
    case 2: Serial.println("Touch detected on GPIO 2"); break;
    case 3: Serial.println("Touch detected on GPIO 15"); break;
    case 4: Serial.println("Touch detected on GPIO 13"); break;
    case 5: Serial.println("Touch detected on GPIO 12"); break;
    case 6: Serial.println("Touch detected on GPIO 14"); break;
    case 7: Serial.println("Touch detected on GPIO 27"); break;
    case 8: Serial.println("Touch detected on GPIO 33"); break;
    case 9: Serial.println("Touch detected on GPIO 32"); break;
    default: Serial.println("Wakeup not by touchpad"); break;
  }
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void fetchCommands() {

}

void callback() {
  //placeholder callback function
}

void sleepSetup() {
  timeClient.begin();
  if (timeClient.update() == 0) {
    logError("Failed to sync time");
    return;
  }
  //Configure Touchpad as wakeup source
  unsigned long secondsToSleep = 0;
  unsigned long currentTime = timeClient.getEpochTime() % 86400;  ///(((timeClient.getHours()  * 60 + timeClient.getMinutes()) * 60) + timeClient.getSeconds());
  Serial.println(timeClient.getFormattedTime());
  Serial.println("Current seconds " + String(currentTime));
  if (currentTime < wakeUpSeconds[0]){
    secondsToSleep = wakeUpSeconds[0] - currentTime;
  }
  else {
    for (int i=0; i<wakeUpTableLength-1; i++){
      if (wakeUpSeconds[i] < currentTime && wakeUpSeconds[i+1] > currentTime){
        // next wakeUp will be in i+1
        secondsToSleep = wakeUpSeconds[i+1] - currentTime;
        break;
      }
    }
  }
  if (secondsToSleep == 0){
    secondsToSleep = 86400 - currentTime + wakeUpSeconds[0];
  }
  Serial.println("Going to sleep now for " + String(secondsToSleep));
  touchAttachInterrupt(T3, callback, 40);
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(secondsToSleep * secondsToMicroSeconds);
  
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  if (wakeUpTableLength == 0){
    wakeUpSeconds[0] = 60 * 60 * 13;
    wakeUpSeconds[1] = 60 * 60 * 14;
    wakeUpTableLength = 2;
  }

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  File config = SD.open("/config/wifi.txt", FILE_READ);
  if (!config) {
    logError("faile to open file from SD card");
  } else {
    wifi_ssid = config.readStringUntil('\n');
    wifi_ssid.trim();
    wifi_password = config.readStringUntil('\n');
    wifi_password.trim();
  }
  config.close();

  afterWakeUpSetup();

  print_wakeup_reason();
  print_wakeup_touchpad();

  postSensorReads();
  sleepSetup();
}



void loop() {
  // do it every minute for now
  delay(1000 * 60);
}
