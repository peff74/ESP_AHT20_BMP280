void BMP280_begin() {
  Wire.beginTransmission(0x77);
  Wire.write(0xD0);  // 0xBE -->  register for chip identification
  Wire.endTransmission();
  Wire.requestFrom(0x77, 1);
  uint8_t chip_ID = Wire.read();
  if (chip_ID == 0x58) {  // 0x58 --> BMP280
    Serial.println("BMP280 found");
  } else {
    Serial.println("Unbekannter Sensor.");
  }



  // Generate soft-reset
  Wire.beginTransmission(0x77);
  Wire.write(0xE0);  // 0xE0 --> Reset register
  Wire.write(0xB6);  // 0xB6 --> Reset value for reset register
  Wire.endTransmission();



  // Wait for copy completion NVM data to image registers
  uint8_t stat_Reg = 1;

  while (stat_Reg == 1) {
    Wire.beginTransmission(0x77);
    Wire.write(0XF3);  // 0XF3 --> Status register
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)0x77, (byte)1);
    stat_Reg = Wire.read();
    Serial.println(stat_Reg);
  }

  // See datasheet 4.2.2 Trimming parameter readout


  // Array for storing the read values
  uint16_t values[12];

  // Addresses of the registers with coefficients Data to be read
  uint8_t registers[] = { 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E };

  for (int i = 0; i < 12; i++) {
    Wire.beginTransmission(0x77);
    Wire.write(registers[i]);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)0x77, (byte)2);
    if (Wire.available() >= 2) {
      values[i] = Wire.read() << 8 | Wire.read();  // compose 16-bit value
    }
  }

  // Reverse the bytes for all 16-bit values (little endian / big endian)
  for (int i = 0; i < 12; i++) {
    values[i] = (values[i] >> 8) | (values[i] << 8);
  }

  // Assigning values to the variables
  _dig_T1 = values[0];
  _dig_T2 = values[1];
  _dig_T3 = values[2];
  _dig_P1 = values[3];
  _dig_P2 = values[4];
  _dig_P3 = values[5];
  _dig_P4 = values[6];
  _dig_P5 = values[7];
  _dig_P6 = values[8];
  _dig_P7 = values[9];
  _dig_P8 = values[10];
  _dig_P9 = values[11];


  // Debug message: Output of each byte (binary) with labelling
  /***********************************************************/
  // Serial.println("Temperature Values:");
  // Serial.print("_dig_T1: ");
  // Serial.println(_dig_T1, HEX);
  // Serial.print("_dig_T2: ");
  // Serial.println(_dig_T2, HEX);
  // Serial.print("_dig_T3: ");
  // Serial.println(_dig_T3, HEX);

  // Serial.println("Pressure Values:");
  // Serial.print("_dig_P1: ");
  // Serial.println(_dig_P1, HEX);
  // Serial.print("_dig_P2: ");
  // Serial.println(_dig_P2, HEX);
  // Serial.print("_dig_P3: ");
  // Serial.println(_dig_P3, HEX);
  // Serial.print("_dig_P4: ");
  // Serial.println(_dig_P4, HEX);
  // Serial.print("_dig_P5: ");
  // Serial.println(_dig_P5, HEX);
  // Serial.print("_dig_P6: ");
  // Serial.println(_dig_P6, HEX);
  // Serial.print("_dig_P7: ");
  // Serial.println(_dig_P7, HEX);
  // Serial.print("_dig_P8: ");
  // Serial.println(_dig_P8, HEX);
  // Serial.print("_dig_P9: ");
  // Serial.println(_dig_P9, HEX);
  /***********************************************************/




  // Set in sleep mode to provide write access to the “config” register
  Wire.beginTransmission(0x77);
  Wire.write(0xF4);  // 0XF3 --> Contol register
  Wire.write(0b00);  // 00   --> sleep mode
  Wire.endTransmission();

  
  // SAMPLING_NONE = 0b000
  // SAMPLING_X1   = 0b001
  // SAMPLING_X2   = 0b010
  // SAMPLING_X4   = 0b011
  // SAMPLING_X8   = 0b100
  // SAMPLING_X16  = 0b101
  
  // MODE_SLEEP  = 0b00
  // MODE_FORCED = 0b01
  // MODE_NORMAL = 0b11

  Wire.beginTransmission(0x77);
  Wire.write(0xF4);
  uint8_t configValues = ((0b001 << 5) | (0b011 << 2) | 0b11);
  //                       temp           press         mode
  Wire.write(configValues);
  Wire.endTransmission();

  delay(10);

  // Set register 0xF5 “config”  ** See datasheet 5.4.6 for details”
  // STANDBY_MS_0_5  = 0b000
  // STANDBY_MS_10   = 0b110
  // STANDBY_MS_20   = 0b111
  // STANDBY_MS_62_5 = 0b001
  // STANDBY_MS_125  = 0b010
  // STANDBY_MS_250  = 0b011
  // STANDBY_MS_500  = 0b100
  // STANDBY_MS_1000 = 0b101

  // FILTER_OFF = 0b000
  // FILTER_X2 = 0b001
  // FILTER_X4 = 0b010
  // FILTER_X8 = 0b011
  // FILTER_X16 = 0b100
  
  Wire.beginTransmission(0x77);
  Wire.write(0xF5);
  configValues = (0b110 << 5) | (0b100 << 2);
  //              standby        filter
  Wire.write(configValues);
  Wire.endTransmission();



  // Wait for first completed conversion
  delay(100);

  // Debug message: Output    of   CTRL_MEAS &  CONFIG (binary)
  /***********************************************************/
  // readAndDisplayRegister(0x77, 0xF4, "CTRL_MEAS");
  // readAndDisplayRegister(0x77, 0xF5, "CONFIG");
  /***********************************************************/
}



void readTemperatureBMP280() {
  int32_t var1, var2, adc_T;

  // Read temperature registers
  Wire.beginTransmission(0x77);
  Wire.write(0xFA);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)0x77, (byte)3);

  adc_T = (Wire.read() << 16) | (Wire.read() << 8) | Wire.read();
  adc_T >>= 4;

  // See datasheet 4.2.3 Compensation formulas
  var1 = ((((adc_T >> 3) - ((int32_t)_dig_T1 << 1))) * ((int32_t)_dig_T2)) >> 11;

  var2 = (((((adc_T >> 4) - ((int32_t)_dig_T1)) * ((adc_T >> 4) - ((int32_t)_dig_T1))) >> 12) * ((int32_t)_dig_T3)) >> 14;

  _t_fine = var1 + var2;

  float T = (((_t_fine * 5) + 128) >> 8);

  temperature_BMP280 = T / 100;
}



void readPressureBMP280() {
  int64_t var1;
  int64_t var2;
  int64_t p;
  int32_t adc_P;

  // Read temperature for t_fine
  readTemperatureBMP280();

  // Read pressure registers
  Wire.beginTransmission(0x77);
  Wire.write(0xF7);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)0x77, (byte)3);
  adc_P = (Wire.read() << 16) | (Wire.read() << 8) | Wire.read();

  adc_P >>= 4;

  // See datasheet 4.2.3 Compensation formulas
  var1 = ((int64_t)_t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_dig_P6;
  var2 = var2 + ((var1 * (int64_t)_dig_P5) << 17);
  var2 = var2 + (((int64_t)_dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)_dig_P3) >> 8) + ((var1 * (int64_t)_dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_dig_P1) >> 33;

  if (var1 == 0) {
    return;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)_dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)_dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_dig_P7) << 4);

  pressure = p / 25600;
}



void readAndDisplayRegister(uint8_t deviceAddress, byte registerAddress, const char* registerName) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, (byte)1);
  if (Wire.available()) {
    uint8_t registerValue = Wire.read();

    Serial.print(registerName);
    Serial.print(" (");
    Serial.print(registerAddress, HEX);
    Serial.print(") : 0b");

    for (int i = 7; i >= 0; i--) {
      Serial.print((registerValue & (1 << i)) ? '1' : '0');
    }

    Serial.println();
  } else {
    Serial.println("Fehler beim Lesen des Registers");
  }
}