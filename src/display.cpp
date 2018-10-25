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
   } else {
     // set MIDI baud
     Serial.begin(31250);
   }
}

void displaySequencer::updateCursor() {
  //not in use
  if(mode == 0 ) {
    for(int i = 0; i < STEP_LENGTH ;i++) {
      if(i == seq.getActiveStep()) {
        cursorString[i] = '^';
      }else if(i == seq.getActiveMenuStep()) {
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
}

void displaySequencer::updateValues(){
  String outNote;
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    lcd.setCursor(0,i);
    if(i == seq.getActiveChannel()) {
      if(seq.getVelocity(seq.getActiveChannel(), seq.getActiveMenuStep()) < 10) {
        outNote= "V";
        outNote += String(seq.getVelocity(seq.getActiveChannel(),seq.getActiveMenuStep()));
        outNote += " ";
      }else if(seq.getVelocity(seq.getActiveChannel(), seq.getActiveMenuStep()) < 100) {
          outNote= "V";
          outNote += String(seq.getVelocity(seq.getActiveChannel(),seq.getActiveMenuStep()));
      } else {
          outNote = String(seq.getVelocity(seq.getActiveChannel(),seq.getActiveMenuStep()));
      }
      if(mode == 0) {
        outNote += ">";
      } else {
        outNote += "<";
      }
    } else {
      outNote = seq.getNoteText(i);
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

}
void displaySequencer::updateSequence(){
  //displays sequence for each channel
  for(int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(int i = 0; i  <STEP_LENGTH; i++) {
      if(seq.getActiveStep() == i) {
        seqString[i] = '|';
      } else if(seq.getGate(y, i) == true) {
        seqString[i] = 'X';
      } else {
        seqString[i] = '-';
      }
    }
    lcd.setCursor(DISPLAY_OFFSET,y);
    lcd.print(seqString);
  }
}

void displaySequencer::updateDisplay(){
  //updateCursor();
  if(somethingChanged) {
    if(debugDisplay) {
      Serial.println("Update DIsplay");
    }
    updateSequence();
    updateValues();
    //after everything ist displayed change the cursor position
    //to active element
    //lcd.setCursor(seq.getActiveMenuStep() + DISPLAY_OFFSET,seq.getActiveChannel());
    somethingChanged = false;
  }
  lcd.setCursor(seq.getActiveMenuStep() + DISPLAY_OFFSET,seq.getActiveChannel());
  //lcd.print('-');
//  delay(300);
}

void displaySequencer::checkInputs(){
  newPosition = myEnc.read();
  encoderButtonState = digitalRead(encoderButtonPin);
  modeButtonState = digitalRead(modeButtonPin);
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    channelButtonStates[i] = digitalRead(channelPins[i]);
  }
  if (newPosition != oldPosition) {
    somethingChanged = true;
    if(newPosition > oldPosition + 1) {
      //turn right
      if (encoderButtonState == HIGH) {
        if(debugDisplay) {
          Serial.println("PRESS&ROTATE");
        }
        if(mode == 0) {
          seq.setCursor(NEXT);
          seq.setGate();
          seq.setNote();
        }
        if(mode == 1 ) {
          seq.setVelocityUp(10);
        }
      } else{
        if(mode == 1) { //velo
          seq.setVelocityUp();
        }
        if (mode == 0 ){//cursor
          seq.setCursor(NEXT);
        }
      }
    } else if(newPosition < oldPosition - 1){
      //turn left
      if (encoderButtonState == HIGH) {
        if(debugDisplay) {
          Serial.println("PRESS&ROTATE");
        }
        if(mode == 0) {
          seq.setCursor(PREV);
          seq.setGate();
          seq.setNote();
        }
        if(mode == 1) {
          seq.setVelocityDown(10);
        }
      } else{
        if(mode == 1) { //velo
          seq.setVelocityDown();
        }
        if (mode == 0 ){//cursor
          seq.setCursor(PREV);
        }
      }
    }
    if(debugDisplay) {
    //  Serial.println(newPosition);
    }
    oldPosition = newPosition;
  }else if(encoderButtonState == HIGH && encoderButtonPressed == false) {
    somethingChanged = true;
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
    somethingChanged = true;
    Serial.println("PRESS MODE");
    if(mode == 0) {
      //velocity
      mode = 1;
    } else {
      //sequence
      mode = 0;
    }
    modeButtonPressed = true;
  } else if(modeButtonState == LOW) {
    modeButtonPressed = false;
  }
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    if (channelButtonStates[i] == HIGH && channelButtonPressed[i] == false) {
      somethingChanged = true;
      seq.setActiveChannel(i);
      //lcd.setCursor(seq.getActiveMenuStep(),seq.getActiveChannel());
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
  lcd.print(MIDI_CHANNEL);
  lcd.setCursor(0,2);
  lcd.print("<><><><><><><><><><>");
  lcd.setCursor(0,3);
  lcd.print("Acid changed my life");

  delay(5000);
  lcd.clear();
}

void displaySequencer::run() {
 checkInputs();
 if(Serial.available()  > 0) {
   byte byte_read = Serial.read();
   switch(byte_read) {
     case MIDI_START:
         seq.start();
       break;
     case MIDI_STOP:
       seq.stop();
       updateDisplay();
       break;
     case MIDI_CONT:
       seq.cont();
       break;
     case MIDI_CLOCK:
        updateDisplay();
        seq.clock();
       break;
     default:
       break;
   }
 }
}
