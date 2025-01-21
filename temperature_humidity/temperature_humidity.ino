#include "DHT.h"

#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#define DHTTYPE DHT11
#define SD_SCK_PIN 18
#define SD_MISO_PIN 19
#define SD_MOSI_PIN 23
#define SD_CS_PIN 5

#define DHTPIN 33


const char* wifi_ssid = "PLAY_Swiatlowodowy_179A";
const char* wifi_password = "WaXcRcLuGj";






DHT dht(DHTPIN, DHTTYPE);

const long utcOffsetInSeconds = 3600;
int counter = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000);  // Update every 60 seconds



void listFiles(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break; // No more files

    for (int i = 0; i < numTabs; i++) {
      Serial.print("\t");
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listFiles(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size());
    }
    entry.close();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // List files on the SD card
  //File root = SD.open("/");
  //listFiles(root, 0);

  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  timeClient.begin();
}



void loop() {
  delay(4000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }
  Serial.printf("Temperature is: %f\n Humidity is %f\n", t, h);

  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  Serial.println(currentTime);

  currentTime.replace(":", "_");

  File file = SD.open("/" + currentTime + ".txt", FILE_WRITE);

  if (file) {
    Serial.println("Saving to file");
    file.printf("Current Temperature is: %f\nHumidity is %f\n", t, h);
  }

  file.close();
  counter+=1;

}
