# Network Relay
This project involves the remote control over a WLAN to make a switch that uses a webpage as a GUI. The core components are a web server and an ESP32 with a relay (without module board) to act as the switch.

The physical circuit uses few components, with a transistor allowing the ESP32 to control the relay. The setup is wired so that the relay is in the normally closed position as default meaning the system will seem absent when not powered.

The ESP32 is programmed to connect to WLAN then listen and respond appropriately to recieved packets. Testing using the Socket library on Python ensures functionality of the circuit, with similar code used for implementation with a web server.

A web server handles the sending of packets from user inputs on a website using a combination of HTML and PHP. This allows control from any device with a web browser. If port-forwarded, this switch could be controlled by anyone on the internet.
## The Circuit
![download](https://github.com/GDL109R/network-relay/assets/96452665/e40fa385-410a-4828-8ef0-3eb1f87ec540)
![download](https://github.com/GDL109R/network-relay/assets/96452665/4b788834-1104-4ad0-a47d-432a5b75dee4)
<img width="605" alt="download" src="https://github.com/GDL109R/network-relay/assets/96452665/50b746cd-4761-4f43-8472-9b70f0ab9bd5">

The use of a transistor compensates the ESP32 power limitation to drive a 5V relay. A use of a custom made USB cable gives power to the circuit, with many wires used being repurposed ethernet cable.
## ESP-32 Code

With the circuit doing most of the work for toggling the relay, a single command can change it's state. Using the Arduino IDE, the default blink program can be modified to use pin 13 as output creating a oscillating output:

```
// Simple toggle

// Pin connected to resistor/transistor
#define relayPin 13

void setup() {
  // Set mode of GPIO pin
  pinMode(relayPin, OUTPUT);
}

void loop() {
  // Toggle switch constantly with a 1000ms interval
  digitalWrite(relayPin, HIGH);
  delay(1000);
  digitalWrite(relayPin, LOW);
  delay(1000);
}
```

With confirmation that the circuit works, networking integration can begin. The first step is to connect to WLAN with SSID and key. Next, a serial connection is initialised over USB to allow viewing of paket data; recieved from setting up listening on a port:

```
// Libraries Required
#include <WiFi.h>
#include "AsyncUDP.h"

AsyncUDP udp;

// Replace with your own network credentials
const char* ssid = "SSID-HERE";
const char* password = "KEY-HERE";

void setup() {
  // Start serial connection
  Serial.begin(115200);
  
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

      // Reply to the client
      packet.printf("Got %u bytes of data", packet.length());
    });
}
}

void loop() {}
```

A Python script using the socket library can send a UDP packet to test this. Here's the packet generator code and the ESP-32's response:

```
import socket

# Setup packet information and confirm in console
UDP_IP = "192.168.1.200"
UDP_PORT = 1234
MESSAGE = "testMessage"

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

# Create a socket object then send formatted packet
sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
```

```
22:19:40.065 -> Connecting to SSID-HERE
22:19:40.582 -> .
22:19:40.582 -> WiFi connected
22:19:40.582 -> IP address: 
22:19:40.582 -> 192.168.1.200
22:19:40.582 -> UDP Listening on IP: 192.168.1.200
22:19:47.351 -> UDP Packet Type: Unicast, From: 192.168.1.76:56605, To: 192.168.1.200:1234, Length: 11, Data: testMessage
```
<img width="1028" alt="download" src="https://github.com/GDL109R/network-relay/assets/96452665/d2431a8c-8cad-4968-9ef4-6586257599bb">

The packet allows the simple transfer of a string that will allow custom instructions to be sent from the web server. A packet analyser (Wireshark) shows the raw data travelling over the network.
Conditional statements can now be added when a packet is recieved to link recieved data to appropriate responses.

```
// Convert packet data to string
String testString = String( (char*) packet.data());

// Use string for conditional statements to change relay state
if (testString == "on") {
digitalWrite(relayPin, LOW);
} else if (testString == "off") {
digitalWrite(relayPin, HIGH);
}
```

## Creating an interface

For this prototype, only a very simple webpage was used; the backend involves a php script that performs the same role as the above Python, where a single UDP packet is sent.

```
<!DOCTYPE html>

<!-- generic html -->
<html lang="en">
    <head>
        <!-- Page setup -->
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>Network Relay</title>
        
        <script src="https://code.jquery.com/jquery-1.10.2.js"></script>
    </head>

    <body>
        <!-- Basic GUI components -->
        <h1 style="font-family: monospace;">Network Relay</h1>
        <button onclick="start()">
            On
        </button>
        <button onclick="stop()">
            Off
        </button>

    </body>

    <!-- JS to take input and push php code to send local packet -->
    <script>
        function start() {
            $.ajax({
                type: "POST",
                url: "main.php",
                data: "phpFunction=start",
                success: function(msg){ 
                    console.log(msg);
                },
                error: function(msg){ 
                    console.log(msg);
                }
            });
        }

        function stop() {
            $.ajax({
                type: "POST",
                url: "main.php",
                data: "phpFunction=stop",
                success: function(msg){ 
                    console.log(msg);
                },
                error: function(msg){ 
                    console.log(msg);
                }
            });
        }
    </script>
</html>
```

The HTML code contains inline AJAX quieries to a php script, which performs the sending of the UDP packet to the specified address.

```
<?php
    // Check if correct syntax given
    if (!(isset($_POST['phpFunction']))) {
        return;
    }

    // Send message depending on variables recieved
    if ($_POST['phpFunction'] == "start") {
        send("on");
    } else {
        send("off");
    }

    // Create a socket and send UDP packet to IP:PORT with data arguement
    function send($data) {
        // Address specific to ESP-32
        $address = "192.168.1.200";
        $port = 1234;
        $sock = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
        $len = strlen($data);
        socket_sendto($sock, $data, $len, 0, $address, $port);
        socket_close($sock);

        // Send confirmation to console (debugging only)
        echo "SENT: " . $data;
    }
```

While the site may not be aesphetically pleasing, it is still functional on a range of devices due to it's simplicity:
<img width="587" alt="image" src="https://github.com/GDL109R/network-relay/assets/96452665/92478c41-2abf-43f2-9a4b-d027a8983949">

From here, the full system can be tested, the same UDP packets are sent as per Python, with the ESP-32 responding correctly to recieved packets creating a network controlled switch.

## Possible future expansions

-   Create a better GUI: while the interface is usable it is small on mobile devices, a better mobile design may aid usability.
    
-   Make WLAN not hard coded: Currently, the only way to change WiFi networks is to reprogram the ESP-32, finding an easier way to interact with it's settings allows for better maintenance.
    
-   Add more automated options: It may be helpful to connect the switch to a timer to setup when it should be active.
    
-   Better power input: Currently the system is powered using a USB cable, it would be best to integrate this into a single plug, or somehow hard wiring it into the appliance.
    
-   Increase security: Plaintext messages are currently being used, more secure communication over LAN can add better protection (and enforce authorisation) for device control.