
# AHT20 / BMP280 sensor script for Arduino

 - This is an AHT20 / BMP280 temperature/pressure/humidity sensor script
   for Arduino.
 -  AHT20 + BMP280 chips are sold for less than a dollar
   on Ali.
  - This script only uses the Wire.h library, nothing else.
  - Works with ESP8266, ESP32 and similar



![AHT20_BMP280 logo](https://github.com/peff74/ESP_AHT20_BMP280/blob/main/AHT20_BMP280.jpg)


## Arduino script features

 -   Read temperature / humidity from AHT20
 -    Read temperature / pressure from BMP280
 - No long loop times when reading the AHT20
 - Simple and clearly written
 - So that even beginners (like me) can understand how it works 
 
## How does it work 
**AHT20**

The ATH20 can be reached via the address 0x38
So every query must begin with:

    Wire.beginTransmission(0x38);

To initiate the AHT20, 0xBE must be sent.
*void AHT20_begin()*

 
    Wire.write(0xBE);

To start the measurement 0xAC,  0x33, 0x00 must be sent.
*void startMeasurementAHT20()*

    Wire.write(0xAC);  
    Wire.write(0x33);  
    Wire.write(0x00);
    
Now you have to wait until the data is available.
ARAIR specifies a waiting time of 80ms. At the same time, the “Busy indication” bit must be monitored; if this is “0”, the measurement is finished.
*void checkbusyAHT20()*

    if (sensor_started && sensor_busy && ((millis() - measurementDelayAHT20 >= 80))) {
        Wire.requestFrom(0x38, 1);
        .
        .
        .
        
The 8 bytes of data can now be called up.
*void getDataAHT20()*

    Wire.requestFrom(0x38, 7);



  The bytes read in can now be processed as shown below

    +----------------------+
    | Byte 0: 00011000 |
    | Byte 1: 10010011 |
    | Byte 2: 10101100 |
    | Byte 3: 10010101 |
    | Byte 4: 00101010 |
    | Byte 5: 11000101 |
    | Byte 6: 10110011 |
    +----------------------+
    
        
    +----------------------------+----------------------------+
    | Humidity (raw)             | Temperature (raw)          |
    +----------------------------+----------------------------+
    | 10010011   10101100   1001 | 0101   00101010   11000101 |
    |   (B1)       (B2)     (B3) | (B3)     (B4)       (B5)   |
    +----------------------------+----------------------------+
    
    +----------------------------------+
    |          Berechnung Humi         |
    |  10010011101011001001 = 604873   |
    |  604873 / 1048576 * 100 = 57.69  |
    +----------------------------------+
    
    +-------------------------------------------+
    |          Berechnung Temp                  |
    |       01010010101011000101 = 338629       |
    |  338629 / 1048576 * 200.0 - 50.0 = 14.59  |
    +-------------------------------------------+
    
    
    Humidity: 57.69%. Temperature: 14.59



