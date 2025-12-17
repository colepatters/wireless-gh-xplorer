#include <Arduino.h>
#include <BleGamepad.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define LIS3DH_CLK 14
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 13
#define LIS3DH_CS 15

Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

struct Button {
    uint8_t pin;
    uint8_t id;
    bool lastState = true; // pull-up, so true = not pressed
};

Button buttons[] = {
    {27, BUTTON_1}, // green
    {26, BUTTON_2}, // red
    {25, BUTTON_3}, // yellow
    {33, BUTTON_4}, // blue
    {19, BUTTON_5}, // orange
    {18, BUTTON_6}, // strum down
    {5, BUTTON_7}, // strum up
    {22, BUTTON_8}, // start
    {21, BUTTON_9}, // select
};

BleGamepad bleGamepad("Guitar Hero Xplorer Wireless", "Cole Patterson");

void setup() {
    Serial.begin(115200);

    if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
        Serial.println("Couldn't connect to LIS3DH");
    }

    BleGamepadConfiguration bleGamepadConfig;

    bleGamepad.begin(&bleGamepadConfig);

    for (auto &btn : buttons) {
        pinMode(btn.pin, INPUT_PULLUP);
    }

    pinMode(32, INPUT); // whammy

    bleGamepad.setRightThumb(0);
}

void loop() {
    if (!bleGamepad.isConnected()) return;

    lis.read();
    sensors_event_t event;
    lis.getEvent(&event);

    float accelRaw = event.acceleration.x;
    accelRaw = constrain(accelRaw, -10.0, 10.0);
    accelRaw += 10.0;
    float accelMagnitude = accelRaw / 20.0;
    float accel = accelMagnitude * 32767;
    int accelInt = int(trunc(accel));

    int raw = analogRead(32);
    raw = constrain(raw, 290, 2270);
    
    int magnitude = raw - 290;
    int value = magnitude * 16;

    bleGamepad.setRightThumb(value, accelInt);

    for (auto &btn : buttons) {
        bool state = digitalRead(btn.pin);
        if (state != btn.lastState) {
            btn.lastState = state;
            if (state == LOW) {
                bleGamepad.press(btn.id);
            } else {
                bleGamepad.release(btn.id);
            }
        }
    }

    delay(5); // optional: reduce BLE traffic slightly
}

