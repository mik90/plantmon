### Plant-monitor
Prints out the soil moisture percentage and amount of light (lux) to an e-ink display

#### Controller
Arduino Nano 33 IoT (SAMD, Arm-based)  
FDQN: `arduino:samd:nano_33_iot`  
Arduino pinout available [here](https://content.arduino.cc/assets/Pinout-NANO33IoT_latest.pdf)

#### Soil moisture
YL-69 soil moisture sensor to check plant health  
Connect with D6 and A6  
[Quick Arduino write-up here](https://create.arduino.cc/projecthub/nekhbet/using-the-yl-39-yl-69-soil-humidity-sensor-with-arduino-968268)

#### Light sensor
Connect with I2C  
*TODO*: How to wire this up? There's not many pins left  
- Share ground w/ the soil moisture sensor?
- VIN from the eInk display
- SCL/SDL from Nano

BH1750 [product page](https://www.adafruit.com/product/4681)  
Asafruit pinout [guide](https://learn.adafruit.com/adafruit-bh1750-ambient-light-sensor/pinouts)  
Arduino C++ Library: [hp_BH1750](https://github.com/Starmbi/hp_BH1750)  

#### Display
Connect with SPI  
eInk display [product page](https://www.adafruit.com/product/4086)  
Pinout guide [here](https://learn.adafruit.com/adafruit-eink-display-breakouts/pinouts)  
Arduino C++ Library: [Adafruit EPD](https://github.com/adafruit/Adafruit_EPD)  
