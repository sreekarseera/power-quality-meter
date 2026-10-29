# Low-voltage verification

Use only isolated, current-limited low-voltage test signals within the ADS1115
input and ESP32 supply limits. Shared logic ground is required. ADS1115 VDD=3.3V,
GND=GND, SDA=GPIO8, SCL=GPIO9, ADDR=GND (0x48). Inputs A0/A1 must remain within
0..3.3V; the selected +/-4.096V ADC gain does not make over-supply inputs safe.
Bias small AC signals near 1.65V. This repository does not instruct unsupervised
mains wiring. Original supplied schematics are reference material, not a safety
certification or a substitute for checking the exact sensor variant.

1. Confirm I2C communication without input signals.
2. Apply a measured low-voltage sine with a DC midpoint, confirm no clipping.
3. Capture one timestamped window; compare frequency/RMS to reference equipment.
4. Record instrument, source amplitude, offset, frequency, sample count,
   duration and discrepancies. Do not fill missing values with examples.
5. Repeat at several amplitudes/frequencies and known phase offsets; document
   sampling interpolation and sensor phase uncertainty before any power claims.

No raw experimental dataset is included because no independently verified
machine-readable capture was located. Tests generate synthetic signals in memory.
