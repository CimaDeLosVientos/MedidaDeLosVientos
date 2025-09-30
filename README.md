# MedidaDeLosVientos
Small IoT weather station based on an ESP32 S3 device. In this document we will explain the materials and setup of the device and compiler.

This project is based in the work explained here: https://randomnerdtutorials.com/build-an-all-in-one-esp32-weather-station-shield/

## Materials
This is the list of materials we used for this project:
- ESP32 S3 N16R8 (WROOM 1)
- SD card module
- BMP180 shield
- DHT sensor
- LDR sensor
- One 5 kOhm resistor
- SD card

## Arduino config
The IDE used to edit, compile and load code to the ESP32 board is the Arduino IDE. Some configurations need to be made in the IDE so that the IDE recognizes the board and its features correctly and the code compiles.

### Driver installation
If your windows does not automatically recognize the board when you plug it, you'll need to install the windows USB drivers. Follow this tutorial: https://randomnerdtutorials.com/install-esp32-esp8266-usb-drivers-cp210x-windows/

### Board configuration
Then, you must configure the Arduino IDE to be able to use ESP32 boards. Following this tutorial (https://randomnerdtutorials.com/install-esp32-esp8266-usb-drivers-cp210x-windows/) you may install the ESP32 boards configuration packages for Arduino. Don't worry if the example does not run yet, some extra configurations need to be done.

The board we used is not a standard module, but a generic development module. In the Arduino IDE, go to Tools > Board > esp32 > ESP32S3 Dev module and select it. Then, configure the following parameters as shown in the image in the Tools menu:

<img width="496" height="908" alt="Arduino_config" src="https://github.com/user-attachments/assets/975d95cf-511b-4b14-a4e5-9d20326f87bb" />

With this, example sketches for the ESP32 should run on your board.
### Library installation
In order to compile the weather station code you'll need to install the BMP, DHT and generic sensor libraries by Adafruit: 
- https://github.com/adafruit/Adafruit-BMP085-Library
- https://github.com/adafruit/DHT-sensor-library
- https://github.com/adafruit/Adafruit_Sensor

## Modules config
Pin assignments for each module are described in the next subsections:

### SD card reader
The SD card reader module uses the SPI protocol to communicate with the board. The pin assignment is as follows:
  - SCK - pin 12
  - MISO - pin 11
  - MOSI - pin 13
  - CS - pin 10
    
Take into account that the ESP32 board has four SPI interfaces, but two of them are already in use for the PSRAM and the WiFi module. The selected pins are for the default user SPI interface and could be overridden in case it's necessary for your project.
On another note, the SD card module we used expects a 5V input. However, the pin labelled as 5V in our board is configured as input by default. In order to convert it to a 5V output, one must solder a jumper in the board (near the pins 11 and 12)

Using a SD card adaptor, you can load the index.html file and the logo png file in the SD and plug it in the SD card reader module.

### BMP180 module
The BMP180 module uses I2C communication. This can be directed to any digital pin in the board. We connected them as follows:
- D1 (SCA) - pin 9
- D2 (SCL) - pin 8

### DHT module
The DHT sensor provides readings through a single wire proprietary communication protocol. This output is connected to pin 15 in our board.

### LDR
The LDR will be connected as a resistive divider in series with the 5KOhm resistor. The reading will be performed through an analog pin. In this case, we selected pin 3.

## Code customization
If you connected the modules correctly, the only step left is to customize the code with your WiFi SSID and password (lines 15 and 16 in the weather_station.ino file). Note that some ESP32 devices are not able to connect to 5GHz WiFi networks, so you may need to setup a 2.4GHz-only network in your router configuration.
You may uncomment some debug lines in the code to follow the booting and setup process.

## Accesing the server
Once the module has successfully initialized the sensors and connected to the WiFi network, you should be able to access the webpage with the sensor readings using any device connected to the same network as the ESP32 by opening a web browser and introducing in the search bar the IP address that was assigned to the ESP32 board. You should see something like this:
<img width="1629" height="754" alt="imagen" src="https://github.com/user-attachments/assets/1c22dd68-be04-46ec-9050-198135f9c780" />

An interesting configuration you may do in your router configuration would be to assign the ESP32 module a static IP address. This way, your router will show the webpage always in the same address and you don't need to look for the IP address that was assigned at boot each time you restart the ESP32 board. In the code, you can see in line 58 that the static IP address we chose is 192.168.0.36. You may change it to whatever value is free in your router configuration.

## System architecture
We created a small board to connect all the sensors together and be able to place them separated from the ESP32 in a greenhouse, for example. This small board has been composed by soldering the components in a through-hole breadboard. The schematic of this board is as follows:
![photo_2025-09-30_17-48-53](https://github.com/user-attachments/assets/2d9dbf0b-5b22-4c9d-a8c2-e28cb780e096)

The prototype of the whole system looks as this:
![photo_2025-09-30_17-48-48](https://github.com/user-attachments/assets/51872351-c7c4-40c1-bc58-b03e6aeb7d92)

