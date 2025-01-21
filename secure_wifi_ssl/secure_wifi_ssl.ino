#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "Your_SSID";       // Replace with your WiFi SSID
const char* password = "Your_PASSWORD"; // Replace with your WiFi password
const char* apiHost = "your-api.com";   // Replace with your API domain
const int apiPort = 443;                // HTTPS port (default is 443)

WiFiClientSecure client; // Secure client for HTTPS connection

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Connect to the API
    Serial.println("Connecting to API...");
    if (client.connect(apiHost, apiPort)) {
        Serial.println("Connected to API server!");

        // Send the GET request
        client.println("GET /endpoint HTTP/1.1");          // Replace "/endpoint" with your API path
        client.println(String("Host: ") + apiHost);        // Host header
        client.println("Connection: close");              // Close connection after response
        client.println();                                 // End of headers

        // Read the server response
        Serial.println("Response from server:");
        while (client.connected() || client.available()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }

        client.stop(); // Disconnect after receiving the response
        Serial.println("Connection closed.");
    } else {
        Serial.println("Failed to connect to API server.");
    }
}

void loop() {
    // Empty loop
}
