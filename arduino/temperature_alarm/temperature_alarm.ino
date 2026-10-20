#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---- Pins ----
constexpr uint8_t DHT_PIN = 2;
constexpr uint8_t BUZZER_PIN = 8;
// LCD (I2C): Mega SDA = 20, SCL = 21 | Uno SDA = A4, SCL = A5

// ---- LCD ----
constexpr uint8_t LCD_ADDRESS = 0x27;  // try 0x3F if the screen stays blank
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

// ---- Thresholds (degrees C) ----
constexpr float TEMP_HIGH = 28.0f;
constexpr float TEMP_LOW = 18.0f;

// ---- Timing (ms) ----
constexpr unsigned long READ_INTERVAL_MS = 2000;  // DHT11 needs >= 1 s between reads

DHT dht(DHT_PIN, DHT11);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
unsigned long lastReadMs = 0;

bool isOutOfRange(float temperature) {
    return temperature > TEMP_HIGH || temperature < TEMP_LOW;
}

const char* statusText(float temperature) {
    if (temperature > TEMP_HIGH) {
        return "ALARM: TOO HIGH";
    }
    if (temperature < TEMP_LOW) {
        return "ALARM: TOO LOW";
    }
    return "Status: OK";
}

void setBuzzer(bool on) {
    digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
}

// Writes text on one row and pads with spaces, so old characters are overwritten
void showLine(uint8_t row, const char* text) {
    char line[LCD_COLUMNS + 1];
    snprintf(line, sizeof(line), "%-16s", text);
    lcd.setCursor(0, row);
    lcd.print(line);
}

void showTemperature(float temperature) {
    char tempText[8];
    dtostrf(temperature, 4, 1, tempText);

    char line[LCD_COLUMNS + 1];
    snprintf(line, sizeof(line), "Temp: %s C", tempText);

    showLine(0, line);
    showLine(1, statusText(temperature));
}

void showSensorError() {
    showLine(0, "Sensor error");
    showLine(1, "Check wiring");
}

void setup() {
    Serial.begin(115200);
    pinMode(BUZZER_PIN, OUTPUT);
    setBuzzer(false);
    dht.begin();
    lcd.init();
    lcd.backlight();
    showLine(0, "Starting...");
}

void loop() {
    const unsigned long now = millis();
    if (now - lastReadMs < READ_INTERVAL_MS) {
        return;
    }
    lastReadMs = now;

    const float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println("ERROR,sensor");
        setBuzzer(false);
        showSensorError();
        return;
    }

    const bool alarm = isOutOfRange(temperature);
    setBuzzer(alarm);
    showTemperature(temperature);

    // One line per reading, easy to parse later (WebSerial dashboard)
    Serial.print("TEMP,");
    Serial.print(temperature, 1);
    Serial.print(",ALARM,");
    Serial.println(alarm ? 1 : 0);
}
