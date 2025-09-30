/*
 * Cima de los Vientos - Proyecto Medida de los Vientos 
 * Basado en el diseño de Rui Santos en http://randomnerdtutorials.com
 */

// Load required libraries
#include <WiFi.h>
#include "SPI.h"
#include "SD.h"
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

// Replace with your network credentials
#define STA_SSID  "YourWiFiSSID"
#define STA_PASS "YourWiFiPassword"

// uncomment one of the lines below for whatever DHT sensor type you're using
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// GPIO the DHT is connected to
const int DHTPin = 15;
//intialize DHT sensor
DHT dht(DHTPin, DHTTYPE);

// create a bmp object
Adafruit_BMP085 bmp;

// Web page file stored on the SD card
File webFile; 

// Set potentiometer GPIO
const int potPin = 32;

const int LDRPin = 3;

// variables to store temperature and humidity
float tempC;
float tempF;
float humi;

// Variable to store the HTTP request
String header;

// Set web server port number to 80
WiFiServer server(80);

// SPI pins reassignment for SD card
int sck = 12;
int miso = 11;
int mosi = 13;
int cs = 10;

// Setup static IP data (for convenience, it's easier to assign a static IP for the ESP32 in the router configuration. This way, the access point for the webpage is always the same)
// IP data 192.168.0.36, 192.168.0.1, 255.255.255.0, 192.168.0.1
IPAddress c_ip(192,168,0,36);
IPAddress c_mask(192,168,0,1);
IPAddress c_subnet(255,255,255,0);
IPAddress c_dns(192,168,0,1);

void setup(){    
  // initialize serial port
  Serial.begin(115200); 

  // Serial.println("Test Print");
  sleep(1);

  // initialize DHT sensor
  dht.begin();

  // initialize BMP180 sensor
  if (!bmp.begin()){
    Serial.println("Could not find BMP180 or BMP085 sensor");
    while (1) {}
  }

  //Initialize SPI
  if(!SPI.begin(sck, miso, mosi, cs)){
      Serial.println("SPI setup Failed");
      return;
  }
  
  // initialize SD card
  if(!SD.begin(cs)){
      Serial.println("Card Mount Failed");
      return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  // initialize SD card
  //Serial.println("Initializing SD card...");
  if (!SD.begin()) {
      Serial.println("ERROR - SD card initialization failed!");
      return;    // init failed
  }
  WiFi.STA.begin();
  int n = WiFi.scanNetworks();

/*
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }  

  */

  // Connect to Wi-Fi network with SSID and password
  // Serial.print("Connecting to ");
  // Serial.println(STA_SSID);

  WiFi.config(c_ip, c_mask, c_subnet, c_dns);
  WiFi.setTxPower(WIFI_POWER_2dBm); // The antenna power is set to its minimum so that the connection is successful (the board does not behave correctly else)
  WiFi.begin(STA_SSID, STA_PASS);

  while (!WiFi.STA.linkUp()) {
    // Serial.println(WiFi.STA);
    delay(5000);
  }

  //while (WiFi.localIP() == "0.0.0.0") {
    //Serial.println("IP not assigned yet");
    //Serial.println(WiFi.STA);
    //delay(5000);
  //}
  // Serial.println(WiFi.STA);

  // Print local IP address and start web server
  // Serial.println("");
  // Serial.println("WiFi connected.");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  //Serial.println(WiFi.STA);
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {  // if new client connects
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        header += c;
        // Serial.print(c); // Uncomment to see the webpage requests
        // if the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // Send XML file or Web page
          // If client already on the web page, browser requests with AJAX the latest
          // sensor readings (ESP32 sends the XML file)
          if (header.indexOf("update_readings") >= 0) {
            // send rest of HTTP header
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            // Send XML file with sensor readings
            sendXMLFile(client);
          }
          // Send the logo for the webpage
          else if(header.indexOf("Logo-CimaDeLosVientos.png") >= 0){
            // Serial.println("GET request caught");
            client.println("Content-Type: image/png;");
            client.println("Connection: keep-alive");
            client.println();
            // send web page stored in microSD card
            webFile = SD.open("/Logo-CimaDeLosVientos.png");
            if (webFile) {
              while(webFile.available()) {
                // send web page to client
                client.write(webFile.read()); 
              }
              webFile.close();
            }          
          }
          // When the client connects for the first time, send it the index.html file
          // stored in the microSD card
          else {  
            // send rest of HTTP header
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Connection: keep-alive");
            client.println();
            // send web page stored in microSD card
            webFile = SD.open("/index.html");
            if (webFile) {
              while(webFile.available()) {
                // send web page to client
                client.write(webFile.read()); 
              }
              webFile.close();
            }
          }
          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
        } // end if (client.available())
    } // end while (client.connected())
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    // Serial.println("Client disconnected.");
  } // end if (client)

  //delay(5000);
}

// Send XML file with the latest sensor readings
void sendXMLFile(WiFiClient cl){
  // Read DHT sensor and update variables
  readDHT();

  // Prepare XML file
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");

  cl.print("<reading>");
  cl.print(tempC);
  cl.println("</reading>");
  
  cl.print("<reading>");
  cl.print(tempF);
  cl.println("</reading>");
  
  cl.print("<reading>");
  cl.print(humi);
  cl.println("</reading>");
  
  float currentTemperatureC = bmp.readTemperature();
  cl.print("<reading>");
  cl.print(currentTemperatureC);
  cl.println("</reading>");
  float currentTemperatureF = (9.0/5.0)*currentTemperatureC+32.0;
  cl.print("<reading>");
  cl.print(currentTemperatureF);
  cl.println("</reading>");
  
  cl.print("<reading>");
  cl.print(bmp.readPressure());
  cl.println("</reading>");

  // IMPORTANT: Read the note about GPIO 4 at the pin assignment 
  cl.print("<reading>");
  cl.print(analogRead(LDRPin));
  cl.println("</reading>");
  
  cl.print("</inputs>");
}

void readDHT(){
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humi = dht.readHumidity();
  // Read temperature as Celsius (the default)
  tempC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  tempF = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humi) || isnan(tempC) || isnan(tempF)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  /*Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print(" %\t Temperature: ");
  Serial.print(tempC);
  Serial.print(" *C ");
  Serial.print(tempF);
  Serial.println(" *F");*/
}
