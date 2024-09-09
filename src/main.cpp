#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define WIFI_NETWORK "Switch manager"
#define WIFI_PASSWORD "Switch@Manager"

#define PIN_SERVO 13
Servo myServo;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void connectToWiFi(){
  Serial.print("Connecting to Wifi");
  WiFi.mode(WIFI_AP);

    // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(WIFI_NETWORK, WIFI_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  myServo.attach(PIN_SERVO);
}

void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient client = server.available();

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /on") >= 0) {
              Serial.println("on");
              // turn the switch to ON
              myServo.write(0);
              delay(2000);
              myServo.write(90);
              delay(500);
            } else if (header.indexOf("GET /off") >= 0) {
              Serial.println("off");
              // turn the switch to OFF
              myServo.write(180);
              delay(2000);
              myServo.write(90);
              delay(500);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");                          
      
            client.println("<p><a href=\"/on\"><button class=\"button\">ON</button></a></p>");
           
            client.println("<p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>");
                                      
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}