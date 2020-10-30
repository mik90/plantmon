#include "Optional.hpp"
#include <Adafruit_EPD.h>
#include <Arduino.h>
#include <Wire.h>
#include <hp_BH1750.h>
namespace mik {

constexpr byte humidity_sensor_pin = A1;
constexpr byte humidity_sensor_vcc = 6; // D6
constexpr int baud_rate = 9600;
constexpr unsigned long second_ms = 1000;
constexpr unsigned long minute_ms = 60 * second_ms;
constexpr unsigned long ten_seconds_ms = 10 * second_ms;
constexpr unsigned long five_seconds_ms = 5 * second_ms;

constexpr auto EPD_CS = 10;
constexpr auto EPD_DC = 9;
constexpr auto SRAM_CS = 11;
constexpr auto EPD_RESET = 5;
constexpr auto EPD_BUSY = 7;

Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
hp_BH1750 lightMeter;

/**
 * @brief Can only read from this sensor every 10 seconds
 */
class HumiditySensor {
private:
  static constexpr long humidity_read_interval_ms = 500;
  /**
   * @brief blocks for 500ms
   */
  uint32_t read_raw() {
    digitalWrite(humidity_sensor_vcc, HIGH);
    delay(humidity_read_interval_ms);
    const uint32_t value = analogRead(humidity_sensor_pin);
    digitalWrite(humidity_sensor_vcc, LOW);
    return 1023U - value;
  }

  /// @brief Last time that the sensor was read using millis()
  unsigned long sensor_read_timestamp_ms_;

public:
  void setup() {
    // Init the humidity sensor board
    pinMode(humidity_sensor_vcc, OUTPUT);
    digitalWrite(humidity_sensor_vcc, LOW);
    // Technically didn't read from it, but meh
    sensor_read_timestamp_ms_ = 0;
  }
  // Soil moisture range
  static constexpr uint32_t measured_moisture_min = 120; // Measured in air (dry)
  static constexpr uint32_t measured_moisture_max = 510; // Measured in a glass of water

  /**
   * @brief Get moisture percentage if available
   */
  Optional<uint32_t> read() {
    if (millis() - sensor_read_timestamp_ms_ < ten_seconds_ms) {
      const auto unadjusted_range = read_raw();
      // Limit values between the measaured range just to be sure
      const auto constrained_range =
          constrain(unadjusted_range, measured_moisture_min, measured_moisture_max);
      // Map the range to a percentage
      const auto moisture_percentage =
          map(constrained_range, measured_moisture_min, measured_moisture_max, 0, 100);

      return Optional<uint32_t>(moisture_percentage);
    } else {
      // Sensor was read too recently
      return Optional<uint32_t>();
    }
  }
};

/**
 * @brief hp_BH1705 library already manages delay
 */
class LightSensor {
public:
  void setup() {
    if (lightMeter.begin(BH1750_TO_GROUND)) {
      lightMeter.calibrateTiming();
    } else {
      Serial.println("Light meter wasn't ready!");
    }
  }

  Optional<float> read() {
    if (lightMeter.hasValue()) {
      return Optional<float>(lightMeter.getLux());
    } else {
      return Optional<float>();
    }
  }
};

class Display {
private:
  static constexpr auto init_msg_display_x = 0;
  static constexpr auto init_msg_display_y = 0;
  static constexpr auto humidity_display_x = 0;
  static constexpr auto humidity_display_y = 36;
  static constexpr auto light_display_x = 0;
  static constexpr auto light_display_y = 72;
  static constexpr long display_interval_ms = 3 * minute_ms;
  /// @brief Last time that the display was written to using millis()
  unsigned long display_write_timestamp_ms_;

public:
  void setup() {
    display.begin();
    display.clearBuffer();

    display.setCursor(init_msg_display_x, init_msg_display_y);
    display.setTextColor(EPD_BLACK);
    display.setTextWrap(false);
    display.print("plantmon init");
    display.display();

    // Display text for 5 seconds
    delay(five_seconds_ms);

    display.clearBuffer();
    display.display();
    display_write_timestamp_ms_ = millis();
  }

  /**
   * @brief Display gets updated here along with delaying for the recommended interval
   */
  void update(Optional<uint32_t> moisture_percentage, Optional<float> lux) {
    bool wrote_to_display = false;

    if (moisture_percentage.has_value()) {
      display.setCursor(humidity_display_x, humidity_display_y);

      const auto text = String("Moisture: " + String(moisture_percentage.value()) + "%");
      display.print(text);
      wrote_to_display = true;
    }

    if (lux.has_value()) {
      display.setCursor(light_display_x, light_display_y);

      const auto text = String("Light: " + String(lux.value()) + " lux");
      display.print(text);
      wrote_to_display = true;
    }

    // Able to write once again
    if (millis() - display_write_timestamp_ms_ < display_interval_ms) {
      display.display();
      display_write_timestamp_ms_ = millis();
    }
  }
};

} // namespace mik

/**
 * -----------------------------------
 * Main Arduino logic
 * -----------------------------------
 */
mik::HumiditySensor humidity_sensor;
mik::LightSensor light_sensor;
mik::Display eink_display;

void setup() {
  using namespace mik;

  Wire.begin(); // Init I2C

  while (!Serial) {
    // Wait for serial to be ready
  }
  delay(second_ms);

  Serial.begin(baud_rate);

  humidity_sensor.setup();
  light_sensor.setup();
  eink_display.setup(); // Display is written to during setup
}

void loop() {
  using namespace mik;
  // Output may not always be available from the sensor, but they will be read from if so
  eink_display.update(humidity_sensor.read(), light_sensor.read());
  // Just delay regardless to avoid using more power than needed
  delay(five_seconds_ms);
}
