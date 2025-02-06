#include "Config.h"

// Debugging
// #define DEBUG_MODE  // Comment this line to disable debugging
#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#endif

// Constants
const unsigned long INACTIVITY_TIMEOUT = 5000; // 5 seconds of inactivity to confirm choice

// Variables
unsigned long lastButtonPressTime = 0; // Tracks the last time the button was pressed
bool choiceConfirmed = false;          // Tracks if the choice has been confirmed

void setupButtons()
{
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(LED1_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);

#ifdef DEBUG_MODE
    DEBUG_PRINT("Debug mode enabled.");
#endif
}

void updateLEDForState()
{
    switch (currentButtonMode)
    {
    case STATE_NORMAL:
        digitalWrite(LED1_PIN, LOW);
        DEBUG_PRINT("Mode: NORMAL");
        break;
    case STATE_MEDICAL:
        digitalWrite(LED1_PIN, HIGH);
        DEBUG_PRINT("Mode: MEDICAL");
        break;
    case STATE_ENVIRONMENT:
        // Fast blinking
        digitalWrite(LED1_PIN, (millis() / 200) % 2);
        DEBUG_PRINT("Mode: ENVIRONMENT (Fast Blink)");
        break;
    case STATE_RESOURCES:
        // Slow blinking
        digitalWrite(LED1_PIN, (millis() / 1000) % 2);
        DEBUG_PRINT("Mode: RESOURCES (Slow Blink)");
        break;
    }
}

void handleButtonPress(int buttonPin, bool &lastButtonState)
{
    bool currentButtonState = !digitalRead(buttonPin); // Invert because INPUT_PULLUP is active LOW

    if (currentButtonState && !lastButtonState)
    { // Button just pressed
        // Cycle through modes
        currentButtonMode = static_cast<ButtonMode>((currentButtonMode + 1) % 4);
        lastButtonPressTime = millis(); // Update the last button press time
        choiceConfirmed = false;        // Reset confirmation flag
        DEBUG_PRINT("Button pressed. Cycling mode.");
    }

    lastButtonState = currentButtonState;
}

void confirmChoice()
{
    switch (currentButtonMode)
    {
    case STATE_NORMAL:
        DEBUG_PRINT("Choice confirmed: NORMAL");
        break;
    case STATE_MEDICAL:
        DEBUG_PRINT("Choice confirmed: MEDICAL");
        break;
    case STATE_ENVIRONMENT:
        DEBUG_PRINT("Choice confirmed: ENVIRONMENT");
        break;
    case STATE_RESOURCES:
        DEBUG_PRINT("Choice confirmed: RESOURCES");
        break;
    }
    choiceConfirmed = true;
    selectedButtonMode = currentButtonMode;
}

void resetButtonCycle()
{
    currentButtonMode = STATE_NORMAL;
    selectedButtonMode = STATE_NORMAL;
    choiceConfirmed = false;
    DEBUG_PRINT("Cycle reset to NORMAL.");
}

void handleButtons()
{
    handleButtonPress(BUTTON1_PIN, lastButton1State);
    updateLEDForState();

    // Check for inactivity and confirm choice if timeout is reached
    if (!choiceConfirmed && (millis() - lastButtonPressTime >= INACTIVITY_TIMEOUT))
    {
        confirmChoice();
    }
}