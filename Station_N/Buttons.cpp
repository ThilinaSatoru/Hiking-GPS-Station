#include "Config.h"

// Debugging
// #define DEBUG_MODE  // Comment this line to disable debugging
#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#endif

void setupButtons() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(LED1_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);

    #ifdef DEBUG_MODE
    DEBUG_PRINT("Debug mode enabled.");
    #endif
}

void updateLEDForState() {
    switch (currentButtonMode) {
        case STATE_NORMAL:
            digitalWrite(LED1_PIN, LOW);
            DEBUG_PRINT("Mode: NORMAL");
            break;
        case STATE_EMERGENCY:
            digitalWrite(LED1_PIN, HIGH);
            DEBUG_PRINT("Mode: EMERGENCY");
            break;
        case STATE_SECONDARY:
            // Fast blinking
            digitalWrite(LED1_PIN, (millis() / 200) % 2);
            DEBUG_PRINT("Mode: SECONDARY (Fast Blink)");
            break;
        case STATE_TERTIARY:
            // Slow blinking
            digitalWrite(LED1_PIN, (millis() / 1000) % 2);
            DEBUG_PRINT("Mode: TERTIARY (Slow Blink)");
            break;
    }
}

void handleButtonPress(int buttonPin, bool &lastButtonState) {
    bool currentButtonState = !digitalRead(buttonPin);  // Invert because INPUT_PULLUP is active LOW

    if (currentButtonState && !lastButtonState) {  // Button just pressed
        // Cycle through modes
        currentButtonMode = static_cast<ButtonState>((currentButtonMode + 1) % 4);
        DEBUG_PRINT("Button pressed. Cycling mode.");
    }

    lastButtonState = currentButtonState;
}

void resetButtonCycle() {
    currentButtonMode = STATE_NORMAL;
    DEBUG_PRINT("Cycle reset to NORMAL.");
}

void handleButtons() {
    handleButtonPress(BUTTON1_PIN, lastButton1State);
    updateLEDForState();
}