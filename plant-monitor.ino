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
constexpr unsigned long display_write_interval_ms = 3 * minute_ms;

constexpr auto EPD_CS = 10;    // ECS: Chip select D10
constexpr auto EPD_DC = 9;     // D/C: Data/Command D9
constexpr auto SRAM_CS = 8;    // SRCS: SRAM Chip select D8
constexpr auto EPD_RESET = -1; // Share reset w/ Arduino
constexpr auto EPD_BUSY = 4;   // D4

Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
hp_BH1750 lightMeter;

/**
 * @brief Can only read from this sensor every 10 seconds
 */
class HumiditySensor {
private:
  /**
   * @brief blocks for 500ms
   */
  uint32_t read_raw() {
    digitalWrite(humidity_sensor_vcc, HIGH);
    delay(500);
    const uint32_t value = analogRead(humidity_sensor_pin);
    digitalWrite(humidity_sensor_vcc, LOW);
    return 1023U - value;
  }

public:
  void setup() {
    // Init the humidity sensor board
    pinMode(humidity_sensor_vcc, OUTPUT);
    digitalWrite(humidity_sensor_vcc, LOW);
  }
  // Soil moisture range
  static constexpr uint32_t measured_moisture_min = 120; // Measured in air (dry)
  static constexpr uint32_t measured_moisture_max = 510; // Measured in a glass of water

  /**
   * @brief Get moisture percentage if available
   * @details This isn't supposed to be read from more than every 10ms, but since dispaly intervals
   *          are every 3 minutes, it doesn't matter.
   */
  Optional<uint32_t> read() {
    const auto unadjusted_range = read_raw();
    // Limit values between the measaured range just to be sure
    const auto constrained_range =
        constrain(unadjusted_range, measured_moisture_min, measured_moisture_max);
    // Map the range to a percentage
    const auto moisture_percentage =
        map(constrained_range, measured_moisture_min, measured_moisture_max, 0, 100);

    Serial.println(String("HumiditySensor has new value:") + String(moisture_percentage));
    return Optional<uint32_t>(moisture_percentage);
  }
};

/**
 * @brief hp_BH1705 library already manages delay
 */
class LightSensor {
public:
  void setup() {
    delay(100); // Light sensor is a bit finicky with startup
    if (lightMeter.begin(BH1750_TO_GROUND)) {
      lightMeter.calibrateTiming();
      lightMeter.start();
    } else {
      Serial.println("Light meter wasn't ready!");
    }
  }

  Optional<float> read() {
    if (lightMeter.hasValue()) {
      const auto value = lightMeter.getLux();
      Serial.println(String("LightSensor has new value: ") + String(value));
      lightMeter.start();
      return Optional<float>(value);
    } else {
      Serial.println("LightSensor doesn't have new value!");
      return Optional<float>();
    }
  }
};

class Display {
private:
  /**
   * @brief Coordinates for the different outputs on the display
   *
   *  Looks somewhat like this:
   *  -----------------------------------------
   *  | Plantmon /                            |
   *  |                                       |
   *  | Moisture: 0%                          |
   *  |                                       |
   *  | Light: 0 lux                          |
   *  |                                       |
   *  -----------------------------------------
   *
   */

  static constexpr int16_t init_msg_display_x = 3;
  static constexpr int16_t init_msg_display_y = 3;

  static constexpr int16_t humidity_display_x = init_msg_display_x;
  static constexpr int16_t humidity_display_y = init_msg_display_y + 36;

  static constexpr int16_t light_display_x = humidity_display_x;
  static constexpr int16_t light_display_y = humidity_display_y + 36;

  uint32_t last_moisture_percentage_ = 0;
  float last_lux_ = 0.0f;
  const String loading_symbols{"|/-\\"};
  unsigned int cur_loading_symbol_idx = 0;
  const unsigned int max_loading_symbol_idx = loading_symbols.length() - 1;

  char get_next_loading_symbol() {
    if (cur_loading_symbol_idx > max_loading_symbol_idx) {
      // Don't overrun the loading symbols String[]
      cur_loading_symbol_idx = 0;
    }
    return loading_symbols.charAt(cur_loading_symbol_idx++);
  }

public:
  void setup() {
    Serial.println("Setting up display...");
    delay(second_ms);
    display.begin();
    display.clearBuffer();
    Serial.println("Display setup done");
  }

  /**
   * @brief Display gets updated here along with delaying for the recommended interval
   */
  void update(Optional<uint32_t> moisture_percentage, Optional<float> lux) {
    Serial.println("\nDisplay is ready to print");

    display.clearBuffer();
    Serial.println("Cleared buffer");
    display.setTextColor(EPD_BLACK);
    display.setTextWrap(true);
    display.setCursor(init_msg_display_x, init_msg_display_y);
    display.print(String("Plantmon ") + String(get_next_loading_symbol()));

    {
      display.setCursor(humidity_display_x, humidity_display_y);
      static bool first_moist_written = false;
      if (first_moist_written && !moisture_percentage.has_value()) {
        display.print(String("Moisture:   %"));
      } else {
        // A value can be written or a valid value is already there
        last_moisture_percentage_ = moisture_percentage.value_or(last_moisture_percentage_);
        display.print(String("Moisture: " + String(last_moisture_percentage_) + "%"));
        first_moist_written = true;
      }
    }

    {
      display.setCursor(light_display_x, light_display_y);

      static bool first_lux_written = false;
      if (first_lux_written && !lux.has_value()) {
        // On first iteration, don't print anything until we've measured something
        display.print(String("Light:      lux"));
      } else {
        // A value can be written or a valid value is already there
        last_lux_ = lux.value_or(last_lux_);
        display.print(String("Light: " + String(lux.value()) + " lux"));
        first_lux_written = true;
      }
    }

    Serial.println("Writing info to display...");
    display.display();
    Serial.println("Info written to display.\n");
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
using namespace mik;

void setup() {
  Wire.begin(); // Init I2C

  delay(second_ms);
  Serial.begin(baud_rate);

  Serial.println("Setting up sensors and display...");
  delay(second_ms);
  humidity_sensor.setup();
  Serial.println("Set up humidity sensor.");

  light_sensor.setup();
  Serial.println("Set up light sensor.");

  eink_display.setup(); // Display is cleared to during setup
  Serial.println("Set up display sensor.");

  Serial.println("setup done.");
}

void loop() {

  // Just update every available interval, life is simpler that way
  Serial.println(String("Waiting for display to be ready for writing..."));
  // TODO Update time left every 30 seconds
  constexpr auto loopIntervalMs = 30 * second_ms;
  // Wait for total of <display_write_interval_ms> ms
  auto loopsToWait = display_write_interval_ms / loopIntervalMs;

  while (loopsToWait > 0) {
    const auto timeLeftMs = loopsToWait * loopIntervalMs;
    const auto timeLeftSec = timeLeftMs / 1000;
    Serial.println(String(timeLeftSec) + String(" seconds left..."));
    delay(loopIntervalMs);
    loopsToWait -= 1;
  }

  Serial.println("Waiting done.");

  eink_display.update(humidity_sensor.read(), light_sensor.read());
}
