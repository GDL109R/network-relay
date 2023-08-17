
// Libraries Required
#include <WiFi.h>
#include "AsyncUDP.h"

AsyncUDP udp;

// Program settings
const char* ssid = "SSID-HERE";
const char* password = "KEY-HERE";
#define relayPin 13

void setup() {
  // General setup
  Serial.begin(11500);
  pinMode(relayPin, OUTPUT);
  
  // Define a static IP for the network
  IPAddress local_IP(192, 168, 1, 200);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  // Attempt to connect to WiFi with given information
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  // Confirm successful connection
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Example code to print packet contents
  if(udp.listen(1234)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();

      // Convert packet data to string
      String testString = String( (char*) packet.data());

      // Use string for conditional statements to change relay state
      if (testString == "on") {
        digitalWrite(relayPin, LOW);
      } else if (testString == "off") {
        digitalWrite(relayPin, HIGH);
      }

      // Reply to the client
      packet.printf("Got %u bytes of data", packet.length());
    });
}
}

void loop() {}
