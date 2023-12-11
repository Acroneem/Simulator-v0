#define DECODE_NEC        
#define DECODE_FAST

#include <Arduino.h>
#define TRIGGER_BUTTON 7
#define RESURRECT_PIN 11
#define RGB_BLUE 8
#define RGB_RED 9
#define RGB_GREEN 10
/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library
// State variables
bool isPlayerLost = false;
unsigned long resurrectionStartTime = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

    pinMode(TRIGGER_BUTTON, INPUT_PULLUP);
    pinMode(RGB_BLUE, OUTPUT);
    pinMode(RGB_RED, OUTPUT);
    pinMode(RGB_GREEN, OUTPUT);
    pinMode(RESURRECT_PIN, INPUT_PULLUP);
}

void loop() {
    if (!isPlayerLost && digitalRead(TRIGGER_BUTTON) == LOW) {
        Serial.println(F("Send standard NEXT with 8 bit address"));
        Serial.flush();

        IrSender.sendFAST(3, 1); // Send IR signal
        delay(100);
    }

    if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.command == 0x03) {
            isPlayerLost = true; // Set player to lost state on receiving 0x03
        }

        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();

        IrReceiver.resume(); // Enable receiving of the next value
    }
    
    //keep the lights off when not lost
    if (!isPlayerLost){
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, LOW);
    }

    // Handle lost state and resurrection
    if (isPlayerLost) {
        digitalWrite(RGB_RED, HIGH);
        digitalWrite(RGB_GREEN, LOW);
        digitalWrite(RGB_BLUE, LOW);

        if (digitalRead(RESURRECT_PIN) == LOW) {
            if (resurrectionStartTime == 0) {
                resurrectionStartTime = millis();
            } else if (millis() - resurrectionStartTime >= 5000) {
                isPlayerLost = false;
                resurrectionStartTime = 0;
                
                // Flash green LED to indicate resurrection complete
                digitalWrite(RGB_GREEN, HIGH);
                delay(200);
                digitalWrite(RGB_GREEN, LOW);
            } else {
                // Pulse blue LED during resurrection process
                digitalWrite(RGB_RED, LOW);
                digitalWrite(RGB_BLUE, HIGH);
                delay(100);
                digitalWrite(RGB_BLUE, LOW);
                delay(100);
            }
        } else {
            resurrectionStartTime = 0; // Reset if button is released
            digitalWrite(RGB_RED, HIGH);
            digitalWrite(RGB_GREEN, LOW);
            digitalWrite(RGB_BLUE, LOW);
        }
    }
}