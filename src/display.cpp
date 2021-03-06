#include "../lib/display.hpp"


#define STEP_LENGTH 16
#define NUMBER_OF_CHANNELS 4
#define DISPLAY_OFFSET 4

displaySequencer::displaySequencer(bool debug) {
    debugDisplay = debug;
    seq = sequencer(debug);

    // initialize channels
    // {pin, muted, buttonPressed, buttonState}
    channels[0] = {9, false, false, false};
    channels[1] = {10, false, false, false};
    channels[2] = {11, false, false, false};
    channels[3] = {12, false, false, false};

    // display
    lcd.init();
    lcd.backlight();
    lcd.cursor();
    lcd.noBlink();

    // encoder
    pinMode(encoderButtonPin, INPUT);
    pinMode(modeButtonPin, INPUT);

    // sequencer
    seq.setActiveChannel(0);
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
      pinMode(channels[i].pin, INPUT);
    }

    if (debugDisplay) {
      // set baud rate for serial monitor
      Serial.begin(9600);
      seq.start();
    } else {
      // set MIDI baud
      Serial.begin(31250);
    }

}

void displaySequencer::updateValues() {
    String outNote;
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        lcd.setCursor(0, i);
        if (i == seq.getActiveChannel()) {
            if (seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()) < 10) {
                outNote= "V";
                if (seq.getMute(i)) {
                    outNote.toLowerCase();
                }
                outNote += String(seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()));
                // outNote += String(seq.getActiveStep(i));
                outNote += " ";
            }else if (seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()) < 100) {
                outNote= "V";
                if (seq.getMute(i)) {
                    outNote.toLowerCase();
                }
                outNote += String(seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()));
            } else {
                outNote = String(seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()));
            }
            if (mode == 0) {
                outNote += ">";
            } else if (mode == 1){
                outNote += "<";
            } else {
                outNote += "e";
            }
        } else {
            outNote = seq.getNoteText(i);
            if (seq.getMute(i)) {
                outNote.toLowerCase();
            }
            if (seq.getNoteTextLength(i) == 2) {
                outNote += "  ";
            }
            if (seq.getNoteTextLength(i) == 3) {
                outNote += " ";
            }
        }
        lcd.print(outNote);
    }
}

void displaySequencer::updateActiveStep() {
  for (int i = 0; i< NUMBER_OF_CHANNELS; i ++) {
    if (debugDisplay) {
      Serial.print("gate oldStep");
      Serial.println(seq.getGate(i, seq.getOldStep(i)));
      Serial.print("gate activeStep");
      Serial.println(seq.getGate(i, seq.getActiveStep(i)));
    }
    lcd.setCursor(DISPLAY_OFFSET + seq.getOldStep(i), i);
    if (seq.getGate(i, seq.getOldStep(i))) {
        if (seq.getCursorPos() == seq.getOldStep(i)) {
          lcd.print(cursorFiledStepSymbol);
        } else {
          lcd.print(filedStepSymbol);
        }
    } else if ((seq.getOldStep(i) % (STEP_LENGTH / 4)) == 0) {
      lcd.print(quarterSymbol);
    } else {
      if (seq.getCursorPos() == seq.getOldStep(i)) {
        lcd.print(cursorEmptyStepSymbol);
      } else {
        lcd.print(emptyStepSymbol);
      }
    }
      lcd.setCursor(DISPLAY_OFFSET + seq.getActiveStep(i), i);
      lcd.print(activeStepSymbol);
  }

}
void displaySequencer::updateSequence() {
  // displays sequence for each channel
  for (int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for (int i = 0; i  <STEP_LENGTH; i++) {
      if (seq.getLength(y) > i) {
        if (seq.getActiveStep(y) == i) {
          seqString[i] = activeStepSymbol;
        } else if (seq.getGate(y, i) == true) {
          if (seq.getCursorPos() == i) {
            seqString[i] = cursorFiledStepSymbol;
          } else {
            seqString[i] = filedStepSymbol;
          }
        } else if (i == seq.getCursorPos() && y == seq.getActiveChannel()) {
          seqString[i] = cursorEmptyStepSymbol;
        }else if ((i % (STEP_LENGTH / 4)) == 0) {
          // marks every 4th step with a =
          seqString[i] = quarterSymbol;
        }else {
          seqString[i] = emptyStepSymbol;
        }
      } else {
        seqString[i] = ' ';
      }
    }
    lcd.setCursor(DISPLAY_OFFSET, y);
    lcd.print(seqString);
  }
}

void displaySequencer::updateDisplay() {
  // updateCursor();
  if (activeStepChanged) {
    updateActiveStep();
    activeStepChanged = false;
  }
  if (sequenceChanged || cursorChanged) {
    if (debugDisplay) {
      Serial.println("Update sequence");
    }
    updateSequence();
    sequenceChanged = false;
    cursorChanged = false;
  }
  if (valuesChanged) {
    if (debugDisplay) {
      Serial.println("Update values");
    }
    updateValues();
    valuesChanged = false;
  }

    // after everything ist displayed change the cursor position
    // to active element
    // lcd.setCursor(seq.getCursorPos() + DISPLAY_OFFSET, seq.getActiveChannel());
    if (cursorChanged) {
      if (debugDisplay) {
        Serial.println("UpdateCursor");
        Serial.print(seq.getCursorPos());
      }
        lcd.setCursor(seq.getCursorPos() + DISPLAY_OFFSET, seq.getActiveChannel());
        cursorChanged = false;
    }

  // lcd.print('-');
//  delay(300);
}

void displaySequencer::checkInputs() {
    // get new data from buttons and encoder
    newEncoderPos = myEnc.read();
    encoderButtonState = digitalRead(encoderButtonPin);
    modeButtonState = digitalRead(modeButtonPin);
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
      channels[i].buttonState = digitalRead(channels[i].pin);
    }

    // encoder
    if (newEncoderPos != oldEncoderPos) {
      if (newEncoderPos > oldEncoderPos + 2) {
        if (debugDisplay) {
          Serial.println("turn right");
        }
        if (encoderButtonState == HIGH) {
          if (debugDisplay) {
            Serial.println("PRESS&ROTATE");
          }
          // set more steps in sequencer mode
          if (mode == 0) {
            seq.setCursorPos(NEXT);
            seq.setGate();
            seq.setNote();
            sequenceChanged = true;
            cursorChanged = true;
          }
          // faster higher velocity in velocity mode
          if (mode == 1) {
            seq.setVelocityUp(10);
            valuesChanged = true;
          }
        } else {
          if (mode == 1) {  // velo
            seq.setVelocityUp();
            valuesChanged = true;
          }
          if (mode == 0) {  // cursor
            seq.setCursorPos(NEXT);
            cursorChanged = true;
          }
          if (mode == 2) {  // euclidean
            seq.euclidAddGate();
            sequenceChanged = true;
          }
        }

        for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
          if (channels[i].buttonState == HIGH) {
            sequenceChanged = true;
            cursorChanged = true;
            seq.setLength(seq.getLength(i) + 1);
          }
        }
        oldEncoderPos = newEncoderPos;
      } else if (newEncoderPos < oldEncoderPos - 2) {
        // turn left
        if (debugDisplay) {
          Serial.println("turn left");
        }
        if (encoderButtonState == HIGH) {
          if (debugDisplay) {
            Serial.println("PRESS&ROTATE");
          }
          if (mode == 0) {
            seq.setCursorPos(PREV);
            seq.setGate();
            seq.setNote();
            sequenceChanged = true;
            cursorChanged = true;
          }
          if (mode == 1) {
            seq.setVelocityDown(10);
            valuesChanged = true;
          }
        } else{
          if (mode == 1) {  // velo
            seq.setVelocityDown();
            valuesChanged = true;
          }
          if (mode == 0 ) {  // cursor
            seq.setCursorPos(PREV);
            cursorChanged = true;
          }
          if (mode == 2) {  // euclidean
            seq.euclidRemoveGate();
            sequenceChanged = true;
          }
        }

        for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
          if (channels[i].buttonState == HIGH) {
            sequenceChanged = true;
            cursorChanged = true;
            seq.setLength(seq.getLength(i) - 1);
          }
        }
        oldEncoderPos = newEncoderPos;
      }
    }

    if (encoderButtonState == HIGH && encoderButtonPressed == false) {
      sequenceChanged = true;
      if (debugDisplay) {
        Serial.println("PRESS");
      }
      if (mode == 0) {
        seq.setGate();
        seq.setNote();
      }
      encoderButtonPressed = true;
    } else if (encoderButtonState == LOW) {
      encoderButtonPressed = false;
    }
    if (modeButtonState == HIGH && modeButtonPressed == false) {
      valuesChanged = true;
      if (debugDisplay) {
          Serial.println("PRESS MODE");
      }

      if (mode == 0) {
        // velocity
        mode = 1;
      } else if (mode == 1){
        // sequence
        mode = 2;
      } else {
          mode = 0;
      }

      if (debugDisplay) {
        for (int i= 0; i< 12; i++) {
              seq.clock();
        }
        activeStepChanged = true;
      }
      modeButtonPressed = true;
    } else if (modeButtonState == LOW) {
      modeButtonPressed = false;
    }
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
      if (channels[i].buttonState == HIGH && channels[i].buttonPressed == false) {
        if (mode == 0) {
          cursorChanged = true;
          valuesChanged = true;
          seq.setActiveChannel(i);
          if (seq.getCursorPos() >= seq.getLength(i)) {
            seq.setCursorPosDirect(seq.getLength(i) - 1);
          }
        }
        if (mode == 1) {
          seq.setMute(i);
          valuesChanged = true;
        }

        // lcd.setCursor(seq.getCursorPos(), seq.getActiveChannel());
        channels[i].buttonPressed = true;
      } else if (channels[i].buttonState == LOW) {
        channels[i].buttonPressed = false;
      }
    }
    updateDisplay();
}

void displaySequencer::startingAnimation() {
  if (debugDisplay) {
    Serial.print("startingAnimation");
  }
  // display a line of text
  lcd.setCursor(0, 0);
  lcd.print("SEQ v3.0.3 #23.23");
  lcd.setCursor(0, 1);
  lcd.print("MIDI-channel ");
  lcd.print(MIDI_CHANNEL + 1);
  lcd.setCursor(0, 3);
  lcd.print("Acid changed my life");

  for (int i = 0; i < 20; i++) {
    lcd.setCursor(i, 2);
    lcd.print('.');
    delay(200);
  }
  String test = "<><>made by Lukn<><>";
  for (int i = 0; i < 20; i++) {
    lcd.setCursor(i, 0);
    lcd.print('.');
    lcd.setCursor(20 - i, 1);
    lcd.print('.');
    lcd.setCursor(i, 2);
    lcd.print(test[i]);
    lcd.setCursor(20 - i, 3);
    lcd.print('.');
    delay(200);
  }
  lcd.clear();
}

void displaySequencer::run() {
    checkInputs();
    if (!debugDisplay) {
        if (Serial.available()  > 0) {
            byte byte_read = Serial.read();
            switch (byte_read) {
                case MIDI_START:
                    seq.start();
                    break;
                case MIDI_STOP:
                    seq.stop();
                    count = 0;
                    updateDisplay();
                    break;
                case MIDI_CONT:
                    seq.cont();
                    break;
                case MIDI_CLOCK:
                    seq.clock();
                    count++;
                    if (count == (12)) {
                        activeStepChanged = true;
                        updateDisplay();
                        count = 0;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
