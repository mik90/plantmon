#include <Adafruit_EPD.h>
#include <Arduino.h>
#include <Wire.h>
#include <hp_BH1750.h>

constexpr byte humidity_sensor_pin = A1;
constexpr byte humidity_sensor_vcc = 6; // D6
constexpr int baud_rate = 9600;
constexpr int second_ms = 1000;
constexpr int ten_seconds_ms = 10 * second_ms;
constexpr int five_seconds_ms = 5 * second_ms;

constexpr auto EPD_CS = 10;
constexpr auto EPD_DC = 9;
constexpr auto SRAM_CS = 11;
constexpr auto EPD_RESET = 5;
constexpr auto EPD_BUSY = 7;

Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
hp_BH1750 lightMeter;

struct HumiditySensor {
  static void setup() {
    // Init the humidity sensor board
    pinMode(humidity_sensor_vcc, OUTPUT);
    digitalWrite(humidity_sensor_vcc, LOW);
  }
  static int read() {
    digitalWrite(humidity_sensor_vcc, HIGH);
    delay(500);
    const int value = analogRead(humidity_sensor_pin);
    digitalWrite(humidity_sensor_vcc, LOW);
    return 1023 - value;
  }
  static void write_output() {
    Serial.print("Humidity: ");
    const auto moisture_perc = get_moisture_percentage();
    Serial.print(moisture_perc);
    Serial.println("%");
    // Have to wait 10 seconds per humidity measurement
    delay(ten_seconds_ms);
  }
  // Soil moisture range
  static constexpr int measured_moisture_min = 120; // Measured in air (dry)
  static constexpr int measured_moisture_max = 510; // Measured in a glass of water

  static int get_moisture_percentage() {
    const auto unadjusted_range = HumiditySensor::read();
    // Limit values between the measaured range just to be sure
    const auto constrained_range =
        constrain(unadjusted_range, measured_moisture_min, measured_moisture_max);
    // Map the range to a percentage
    const auto moisture_percentage =
        map(constrained_range, measured_moisture_min, measured_moisture_max, 0, 100);

    return moisture_percentage;
  }
};

struct Display {
  static void draw_text(const char* text, uint16_t color) {
    display.setCursor(0, 0);
    display.setTextColor(color);
    display.setTextWrap(true);
    display.print(text);
  }

  static void setup() {
    // Display "hello world" for 5 seconds
    display.begin();
    display.clearBuffer();
    draw_text("Hello world", EPD_BLACK);
    display.display();
    delay(five_seconds_ms);
    display.clearBuffer();

    // Draw some lines
    for (int16_t i = 0; i < display.width(); i += 4) {
      display.drawLine(0, 0, i, display.height() - 1, EPD_BLACK);
    }

    for (int16_t i = 0; i < display.height(); i += 4) {
      display.drawLine(display.width() - 1, 0, 0, i, EPD_RED); // on grayscale this will be mid-gray
    }
    display.display();
  }
};

struct LightSensor {
  static void setup() {
    if (lightMeter.begin(BH1750_TO_GROUND)) {
      lightMeter.calibrateTiming();
    } else {
      Serial.println("Light meter wasn't ready!");
    }
  }

  static void maybe_write_output() {
    if (lightMeter.hasValue()) {
      const float lux = lightMeter.getLux();
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println("lux");
      lightMeter.start();
    }
  }
};

void setup() {

  Wire.begin(); // Init I2C

  while (!Serial) {
    // Wait for serial to be ready
  }
  delay(1000);

  Serial.begin(baud_rate);

  HumiditySensor::setup();
  LightSensor::setup();
  Display::setup(); // Display is written to during setup
}

void loop() {

  HumiditySensor::write_output();
  LightSensor::maybe_write_output(); // Output may not always be available
}
