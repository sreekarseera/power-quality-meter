/*
 * Project: Power Quality Recorder
 * File: 01_i2c_scanner.ino
 * Description: Simple I2C Scanner to test if ESP32-S3 communicates with the ADS1115 ADC.
 * Default ADS1115 Address with ADDR pin connected to GND: 0x48
 */

#include <Wire.h>

// ESP32-S3 Custom I2C Pins (can be changed to match your board)
#define I2C_SDA 8
#define I2C_SCL 9

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for Serial monitor to open
  }

  Serial.println("\n==========================================");
  Serial.println("  ESP32-S3 I2C Bus Scanner Tool");
  Serial.println("==========================================");
  Serial.printf("Initializing I2C on SDA=GPIO%d, SCL=GPIO%d...\n", I2C_SDA, I2C_SCL);

  Wire.begin(I2C_SDA, I2C_SCL);
}

void loop() {
  byte error, address;
  int deviceCount = 0;

  Serial.println("\nScanning I2C bus for connected modules...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf(" -> Found device at 7-bit address 0x%02X", address);
      if (address == 0x48) {
        Serial.print("  <-- ADS1115 ADC (Default Address: ADDR connected to GND)");
      } else if (address == 0x3C || address == 0x3D) {
        Serial.print("  <-- OLED Display (SSD1306)");
      }
      Serial.println();
      deviceCount++;
    } else if (error == 4) {
      Serial.printf(" -> Unknown error at address 0x%02X\n", address);
    }
  }

  if (deviceCount == 0) {
    Serial.println("❌ No I2C devices found!");
    Serial.println("Checklist:");
    Serial.println("  1. Is 3.3V connected to VDD on ADS1115?");
    Serial.println("  2. Is GND connected properly?");
    Serial.println("  3. Are SDA (GPIO8) and SCL (GPIO9) crossed or swapped?");
  } else {
    Serial.printf(" Scan complete. Found %d device(s).\n", deviceCount);
  }

  delay(5000); // Re-scan every 5 seconds
}
