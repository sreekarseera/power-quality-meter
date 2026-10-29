#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

// I2C Pin Definitions for ESP32-S3
#define I2C_SDA 8
#define I2C_SCL 9

// Sampling configuration
#define SAMPLES_PER_CYCLE 40       // Approximate samples per 50Hz cycle
#define NUMBER_OF_CYCLES  10       // Sample across 10 AC cycles (200ms window)
#define TOTAL_SAMPLES     (SAMPLES_PER_CYCLE * NUMBER_OF_CYCLES)

// Calibration Factors (Tune these with a trusted multimeter)
// V_CALIBRATION: Multiplier to convert raw sensor voltage to real 230V mains RMS
float V_CALIBRATION = 766.0; 

// I_CALIBRATION: Multiplier to convert raw sensor reading to real Amperes RMS
float I_CALIBRATION = 0.85;

// Noise threshold (any current below this is considered zero to avoid phantom readings)
float NOISE_CURRENT_THRESHOLD = 0.05; // 50mA

Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("\n==================================================");
  Serial.println("   ⚡ ESP32-S3 POWER QUALITY RECORDER v1.0 ⚡");
  Serial.println("==================================================");

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!ads.begin(0x48, &Wire)) {
    Serial.println(" ERROR: ADS1115 ADC not detected! Check wiring.");
    while (1) {
      delay(1000);
    }
  }

  // GAIN_ONE gives +/- 4.096V range (0.125mV per bit)
  ads.setGain(GAIN_ONE);
  // Maximum sampling speed for ADS1115 (860 samples/sec)
  ads.setDataRate(RATE_ADS1115_860SPS);

  Serial.println(" ADS1115 Initialized successfully.");
  Serial.println("Starting measurements...\n");
}

void loop() {
  // Buffers for raw readings
  float sum_v_sq = 0;
  float sum_i_sq = 0;
  float sum_p = 0;
  
  float v_mean = 0;
  float i_mean = 0;

  int16_t raw_v_samples[TOTAL_SAMPLES];
  int16_t raw_i_samples[TOTAL_SAMPLES];

  // 1. Collect a burst of samples
  unsigned long start_time = micros();
  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    raw_v_samples[i] = ads.readADC_SingleEnded(0); // A0: Voltage
    raw_i_samples[i] = ads.readADC_SingleEnded(1); // A1: Current
    
    v_mean += raw_v_samples[i];
    i_mean += raw_i_samples[i];
  }
  unsigned long end_time = micros();

  // 2. Compute DC Offset (Midpoint)
  v_mean /= TOTAL_SAMPLES;
  i_mean /= TOTAL_SAMPLES;

  // 3. Remove DC Offset & compute True RMS and Instantaneous Power
  float max_v_instant = 0;
  int zero_crossings = 0;

  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    // Subtract DC bias to center wave around 0V
    float v_inst_volts = ads.computeVolts(raw_v_samples[i] - (int16_t)v_mean);
    float i_inst_volts = ads.computeVolts(raw_i_samples[i] - (int16_t)i_mean);

    // Apply calibration multipliers
    float v_actual = v_inst_volts * V_CALIBRATION;
    float i_actual = i_inst_volts * I_CALIBRATION;

    // Track peak voltage
    if (fabs(v_actual) > max_v_instant) {
      max_v_instant = fabs(v_actual);
    }

    // Accumulate squared terms for RMS
    sum_v_sq += (v_actual * v_actual);
    sum_i_sq += (i_actual * i_actual);

    // Accumulate instantaneous real power (P = v * i)
    sum_p += (v_actual * i_actual);

    // Count zero crossings for frequency estimation
    if (i > 0 && ((raw_v_samples[i-1] - v_mean) < 0 && (raw_v_samples[i] - v_mean) >= 0)) {
      zero_crossings++;
    }
  }

  // 4. Calculate Final Power Quality Parameters
  float v_rms = sqrt(sum_v_sq / TOTAL_SAMPLES);
  float i_rms = sqrt(sum_i_sq / TOTAL_SAMPLES);
  float active_power = sum_p / TOTAL_SAMPLES; // Real Power in Watts (W)

  // Filter out low-current noise
  if (i_rms < NOISE_CURRENT_THRESHOLD) {
    i_rms = 0.0;
    active_power = 0.0;
  }

  // Apparent Power (S = Vrms * Irms) in Volt-Amps (VA)
  float apparent_power = v_rms * i_rms;

  // Power Factor (PF = Real Power / Apparent Power)
  float power_factor = 0.0;
  if (apparent_power > 0.05) {
    power_factor = fabs(active_power / apparent_power);
    if (power_factor > 1.0) power_factor = 1.0; // clamp physical max
  }

  // Reactive Power (Q = sqrt(S^2 - P^2)) in VAR
  float reactive_power = 0.0;
  if (apparent_power > active_power) {
    reactive_power = sqrt((apparent_power * apparent_power) - (active_power * active_power));
  }

  // Frequency Estimation
  float sampling_duration_sec = (end_time - start_time) / 1000000.0;
  float frequency = 0.0;
  if (zero_crossings > 1 && sampling_duration_sec > 0) {
    frequency = (zero_crossings - 1) / sampling_duration_sec;
  }

  // Crest Factor (Peak / RMS) - Ideal Sine wave is ~1.414
  float crest_factor = (v_rms > 10.0) ? (max_v_instant / v_rms) : 0.0;

  // 5. Print Output to Serial Monitor
  printDashboard(v_rms, i_rms, active_power, apparent_power, reactive_power, power_factor, frequency, crest_factor);

  delay(1000); // Wait 1 second before next measurement cycle
}

void printDashboard(float v_rms, float i_rms, float active_power, float apparent_power, 
                    float reactive_power, float pf, float freq, float crest_factor) {
  Serial.println("--------------------------------------------------");
  Serial.println("  POWER QUALITY MEASUREMENTS SUMMARY");
  Serial.println("--------------------------------------------------");
  Serial.printf("  AC Mains Voltage (Vrms) : %.2f V\n", v_rms);
  Serial.printf("  AC Load Current (Irms)   : %.3f A (%.1f mA)\n", i_rms, i_rms * 1000.0);
  Serial.printf("  Real / Active Power (P)  : %.2f W\n", active_power);
  Serial.printf("  Apparent Power (S)       : %.2f VA\n", apparent_power);
  Serial.printf("  Reactive Power (Q)       : %.2f VAR\n", reactive_power);
  Serial.printf("  Power Factor (PF)        : %.3f\n", pf);
  if (freq > 30.0 && freq < 70.0) {
    Serial.printf("  Grid Frequency           : %.1f Hz\n", freq);
  } else {
    Serial.println("  Grid Frequency           : -- Hz");
  }
  Serial.printf("  Voltage Crest Factor     : %.3f (Ideal: 1.414)\n", crest_factor);
  Serial.println("--------------------------------------------------\n");
}
