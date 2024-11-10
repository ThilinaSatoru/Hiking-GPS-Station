#include "Config.h"

const unsigned long LED_TIMEOUT = 2000;

void setupButtons() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(LED1_PIN, OUTPUT);  
    digitalWrite(LED1_PIN, LOW);
}
