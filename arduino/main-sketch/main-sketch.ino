#include <Arduino.h>
#include <BleGamepad.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLIS3DH.h>

#define LIS3DH_CLK 14
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 13
#define LIS3DH_CS 15

LIS3DH lis = LIS3DH(SPI_MODE, 15);

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

BleGamepad bleGamepad("GH3 Gibson", "Cole Patterson");

unsigned long previousMillis = 0;
const long interval = 250;

bool pairingLedState = true;

void setup() {
    // Serial.begin(115200);

    SPI.begin(LIS3DH_CLK, LIS3DH_MISO, LIS3DH_MOSI, LIS3DH_CS);

    uint8_t lisCode = 0;
    lisCode = lis.begin();

    if ((lisCode != 0x00) && (lisCode !=0xFF)) {
        // Serial.println("Couldn't connect to LIS3DH");
    }

    BleGamepadConfiguration bleGamepadConfig;

    bleGamepad.begin(&bleGamepadConfig);

    for (auto &btn : buttons) {
        pinMode(btn.pin, INPUT_PULLUP);
    }

    pinMode(32, INPUT); // whammy
    pinMode(23, OUTPUT); // active LED

    bleGamepad.setRightThumb(0);
}

void loop() {
    if (!bleGamepad.isConnected()) {
        if (pairingLedState) digitalWrite(23, LOW);
        else digitalWrite(23, HIGH);

        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval) {
            pairingLedState = !pairingLedState;
            previousMillis = currentMillis;
        }

        return;
    };

    digitalWrite(23, LOW);

    float accelRaw = constrain(lis.readFloatAccelX(), -1.0, 1.0) + 1.0;
    float accel = 32767 - (accelRaw * 16383.5);
    int accelInt = int(trunc(accel));

    int raw = analogRead(32);
    raw = constrain(raw, 0, 4096);

    bleGamepad.setRightThumb(raw * 4, accelInt);

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

