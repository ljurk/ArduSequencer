#include "..\lib\display.hpp"


#define STEP_LENGTH 16
#define NUMBER_OF_CHANNELS 4
#define DISPLAY_OFFSET 4
displaySequencer::displaySequencer(bool debug) {
   debugDisplay = debug;
   seq = sequencer(debug);

   //display
   lcd.init();
   lcd.backlight();
   lcd.cursor();
   lcd.noBlink();
   pinMode(encoderButtonPin,INPUT);
   pinMode(modeButtonPin,INPUT);
   seq.setActiveChannel(0);
   for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
     pinMode(channelPins[i],INPUT);
   }
   if(debugDisplay) {
     //set baud rate for serial monitor
     Serial.begin(9600);
     seq.start();
   } else {
     // set MIDI baud
     Serial.begin(31250);
   }
}

void displaySequencer::updateCursor() {
  //not in use
  /*if(mode == 0 ) {
    for(int i = 0; i < STEP_LENGTH ;i++) {
      if(i == seq.getActiveStep()) {
        cursorString[i] = '^';
      }else if(i == seq.getCursorPos()) {
          cursorString[i] =' ';
        } else {
          cursorString[i] = ' ';
        }
      }
    } else {
      for(int i = 0; i < STEP_LENGTH ;i++) {
          cursorString[i] = ' ';
        }
    }
    lcd.setCursor(0,1);
    lcd.print(cursorString);
    */
}

void displaySequencer::updateValues(){
  String outNote;
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    lcd.setCursor(0,i);
    if(i == seq.getActiveChannel()) {
      if(seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()) < 10) {
        outNote= "V";
        if(seq.getMute(i)) {
            outNote.toLowerCase();
        }
        outNote += String(seq.getVelocity(seq.getActiveChannel(),seq.getCursorPos()));
        //outNote += String(seq.getActiveStep(i));
        outNote += " ";
      }else if(seq.getVelocity(seq.getActiveChannel(), seq.getCursorPos()) < 100) {
          outNote= "V";
          if(seq.getMute(i)) {
              outNote.toLowerCase();
          }
          outNote += String(seq.getVelocity(seq.getActiveChannel(),seq.getCursorPos()));
      } else {
          outNote = String(seq.getVelocity(seq.getActiveChannel(),seq.getCursorPos()));
      }
      if(mode == 0) {
        outNote += ">";
      } else {
        outNote += "<";
      }
    } else {
      outNote = seq.getNoteText(i);
      if(seq.getMute(i)) {
        outNote.toLowerCase();
      }
      if(seq.getNoteTextLength(i) == 2) {
        outNote += "  ";
      }
      if(seq.getNoteTextLength(i) == 3) {
        outNote += " ";
      }
    }
    lcd.print(outNote);
  }
}

void displaySequencer::updateActiveStep(){
  for(int i = 0; i< NUMBER_OF_CHANNELS; i ++) {
    if(debugDisplay) {
      Serial.print("gate oldStep");
      Serial.println(seq.getGate(i, seq.getOldStep(i)));
      Serial.print("gate activeStep");
      Serial.println(seq.getGate(i, seq.getActiveStep(i)));
    }
    lcd.setCursor(DISPLAY_OFFSET + seq.getOldStep(i),i);
    if(seq.getGate(i, seq.getOldStep(i))) {
        lcd.print('X');
    } else if(seq.getOldStep(i) == (STEP_LENGTH / 2) || seq.getOldStep(i) == (STEP_LENGTH / 4) || seq.getOldStep(i) == 0 ) {
      lcd.print('=');
    }else  {
      lcd.print('-');
    }
      lcd.setCursor(DISPLAY_OFFSET + seq.getActiveStep(i),i);
      lcd.print('|');
  }

}
void displaySequencer::updateSequence(){
  //displays sequence for each channel
  for(int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(int i = 0; i  <STEP_LENGTH; i++) {
      if(seq.getLength(y) > i) {
        if(seq.getActiveStep(y) == i) {
          seqString[i] = '|';
        } else if(seq.getGate(y, i) == true) {
          seqString[i] = 'X';
        } else if((i % (STEP_LENGTH / 4)) == 0) {
          //marks every 4th step with a =
          seqString[i] = '=';
        }else {
          seqString[i] = '-';
        }
      } else {
        seqString[i] = ' ';
      }
    }
    lcd.setCursor(DISPLAY_OFFSET,y);
    lcd.print(seqString);
  }
}

void displaySequencer::updateDisplay(){
  //updateCursor();
  if(activeStepChanged) {
    updateActiveStep();
    activeStepChanged = false;
  }
  if(sequenceChanged) {
    if(debugDisplay) {
      Serial.println("Update sequence");
    }
    updateSequence();
    sequenceChanged = false;
  }
  if(valuesChanged) {
    if(debugDisplay) {
      Serial.println("Update values");
    }
    updateValues();
    valuesChanged = false;
  }

    //after everything ist displayed change the cursor position
    //to active element
    //lcd.setCursor(seq.getCursorPos() + DISPLAY_OFFSET,seq.getActiveChannel());
    if(cursorChanged) {
      if(debugDisplay) {
        Serial.println("UpdateCursor");
        Serial.print(seq.getCursorPos());
      }
        lcd.setCursor(seq.getCursorPos() + DISPLAY_OFFSET,seq.getActiveChannel());
        cursorChanged = false;
    }

  //lcd.print('-');
//  delay(300);
}

void displaySequencer::checkInputs(){
  //get new data from buttons and encoder
  newEncoderPos = myEnc.read();
  encoderButtonState = digitalRead(encoderButtonPin);
  modeButtonState = digitalRead(modeButtonPin);
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    channelButtonStates[i] = digitalRead(channelPins[i]);
  }

  //encoder
  if (newEncoderPos != oldEncoderPos) {
    if(newEncoderPos > oldEncoderPos + 2) {
      if(debugDisplay) {
        Serial.println("turn right");
      }
      if (encoderButtonState == HIGH) {
        if(debugDisplay) {
          Serial.println("PRESS&ROTATE");
        }
        //set more steps in sequencer mode
        if(mode == 0) {
          seq.setCursorPos(NEXT);
          seq.setGate();
          seq.setNote();
          sequenceChanged = true;
          cursorChanged = true;
        }
        //faster higher velocity in velocity mode
        if(mode == 1 ) {
          seq.setVelocityUp(10);
          valuesChanged = true;
        }
      } else {
        if(mode == 1) { //velo
          seq.setVelocityUp();
          valuesChanged = true;
        }
        if (mode == 0 ){//cursor
          seq.setCursorPos(NEXT);
          cursorChanged = true;
        }
      }

      for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        if (channelButtonStates[i] == HIGH) {
          sequenceChanged = true;
          cursorChanged = true;
          seq.setLength(seq.getLength(i) + 1);
        }
      }
      oldEncoderPos = newEncoderPos;
    } else if(newEncoderPos < oldEncoderPos - 2) {
      //turn left
      if(debugDisplay) {
        Serial.println("turn left");
      }
      if (encoderButtonState == HIGH) {
        if(debugDisplay) {
          Serial.println("PRESS&ROTATE");
        }
        if(mode == 0) {
          seq.setCursorPos(PREV);
          seq.setGate();
          seq.setNote();
          sequenceChanged = true;
          cursorChanged = true;
        }
        if(mode == 1) {
          seq.setVelocityDown(10);
          valuesChanged = true;
        }
      } else{
        if(mode == 1) { //velo
          seq.setVelocityDown();
          valuesChanged = true;
        }
        if (mode == 0 ){//cursor
          seq.setCursorPos(PREV);
          cursorChanged = true;
        }
      }

      for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        if (channelButtonStates[i] == HIGH) {
          sequenceChanged = true;
          cursorChanged = true;
          seq.setLength(seq.getLength(i) - 1);
        }
      }
      oldEncoderPos = newEncoderPos;
    }


  }
  if(encoderButtonState == HIGH && encoderButtonPressed == false) {
    sequenceChanged = true;
    if(debugDisplay) {
      Serial.println("PRESS");
    }
    if(mode == 0) {
      seq.setGate();
      seq.setNote();
    }
    encoderButtonPressed = true;
  } else if(encoderButtonState == LOW) {
    encoderButtonPressed = false;
  }
  if(modeButtonState == HIGH && modeButtonPressed == false) {
    valuesChanged = true;
    if(debugDisplay) {
        Serial.println("PRESS MODE");
    }

    if(mode == 0) {
      //velocity
      mode = 1;
    } else {
      //sequence
      mode = 0;
    }
    if(debugDisplay) {
      for (int i= 0; i< 12; i++) {
            seq.clock();
      }
      activeStepChanged = true;
    }
    modeButtonPressed = true;
  } else if(modeButtonState == LOW) {
    modeButtonPressed = false;
  }
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    if (channelButtonStates[i] == HIGH && channelButtonPressed[i] == false) {
      if(mode == 0) {
        cursorChanged = true;
        valuesChanged = true;
        seq.setActiveChannel(i);
        if(seq.getCursorPos() >= seq.getLength(i)) {
          seq.setCursorPosDirect(seq.getLength(i) - 1);
        }
      }
      if(mode == 1) {
        seq.setMute(i);
        valuesChanged = true;
      }

      //lcd.setCursor(seq.getCursorPos(),seq.getActiveChannel());
      channelButtonPressed[i] = true;
    } else if(channelButtonStates[i] == LOW) {
      channelButtonPressed[i] = false;
    }
  }
  updateDisplay();
}

void displaySequencer::startingAnimation() {
  if(debugDisplay) {
    Serial.print("startingAnimation");
  }
  // display a line of text
  lcd.setCursor(0,0);
  lcd.print("SEQ v3.0.3 #23.23");
  lcd.setCursor(0,1);
  lcd.print("MIDI-channel ");
  lcd.print(MIDI_CHANNEL + 1);

  lcd.setCursor(0,3);
  lcd.print("Acid changed my life");

  for(int i = 0; i < 20; i++) {
    lcd.setCursor(i,2);
    lcd.print('.');
    delay(500);
  }
  lcd.clear();
}

void displaySequencer::run() {
 checkInputs();
  if(!debugDisplay) {
   if(Serial.available()  > 0) {
     byte byte_read = Serial.read();
     switch(byte_read) {
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
          if(count == (12)) {
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
