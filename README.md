# ESP_AHT20_BMP280

ESP32 / ESP8266 Arduino script for the AHT20 + BMP280 sensor combo — using only `Wire.h`, nothing else.

![AHT20_BMP280](https://github.com/peff74/ESP_AHT20_BMP280/blob/main/AHT20_BMP280.jpg)

[![Hits](https://hitscounter.dev/api/hit?url=https%3A%2F%2Fgithub.com%2Fpeff74%2FESP_AHT20_BMP280&label=Hits&icon=github&color=%23198754&message=&style=flat-square&tz=Europe%2FBerlin)](https://github.com/peff74/ESP_AHT20_BMP280)

---

## About

The AHT20 + BMP280 module combines a temperature/humidity sensor and a barometric pressure sensor on a single board. These modules are widely available for under a dollar and work great with ESP8266 and ESP32.

This script reads all three values — **temperature**, **humidity**, and **pressure** — without any third-party library. Just `Wire.h`.

---

## Features

- Reads temperature & humidity from the AHT20
- Reads temperature & pressure from the BMP280
- Non-blocking AHT20 measurement (no `delay()` in the loop)
- CRC check on AHT20 data
- Delta comparison between AHT20 and BMP280 temperature readings
- Simple, well-commented code — easy to follow for beginners

---

## Hardware

### What you need

- ESP32 or ESP8266
- AHT20 + BMP280 combo module (sold as a single board)

### Wiring

| Sensor Pin | ESP32       | ESP8266     |
|------------|-------------|-------------|
| VCC        | 3.3V        | 3.3V        |
| GND        | GND         | GND         |
| SDA        | GPIO 21     | GPIO 4 (D2) |
| SCL        | GPIO 22     | GPIO 5 (D1) |

> The default I²C pins are used. If your board uses different pins, pass them to `Wire.begin(SDA, SCL)` in `setup()`.

### I²C Addresses

| Sensor | Address |
|--------|---------|
| AHT20  | `0x38`  |
| BMP280 | `0x77`  |

---

## Getting Started

1. Clone or download this repository.
2. Place all three `.ino` files (`AHT20_BMP280.ino`, `AHT20.ino`, `BMP280.ino`) in a **single folder** named `AHT20_BMP280`.
3. Open `AHT20_BMP280.ino` in the Arduino IDE — it will automatically include the other files.
4. Select your board and port, then upload.

No library installation needed. `Wire.h` is part of the Arduino core.

---

## Serial Output

Open the Serial Monitor at **115200 baud**. Every 5 seconds you will see:

```
BMP280 found
Temperatur: 23.45 C
Druck: 1013.25 hPa
Humidity: 57.69 %
Temperatur: 23.12 C
Temperatur Delta : 0.33 | Min Delta: 0.33 | Max Delta: 0.33
```

---

## How It Works

### AHT20

The AHT20 is accessed at I²C address `0x38`. The measurement is triggered once and the result is read back after a minimum of 80 ms. To avoid blocking the main loop, a simple state machine tracks whether a measurement has been started and whether the sensor is still busy.

**Init**
```cpp
Wire.write(0xBE);   // initialize register
```

**Start measurement**
```cpp
Wire.write(0xAC);   // trigger measurement
Wire.write(0x33);   // MEASUREMENT_CTRL
Wire.write(0x00);   // MEASUREMENT_CTRL_NOP
```

**Check busy bit** (Bit 7 of status byte — checked after ≥ 80 ms)
```cpp
Wire.requestFrom(0x38, 1);
if (!(byte & 0x80)) { /* data ready */ }
```

**Read 7 bytes**
```cpp
Wire.requestFrom(0x38, 7);
```

The raw sensor bytes are structured as follows:

```
+------------------+
| Byte 0: Status   |
| Byte 1: Humi MSB |
| Byte 2: Humi     |
| Byte 3: Humi/Tmp |
| Byte 4: Tmp      |
| Byte 5: Tmp LSB  |
| Byte 6: CRC      |
+------------------+

+----------------------------+----------------------------+
| Humidity (20 bit)          | Temperature (20 bit)       |
+----------------------------+----------------------------+
| B1[7:0]  B2[7:0]  B3[7:4] | B3[3:0]  B4[7:0]  B5[7:0] |
+----------------------------+----------------------------+
```

**Example raw bytes:**
```
+------------------+
| Byte 0: 00011000 |
| Byte 1: 10010011 |
| Byte 2: 10101100 |
| Byte 3: 10010101 |
| Byte 4: 00101010 |
| Byte 5: 11000101 |
| Byte 6: 10110011 |
+------------------+
```

**Conversion formulas:**
```
+----------------------------------+
|         Humidity calculation     |
|  10010011 10101100 1001 = 604873  |
|  604873 / 1048576 * 100 = 57.69 %|
+----------------------------------+

+--------------------------------------------+
|         Temperature calculation            |
|  0101 00101010 11000101 = 338629           |
|  338629 / 1048576 * 200.0 - 50.0 = 14.59 °C|
+--------------------------------------------+
```

---

### BMP280

The BMP280 is accessed at I²C address `0x77`.

**Step 1 — Verify chip ID**

Register `0xD0` holds the chip ID. The BMP280 always returns `0x58`.
```cpp
Wire.write(0xD0);
// expected response: 0x58
```

**Step 2 — Soft reset**

Writing `0xB6` to register `0xE0` resets the sensor to its power-on state.
```cpp
Wire.write(0xE0);  // reset register
Wire.write(0xB6);  // reset value
```
Afterwards, register `0xF3` (status) is polled until Bit 0 (`im_update`) clears, which means the NVM data has been copied to the internal image registers.

**Step 3 — Read trim (calibration) parameters**

The BMP280 stores 12 factory-calibrated 16-bit coefficients in registers `0x88`–`0x9E`. These are unique per chip and are required to convert the raw ADC values into real physical units.

```
Register | Parameter | Type
---------+-----------+----------
0x88     | dig_T1    | uint16_t
0x8A     | dig_T2    | int16_t
0x8C     | dig_T3    | int16_t
0x8E     | dig_P1    | uint16_t
0x90     | dig_P2    | int16_t
0x92     | dig_P3    | int16_t
0x94     | dig_P4    | int16_t
0x96     | dig_P5    | int16_t
0x98     | dig_P6    | int16_t
0x9A     | dig_P7    | int16_t
0x9C     | dig_P8    | int16_t
0x9E     | dig_P9    | int16_t
```

The values are stored little-endian (LSB first).

**Step 4 — Configure sensor**

Register `0xF4` (`ctrl_meas`) sets the oversampling for temperature and pressure, and the operating mode:
```cpp
// Bits [7:5] = temperature oversampling  → 0b001 = x1
// Bits [4:2] = pressure oversampling     → 0b011 = x4
// Bits [1:0] = mode                      → 0b11  = normal mode
0b00101111  →  0x2F
```

Register `0xF5` (`config`) sets the standby time between measurements and the IIR filter:
```cpp
// Bits [7:5] = standby time  → 0b110 = 10 ms
// Bits [4:2] = filter        → 0b100 = x16
0b11010000  →  0xD0
```

**Step 5 — Read raw temperature**

Registers `0xFA`, `0xFB`, `0xFC` hold the 20-bit raw ADC value:
```cpp
Wire.requestFrom(0x77, 3);
adc_T = (Byte0 << 16) | (Byte1 << 8) | Byte2;
adc_T >>= 4;  // right-shift by 4 → 20-bit value
```

Example bytes:
```
+------------------+
| Byte 0: 01101101 |  (0xFA)
| Byte 1: 01101100 |  (0xFB)
| Byte 2: 00000000 |  (0xFC)
+------------------+

Raw:  01101101 01101100 0000  →  adc_T = 448096
```

The raw value is then compensated using the trim parameters (BMP280 datasheet §4.2.3). The intermediate result `_t_fine` is stored globally and reused for the pressure compensation:
```
var1 = ((adc_T >> 3) - (dig_T1 << 1)) * dig_T2 >> 11
var2 = (((adc_T >> 4) - dig_T1)² >> 12) * dig_T3 >> 14

_t_fine = var1 + var2
Temperature = ((_t_fine * 5 + 128) >> 8) / 100   →  e.g. 23.45 °C
```

**Step 6 — Read raw pressure**

Registers `0xF7`, `0xF8`, `0xF9` hold the 20-bit raw ADC pressure value, read the same way as temperature:
```cpp
Wire.requestFrom(0x77, 3);
adc_P = (Byte0 << 16) | (Byte1 << 8) | Byte2;
adc_P >>= 4;  // right-shift by 4 → 20-bit value
```

The compensation formula uses `_t_fine` from the temperature calculation alongside the pressure trim parameters. The final result is in Pa, divided by 100 to give hPa:
```
p = compensated result in Pa
pressure = p / 256 / 100   →  e.g. 1013.25 hPa
```

> `readPressureBMP280()` already calls `readTemperatureBMP280()` internally to keep `_t_fine` up to date. If you only need pressure (and temperature as a by-product), calling `readPressureBMP280()` alone is sufficient — you do not need to call both explicitly.

---

## File Structure

```
AHT20_BMP280/
├── AHT20_BMP280.ino   # setup(), loop(), shared variables
├── AHT20.ino          # AHT20 init, measurement trigger, data read
└── BMP280.ino         # BMP280 init, temperature & pressure read
```

---

## License

MIT License — free to use, modify and distribute.
