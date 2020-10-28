## Plant-monitor

### Controller
Arduino Nano 33 IoT (SAMD Arm-based)
FDQN: `arduino:samd:nano_33_iot`

### Soil moisture
YL-69 soil moisture sensor to check plant health
- Connect with D6 and A6
[Quick Arduino write-up](https://create.arduino.cc/projecthub/nekhbet/using-the-yl-39-yl-69-soil-humidity-sensor-with-arduino-968268)

### Light sensor
- Connect with I2C
[BH1750](https://www.adafruit.com/product/4681)
[Arduino C++ Library: "hp_BH1750"](https://github.com/Starmbi/hp_BH1750)

### Display
- Connect with SPI
[eInk display](https://www.adafruit.com/product/4086)
[Pinout guide](https://learn.adafruit.com/adafruit-eink-display-breakouts/pinouts)
[Arduino C++ Library: "Adafruit EPD"](https://github.com/adafruit/Adafruit_EPD)