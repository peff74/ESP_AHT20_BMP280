void AHT20_begin() {
  Wire.beginTransmission(0x38);
  Wire.write(0xBE);  // 0xBE --> init register for AHT2x
  Wire.endTransmission();
}

void startMeasurementAHT20() {
  Wire.beginTransmission(0x38);
  Wire.write(0xAC);  // 0xAC --> start measurement
  Wire.write(0x33);  // 0x33 --> not really documented what it does, but it's called MEASUREMENT_CTRL
  Wire.write(0x00);  // 0x00 --> not really documented what it does, but it's called MEASUREMENT_CTRL_NOP
  Wire.endTransmission();
  measurementDelayAHT20 = millis();
  sensor_started = true;
  sensor_busy = true;
}

void checkbusyAHT20() {
  if (millis() < measurementDelayAHT20) {
    measurementDelayAHT20 = millis();
  }

  if (sensor_started && sensor_busy && ((millis() - measurementDelayAHT20 >= 200))) {
    sensor_started = false;
    sensor_busy = false;
  }

  if (sensor_started && sensor_busy && ((millis() - measurementDelayAHT20 >= 80))) {
    Wire.requestFrom(0x38, 1);
    if (Wire.available()) {
      unsigned char c = Wire.read();
      if (!(c & 0x80)) {
        sensor_busy = false;
      }
    }
  }
}


void getDataAHT20() {
  if (sensor_started && !sensor_busy) {
    Wire.requestFrom(0x38, 7);  // Request 7 bytes of data

    unsigned char str[7] = { 0 };
    int index = 0;

    // Fault detection
    unsigned long timeoutMillis = 200;
    unsigned long startMillis = millis();



    while (Wire.available()) {
      str[index] = Wire.read();  // Receive a byte as character

      // Debug message: Output of each byte (binary) with labelling
      /***********************************************************/
      // Serial.print("Byte ");
      // Serial.print(index);
      // Serial.print(": ");
      // for (int i = 7; i >= 0; --i) {
      // 	Serial.print((str[index] >> i) & 1);
      // }
      // Serial.println();
      /***********************************************************/


      index++;



      // Fault detection
      if (millis() - startMillis > timeoutMillis) {
        Serial.println("Timeout while waiting for data from AHT20");
        return;
      }
    }
    if (index == 0 || (str[0] & 0x80)) {
      Serial.println("Failed to get data from AHT20");
      sensor_started = false;
      return;
    }

    // Check CRC
    uint8_t crc = 0xFF;
    for (uint8_t byteIndex = 0; byteIndex < 6; byteIndex++) {
      crc ^= str[byteIndex];
      for (uint8_t bitIndex = 8; bitIndex > 0; --bitIndex) {
        if (crc & 0x80) {
          crc = (crc << 1) ^ 0x31;
        } else {
          crc = (crc << 1);
        }
      }
    }
    if (crc != str[6]) {
      Serial.println("CRC check failed");
      sensor_started = false;
      return;
    }

    // Parse data
    float humi, temp;
    // Extract the raw data for humidity from the bytes
    unsigned long __humi = str[1];  // Byte 1: The first 8 bits of the raw data for humidity
    __humi <<= 8;                   // Move the bits 8 positions to the left
    __humi += str[2];               // Byte 2: Add the next 8 bits
    __humi <<= 4;                   // Move the bits 4 positions to the left
    __humi += str[3] >> 4;          // Byte 3: Add the last 4 bits (shifted to the right)

    // Debug message: Output of the value created after the bit shift (binary)
    /************************************************************************/
    // Serial.print("Humidity (raw): ");
    // for (int i = 19; i >= 0; --i) {
    // 	Serial.print((__humi >> i) & 1);
    // }
    // Serial.println();
    /************************************************************************/

    humi = (float)__humi / 1048576.0;
    humidity = humi * 100.0;

    // Extract the raw data for temperature from the bytes
    unsigned long __temp = str[3] & 0x0f;  // Byte 3: The last 4 bits of the raw data for the temperature
    __temp <<= 8;                          // Move the bits 8 positions to the left
    __temp += str[4];                      // Byte 4: Add the next 8 bits
    __temp <<= 8;                          // Move the bits to the left again by 8 positions
    __temp += str[5];                      // Byte 5: Add the last 8 bits


    // Debug message: Output of the value created after the bit shift (binary)
    /************************************************************************/
    // Serial.print("Temperature (raw): ");
    // for (int i = 19; i >= 0; --i) {
    // 	Serial.print((__temp >> i) & 1);
    // }
    // Serial.println();
    /************************************************************************/

    temp = (float)__temp / 1048576.0 * 200.0 - 50.0;

    temperature_AHT20 = temp;

    sensor_started = false;
  }
}
