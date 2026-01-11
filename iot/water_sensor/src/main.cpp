#include <Arduino.h>

// Configuration
const int SENSOR_PIN = 0;       // GPIO 0 (Check your board pinout!)
const float V_SUPPLY_MIN = 0.5; // Voltage at 0 PSI
const float V_SUPPLY_MAX = 4.5; // Voltage at Max PSI
const float P_MAX = 100.0;      // Max PSI
const float P_MIN = 0.0;        // Min PSI
const float ADC_REF_V =
    3.3;                  // ESP32 ADC Reference Voltage (adjust if calibrated)
const int ADC_RES = 4095; // 12-bit ADC

// Resistor divider values (Input -> R1 -> Pin -> R2 -> GND)
// Sensor Max Output = 4.5V. Target Max Input to ESP32 = 3.3V.
// Divisor ratio = R2 / (R1 + R2)
// Example: R1 = 2200, R2 = 4700. Ratio = 4700 / 6900 ~= 0.681.
// 4.5V * 0.681 ~= 3.06V (Safe)
const float R1 = 2200.0;
const float R2 = 4700.0;
const float VOLTAGE_DIVIDER_RATIO = R2 / (R1 + R2);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  } // Wait for serial port

  // ADC Resolution defaults to 12-bit on ESP32, but good to be explicit or
  // check docs
  analogReadResolution(12);

  Serial.println("ESP32-C6 Pressure Sensor Reader Started");
  Serial.print("Divider Ratio: ");
  Serial.println(VOLTAGE_DIVIDER_RATIO, 4);
}

void loop() {
  // 1. Read ADC
  int adcVal = analogRead(SENSOR_PIN);

  // 2. Convert to Pin Voltage
  // V_pin = (ADC / Max_ADC) * V_Ref
  float pinVoltage = (float)adcVal / ADC_RES * ADC_REF_V;

  // 3. Convert to Sensor Output Voltage (reverse voltage divider)
  // V_sensor = V_pin / Ratio
  float sensorVoltage = pinVoltage / VOLTAGE_DIVIDER_RATIO;

  // 4. Convert Voltage to Pressure
  // Linear mapping: P = (V_sensor - V_min) * (P_max / (V_max - V_min))
  // Note: V_max_sensor - V_min_sensor = 4.5 - 0.5 = 4.0V range

  float pressure = 0.0;

  // Clamp voltage to valid range to avoid negative pressure noise at 0
  if (sensorVoltage < V_SUPPLY_MIN) {
    pressure = 0.0;
  } else {
    pressure = (sensorVoltage - V_SUPPLY_MIN) *
               (P_MAX / (V_SUPPLY_MAX - V_SUPPLY_MIN));
  }

  // 5. Report
  Serial.print("ADC: ");
  Serial.print(adcVal);
  Serial.print(" | V_Pin: ");
  Serial.print(pinVoltage, 3);
  Serial.print("V | V_Sensor: ");
  Serial.print(sensorVoltage, 3);
  Serial.print("V | Pressure: ");
  Serial.print(pressure, 2);
  Serial.println(" PSI");

  delay(1000);
}
