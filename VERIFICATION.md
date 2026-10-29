# Verification — 29 October 2026

4 automated tests passed locally using Python 3.14.6:

    python3 -m unittest discover -s tests -v

Synthetic signal tests checked sine RMS/frequency, phase-shift power factor, DC input and rejected malformed/clipped captures.

No physical hardware validation or new instrument measurement is claimed.

ESP32 target compilation was not completed: installing the PlatformIO toolchain
exceeded free disk space on the Mac. The build was stopped and its temporary
package cache removed. The configured GitHub Actions firmware job has not been
run. Passing desktop tests does not prove the ESP32 build or physical operation.
