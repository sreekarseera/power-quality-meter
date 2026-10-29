# Power Quality Meter

ESP32-S3 + ADS1115 acquisition and offline signal analysis. Supplied firmware
stages are preserved in originals/. The new firmware captures timestamped,
sequential channel readings; the Python tool aligns their times and computes
AC RMS, estimated active/apparent power, power factor, frequency and crest factor.

## Run

Install PlatformIO Core 6.1.18. From this directory:

    pio run
    pio device monitor
    python3 -m unittest discover -s tests -v
    python3 tools/analyze.py capture.csv

Upload only to the matching ESP32-S3 DevKitC-1 board with pio run -t upload.
Capture exactly one CSV window (the header through the last data row preceding
END_WINDOW). Timestamp columns reset at each window; concatenated windows are
rejected. Output comments beginning with # may remain in the capture file.

Default scales are 1: output is in sensor-voltage units, not calibrated mains
volts/amperes/watts. Supply independently measured positive calibration factors
with --voltage-scale and --current-scale. No calibration values were invented.
Use docs/bench-procedure.md before attaching any input.

## Limits

The ADS1115 multiplexes channels; it does not sample voltage/current
simultaneously. Interpolation cannot restore lost high-frequency detail or
remove sensor phase error. Midpoints of blocking ADC calls are approximate
timestamps. This is an educational meter, not a certified disturbance recorder
or harmonic analyzer. No successful hardware run of the new firmware is claimed.
The original firmware's 400-pair/200-ms comment is not carried into this build.

See docs/attribution.md and SOURCE_MANIFEST.json for provenance. The current
implementation was prepared on 29 October 2026; source dates are not Git history.
