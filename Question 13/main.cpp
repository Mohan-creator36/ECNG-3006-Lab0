#include <Arduino.h>

#define LED_PIN 2

void setup()
{
    pinMode(LED_PIN, OUTPUT);

    SerialUSB.begin(9600);

    while (!SerialUSB)
    {
        // Wait for the Serial Monitor to open
    }
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    SerialUSB.println("On");
    delay(1000);

    digitalWrite(LED_PIN, LOW);
    SerialUSB.println("Off");
    delay(1000);
}
