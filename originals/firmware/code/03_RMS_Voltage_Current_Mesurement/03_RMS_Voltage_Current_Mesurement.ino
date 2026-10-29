#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_ADS1115 ads;

// CALIBRATION VALUES
float VOLTAGE_CALIBRATION = 769.0; // Calibrated for ZMPT101B voltage scaling
float CURRENT_CALIBRATION = 0.85;  // Calibrated for SCT-013 current clamp

const float V_OFFSET = 1.650; 
const float I_OFFSET = 1.651; 

float smoothV = 240.0;
float smoothI = 0.0;

// ⚡ NOISE CUTOFF THRESHOLD
// Any raw current below 0.05 A (12W) is treated as zero-load background noise
const float CURRENT_NOISE_THRESHOLD = 0.05; 

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ads.begin(0x48, &Wire)) {
    Serial.println("❌ ADS1115 Not Found!");
    while (1) { delay(10); }
  }

  ads.setGain(GAIN_ONE); 
  ads.setDataRate(RATE_ADS1115_860SPS); // Max sampling speed

  Serial.println("==================================================");
  Serial.println("⚡ Power Quality Monitor (V_CAL = 769.0 | I_CAL = 0.85)");
  Serial.println("==================================================\n");
}

void loop() {
  // 1. Sample Voltage (A0) over 100ms
  unsigned long startTime = millis();
  double sumSqV = 0.0;
  int countV = 0;

  while (millis() - startTime < 100) {
    int16_t rawA0 = ads.readADC_SingleEnded(0);
    float voltA0 = ads.computeVolts(rawA0);
    float acV = voltA0 - V_OFFSET;
    sumSqV += (acV * acV);
    countV++;
  }

  // 2. Sample Current (A1) over 100ms
  startTime = millis();
  double sumSqI = 0.0;
  int countI = 0;

  while (millis() - startTime < 100) {
    int16_t rawA1 = ads.readADC_SingleEnded(1);
    float voltA1 = ads.computeVolts(rawA1);
    float acI = voltA1 - I_OFFSET;
    sumSqI += (acI * acI);
    countI++;
  }

  // 3. RMS & Calibration Calculation
  float vSensorRMS = (countV > 0) ? sqrt(sumSqV / countV) : 0;
  float iSensorRMS = (countI > 0) ? sqrt(sumSqI / countI) : 0;

  float rawV_RMS = vSensorRMS * VOLTAGE_CALIBRATION;
  float rawI_RMS = iSensorRMS * CURRENT_CALIBRATION;

  // ⚡ CUT OFF NOISE FLOOR
  if (rawI_RMS < CURRENT_NOISE_THRESHOLD) {
    rawI_RMS = 0.0;
  }

  // Smoothing filter
  smoothV = (smoothV * 0.8) + (rawV_RMS * 0.2);
  smoothI = (smoothI * 0.8) + (rawI_RMS * 0.2);

  // If current smoothed to near zero, reset it completely to 0.0
  if (smoothI < 0.01) {
    smoothI = 0.0;
  }

  float apparentPower = smoothV * smoothI;

  Serial.print("AC Voltage: ");
  Serial.print(smoothV, 1);
  Serial.print(" V  |  AC Current: ");
  Serial.print(smoothI, 2);
  Serial.print(" A  |  Power: ");
  Serial.print(apparentPower, 1);
  Serial.println(" W");

  delay(300);
}