#include <Wire.h>


// BMP280
/*******************************/
float temperature_BMP280;
float pressure;


// Temperature variable
int32_t _t_fine;


// Trimming parameters
uint16_t _dig_T1;
int16_t _dig_T2;
int16_t _dig_T3;

uint16_t _dig_P1;
int16_t _dig_P2;
int16_t _dig_P3;
int16_t _dig_P4;
int16_t _dig_P5;
int16_t _dig_P6;
int16_t _dig_P7;
int16_t _dig_P8;
int16_t _dig_P9;
/*******************************/



//AHT20
/*******************************/
float temperature_AHT20;
float humidity;
bool sensor_started = false;
bool sensor_busy = false;
unsigned long measurementDelayAHT20 = 0;
/*******************************/



float delta = 0;
float minDelta = 10;
float maxDelta = 0;


//  Heartbeat
unsigned long HeartbeatMillis = 0;
const long Heartbeatinterval = 5000;




void setup() {
  Serial.begin(115200);
  Wire.begin();
  AHT20_begin();
  BMP280_begin();
  startMeasurementAHT20();
}

void loop() {

  checkbusyAHT20();
  getDataAHT20();

  unsigned long currentMillis = millis();
  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;


    //BMP280
    readTemperatureBMP280();
    Serial.print("Temperatur: ");
    Serial.print(temperature_BMP280);
    Serial.println(" C");

    readPressureBMP280();
    Serial.print("Druck: ");
    Serial.print(pressure);
    Serial.println(" hPa");


    // AHT20
    startMeasurementAHT20();

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Temperatur: ");
    Serial.print(temperature_AHT20);
    Serial.println(" C");



    // Calculate the delta between the temperature values AHT20 and BMP280
    delta = (temperature_BMP280 > temperature_AHT20) ? (temperature_BMP280 - temperature_AHT20) : (temperature_AHT20 - temperature_BMP280);

    if (delta < minDelta) {
      minDelta = delta;
    }
    if (delta > maxDelta) {
      maxDelta = delta;
    }
    Serial.print("Temperatur Delta : ");
    Serial.print(delta);
    Serial.print(" | Min Delta: ");
    Serial.print(minDelta);
    Serial.print(" | Max Delta: ");
    Serial.println(maxDelta);
  }
}
