#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Define I2C Pins for ESP32-S3 (Change if your board uses 21/22)
#define SDA_PIN 8
#define SCL_PIN 9

// Create an instance of the ADS1115 ADC
Adafruit_ADS1115 ads;

void setup() {
  // Initialize Serial Monitor at 115200 baud rate
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n--------------------------------------------------");
  Serial.println("⚡ ESP32-S3 + ADS1115 ADC Reader (Arduino C++)");
  Serial.println("--------------------------------------------------");

  // Initialize custom I2C pins for ESP32-S3
  Wire.begin(SDA_PIN, SCL_PIN);

  // Start ADS1115 at address 0x48 (ADDR connected to GND)
  if (!ads.begin(0x48, &Wire)) {
    Serial.println("❌ Failed to find ADS1115 chip! Please check your wiring.");
    while (1) { delay(10); } // Stop execution if chip not found
  }

  // Set gain to 1x (+/- 4.096V range, 1 LSB = 0.125mV)
  ads.setGain(GAIN_ONE);

  Serial.println("✅ ADS1115 Connected Successfully!");
  Serial.println("Reading Channel A0 (Voltage) and Channel A1 (Current)...\n");
}

void loop() {
  // Read raw 16-bit signed integer values from Channels 0 & 1
  int16_t raw_a0 = ads.readADC_SingleEnded(0); // ZMPT101B Voltage Sensor
  int16_t raw_a1 = ads.readADC_SingleEnded(1); // SCT-013 Current Sensor

  // Convert raw readings into actual Voltage (Volts)
  float volts_a0 = ads.computeVolts(raw_a0);
  float volts_a1 = ads.computeVolts(raw_a1);

  // Print formatted output to Serial Monitor
  Serial.print("CH A0 (Voltage): Raw = ");
  Serial.print(raw_a0);
  Serial.print("\t| Voltage = ");
  Serial.print(volts_a0, 4);
  Serial.print(" V  ||  ");

  Serial.print("CH A1 (Current): Raw = ");
  Serial.print(raw_a1);
  Serial.print("\t| Voltage = ");
  Serial.print(volts_a1, 4);
  Serial.println(" V");

  delay(500); // Read every 500ms
}

