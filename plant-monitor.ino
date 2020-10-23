#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

constexpr byte humidity_sensor_pin = A1;
constexpr byte humidity_sensor_vcc = 6;
constexpr int baud_rate = 9600;
constexpr int second_ms = 1000;
constexpr int ten_seconds_ms = 10 * second_ms;

// Soil moisture range
// Min: 121 in air (dry)
// Max: 510 in glass of water

BH1750 lightMeter;

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
}

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
  float lux = lightMeter.readLightLevel();
  delay(ten_seconds_ms);
}
