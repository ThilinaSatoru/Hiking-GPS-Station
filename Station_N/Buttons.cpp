#include "Config.h"

const unsigned long LED_TIMEOUT = 2000;

void setupButtons() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
}

void handleButtonPress(int buttonPin, bool &lastButtonState) {
    bool currentButtonState = !digitalRead(buttonPin);
    
    if (currentButtonState && !lastButtonState) {  // Button just pressed
        if (buttonPin == BUTTON1_PIN) {
            EMERGENCY = true;  // Set emergency state
            led1State = true;  // Update LED1 state to match EMERGENCY
            digitalWrite(LED1_PIN, HIGH);
            
            emergencyDisplayState = EMERGENCY_DISPLAY_SENDING;

        } else if (buttonPin == BUTTON2_PIN) {
            HELP = true;
            led2State = true;
            // led2Timer = millis();
            digitalWrite(LED2_PIN, HIGH);
        }
    }
    
    lastButtonState = currentButtonState;
}

void handleLEDStates() {
    // LED1 is controlled by EMERGENCY state
    if (!EMERGENCY && led1State) {
        led1State = false;
        digitalWrite(LED1_PIN, LOW);
    } else if (EMERGENCY && !led1State) {
        led1State = true;
        digitalWrite(LED1_PIN, HIGH);
    }
}

void updateLEDState() {
    // This function is called when EMERGENCY state changes programmatically
    digitalWrite(LED1_PIN, EMERGENCY);
    led1State = EMERGENCY;
    digitalWrite(LED2_PIN, HELP);
    led2State = HELP;
}

void handleButtons() {
    handleButtonPress(BUTTON1_PIN, lastButton1State);
    handleButtonPress(BUTTON2_PIN, lastButton2State);
    handleLEDStates();
}