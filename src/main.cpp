// New timestamped acquisition firmware, 2026-10-29. Low-voltage bench inputs only.
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

Adafruit_ADS1115 adc;
constexpr size_t N = 256;
struct Reading { uint32_t tv, ti; float v, i; };
Reading readings[N];

void setup() {
  Serial.begin(115200);
  const auto started = millis();
  while (!Serial && millis() - started < 3000) delay(10);
  Wire.begin(8, 9);
  Wire.setTimeOut(100);
  if (!adc.begin(0x48, &Wire)) {
    Serial.println("# ERROR: ADS1115 missing at 0x48; acquisition halted");
    while (true) delay(1000);
  }
  adc.setGain(GAIN_ONE);
  adc.setDataRate(RATE_ADS1115_860SPS);
  Serial.println("# sensor-voltage capture only; no mains calibration assumed");
}

void loop() {
  const uint32_t origin = micros();
  for (size_t n=0; n<N; ++n) {
    uint32_t a=micros();
    int16_t rv=adc.readADC_SingleEnded(0);
    uint32_t b=micros();
    readings[n].tv=(a-origin)+(uint32_t)(b-a)/2;
    readings[n].v=adc.computeVolts(rv);
    a=micros();
    int16_t ri=adc.readADC_SingleEnded(1);
    b=micros();
    readings[n].ti=(a-origin)+(uint32_t)(b-a)/2;
    readings[n].i=adc.computeVolts(ri);
  }
  // Read-call midpoint timestamps estimate conversion times, not hardware timestamps.
  // Buffer first, print second so serial output doesn't interrupt the sampling window.
  Serial.println("t_v_us,sensor_v,t_i_us,sensor_i");
  for (const auto &r:readings)
    Serial.printf("%lu,%.7f,%lu,%.7f\n",(unsigned long)r.tv,r.v,
                  (unsigned long)r.ti,r.i);
  Serial.println("# END_WINDOW");
  delay(1000);
}
