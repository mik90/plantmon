#include <Adafruit_EPD.h>
#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

constexpr byte humidity_sensor_pin = A1;
constexpr byte humidity_sensor_vcc = 6;
constexpr int baud_rate = 9600;
constexpr int second_ms = 1000;
constexpr int ten_seconds_ms = 10 * second_ms;
constexpr int five_seconds_ms = 5 * second_ms;

constexpr auto EPD_CS = 10;
constexpr auto EPD_DC = 9;
constexpr auto SRAM_CS = 11;
constexpr auto EPD_RESET = 5;
constexpr auto EPD_BUSY = 7;

#define COLOR1 EPD_BLACK
#define COLOR2 EPD_RED

Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
BH1750 lightMeter;

void testdrawtext(char* text, uint16_t color);

void setup() {
  // Init the humidity sensor board
  pinMode(humidity_sensor_vcc, OUTPUT);
  digitalWrite(humidity_sensor_vcc, LOW);

  Wire.begin(); // Init I2C

  while (!Serial) {
    // Wait for serial to be ready
  }
  delay(1000);
  Serial.begin(baud_rate);
  lightMeter.begin();

  {
    // Display "hello world" for 5 seconds
    display.begin();
    display.clearBuffer();
    testdrawtext("Hello world", EPD_BLACK);
    display.display();
    delay(five_seconds_ms);
    display.clearBuffer();
  }

  {
    // Draw some lines
    for (int16_t i = 0; i < display.width(); i += 4) {
      display.drawLine(0, 0, i, display.height() - 1, EPD_BLACK);
    }

    for (int16_t i = 0; i < display.height(); i += 4) {
      display.drawLine(display.width() - 1, 0, 0, i, EPD_RED); // on grayscale this will be mid-gray
    }
    display.display();
  }
}

/**
 * Soil moisture range
 * Min: ~121 in air (dry)
 * Max: ~510 in glass of water
 * TODO map value to 0-100%
 */
int read_humidity_sensor() {
  digitalWrite(humidity_sensor_vcc, HIGH);
  delay(500);
  int value = analogRead(humidity_sensor_pin);
  digitalWrite(humidity_sensor_vcc, LOW);
  return 1023 - value;
}

void loop() {
  Serial.print("Humidity Level (0-1023): ");
  Serial.println(read_humidity_sensor());

  const float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.print("lux");

  delay(ten_seconds_ms);
}

void testdrawtext(const char* text, uint16_t color) {
  display.setCursor(0, 0);
  display.setTextColor(color);
  display.setTextWrap(true);
  display.print(text);
}
