# ⚡ ESP32-S3 Power Quality Recorder - Wiring & Breadboard Guide

**Project:** AC Mains Power Quality & Energy Monitor  
**Author:** Sreekar  
**Topic:** Measuring 230V AC Mains Voltage, Current & Power using ESP32-S3 and ADS1115  

---

## 📝 Project Overview & Why ESP32-S3?

In this project, I am building a digital power quality meter to measure household AC voltage (230V, 50Hz) and current drawn by home appliances (like a lamp or iron box), and calculate true RMS voltage, current, and power consumption.

> **Note on Microcontroller:**  
> I originally planned to use a Raspberry Pi Pico / Picoduino, but the board had power issues and wasn't getting detected by my computer. So I switched to the **ESP32-S3** board! The ESP32-S3 is great because it has plenty of speed (dual-core 240MHz), lots of memory, and built-in Wi-Fi so I can even make a simple web dashboard to see the live voltage graph on my phone later!

---

## ⚠️ Important Safety Rules (Working with AC Mains!)

Since we are measuring 230V AC mains electricity:
1. **Never touch any exposed wires** while the board is plugged into the wall socket.
2. The **ZMPT101B voltage transformer** gives **galvanic isolation**, meaning the high voltage mains side is physically isolated from the low voltage 3.3V/5V microcontroller side by magnetic coupling.
3. The **SCT-013 current clamp** simply clips around ONE insulated wire (Phase/Live wire only, not both wires together) using electromagnetic induction, so it is 100% contactless and safe to hook up!

---

## 🔌 Components List & How They Work

1. **ESP32-S3 Board**: The brain of the project. Runs the code and reads digital data from the ADC over I2C.
2. **ADS1115 16-Bit ADC Module**:
   - Built-in ESP32 ADCs can be noisy and non-linear.
   - The ADS1115 gives 16-bit precision ($2^{16} = 65,536$ levels), giving super smooth sine wave readings for AC calculations.
3. **ZMPT101B AC Voltage Sensor Module**:
   - Has a micro-transformer (Mutual Inductance $e = -M \frac{di}{dt}$) + on-board op-amp.
   - Steps down high AC voltage into a safe small AC wave centered around 1.65V / 2.5V DC offset.
4. **SCT-013-000 Non-Invasive Current Sensor (CT Clamp)**:
   - Works like a step-down current transformer (100A in the primary gives 50mA in the secondary).
   - We connect a burden resistor and a voltage divider (to add a DC bias) so the ADC can read positive and negative current swings safely.

---

## 📋 Breadboard Pinout & Wiring Table

### 1. Connecting ADS1115 ADC to ESP32-S3 (I2C Interface)

| ADS1115 Pin | ESP32-S3 Pin | Purpose / Notes |
| :--- | :--- | :--- |
| **VDD** | `3V3` | Powers the ADC at 3.3V (keeps I2C logic safe for ESP32) |
| **GND** | `GND` | Common Ground connection |
| **SCL** | `GPIO 9` | I2C Clock line |
| **SDA** | `GPIO 8` | I2C Data line |
| **ADDR** | `GND` | Sets the I2C address to `0x48` |
| **ALERT** | *(Not connected for now)* | Interrupt pin (can leave open) |

*Note: In Arduino code, we can define `Wire.begin(8, 9);` to set SDA to Pin 8 and SCL to Pin 9.*

---

### 2. Connecting Sensors to ADS1115 ADC Inputs

| Sensor Name | Sensor Output Pin | ADS1115 Input Pin | What it measures |
| :--- | :--- | :--- | :--- |
| **ZMPT101B Voltage Module** | `OUT` pin | `A0` | AC Voltage Waveform |
| **SCT-013 Current Sensor** | Signal out from bias circuit | `A1` | AC Current Waveform |
| **Ground** | `GND` | `GND` (Common) | Reference Ground |

---

## 📐 Simple Breadboard Diagram (ASCII Layout)

```text
       +---------------------------------------------+
       |               ESP32-S3 Board                |
       |  [USB-C]                                    |
       |                                             |
       |  [3V3] --------+-------------------------+  |
       |  [GND] --------|--------+                |  |
       |  [GPIO 8] -----|--------|---+ (SDA)      |  |
       |  [GPIO 9] -----|--------|---|---+ (SCL)  |  |
       +----------------|--------|---|---|-------+--+
                        |        |   |   |
                        |        |   |   |
                 +------v--------v---v---v------+
                 |          ADS1115 ADC         |
                 |  VDD   GND   SDA   SCL  ADDR |
                 |   |     |                |   |
                 |   |     +----------------+   |
                 |                              |
                 |   A0     A1     A2     A3    |
                 +---|------|-------------------+
                     |      |
                     |      +<--- [SCT-013 Current Sensor circuit]
                     |
                     +<---------- [ZMPT101B Voltage Sensor OUT]
```

---

## 🧪 Next Steps for Testing

1. **Step 1:** Plug in the ESP32-S3 to the computer via USB and upload an I2C scanner sketch to verify ADS1115 is detected at address `0x48`.
2. **Step 2:** Test ZMPT101B with a step-down 12V AC transformer first (for extra safety before connecting directly to 230V mains).
3. **Step 3:** Tune the blue potentiometer on the ZMPT101B board with a screwdriver so the output sine wave fits nicely between 0V and 3.3V without clipping.
4. **Step 4:** Write the sampling code in Arduino IDE to calculate:
   $$V_{rms} = \sqrt{\frac{1}{N} \sum_{i=1}^{N} (V_i - V_{midpoint})^2}$$
