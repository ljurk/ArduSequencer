#include "..\lib\display.hpp"


#define STEP_LENGTH 16

displaySequencer::displaySequencer(bool debug) {
   debugDisplay = debug;
   seq = sequencer(debug);

   //display
   lcd.init();
   lcd.backlight();
   lcd.blink();
   pinMode(buttonPin,INPUT);
   if(debugDisplay) {
     //set baud rate for serial monitor
     Serial.begin(9600);
   } else {
     // set MIDI baud
     Serial.begin(31250);
   }
 }


void displaySequencer::updateCursor() {
  //display.setTextSize(2);
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
    //display.setTextSize(1);
}

void displaySequencer::updateValues(){
  //display.setTextSize(1);
  lcd.setCursor(0,3);
  if(mode == 1) {
    //display.setTextColor(BLACK);
    //display.fillRect(0,48, 64, 16, WHITE);
  } else {
    //display.setTextColor(WHITE);
  }
  String outNote = "NOTE ";
  outNote += String(seq.getNote(seq.getActiveMenuStep()));
  String outVelo= " VELO ";
  outVelo += String(seq.getVelocity(seq.getActiveMenuStep()));
  //display.setCursor(0,48);
  lcd.print(outNote);
  if(mode == 2) {
    //display.setTextColor(BLACK);
    //display.fillRect(64,48, 64, 16, WHITE);
  } else {
    //display.setTextColor(WHITE);
  }
  lcd.setCursor(10,3);
  lcd.print(outVelo);
  //display.setTextColor(WHITE);
}

void displaySequencer::updateActiveStep(){

}
void displaySequencer::updateSequence(){
  //display.setTextSize(2);
  for(int i = 0; i  <STEP_LENGTH; i++) {
      if(seq.getGate(i) == true) {
        seqString[i] = 'X';
      } else {
        seqString[i] = '-';
      }
  }
  lcd.setCursor(0,0);
  lcd.print(seqString);
  //display.setTextSize(1);
}

void displaySequencer::updateDisplay(){
  if(debugDisplay) {
    Serial.print("update Display");
  }

  updateValues();
  updateCursor();
  updateSequence();
  if(mode == 0) {
    lcd.setCursor(seq.getActiveMenuStep(),0);
  } else if(mode == 1) {
    lcd.setCursor(0,3);
  } else if (mode == 2) {
    lcd.setCursor(11,3);
  }
  //display.display();
}

void displaySequencer::checkInputs(){
  newPosition = myEnc.read();
  buttonState = digitalRead(buttonPin);
  if (newPosition != oldPosition) {
    if(newPosition > oldPosition + 1) {
      //turn right
      if (buttonState == HIGH){
        mode++;
        if(mode == 3) {//mode
          mode = 0;
        }
        if(mode == 0) {
          lcd.setCursor(seq.getActiveMenuStep(),0);
        } else if(mode == 1) {
          lcd.setCursor(0,3);
        } else if (mode == 2) {
          lcd.setCursor(11,3);
        }
      } else{
        if(mode == 1) { //note
          seq.noteUp();
        }
        if(mode == 2) { //velo
          seq.setVelocityUp();
        }
        if (mode == 0){//cursor
          seq.nextStep();
        }
      }
    } else if(newPosition < oldPosition - 1){
        //turn left
        if (buttonState == HIGH){
          mode--;
          if(mode == 255) {//mode
            mode = 2;
          }
        } else{
          if(mode == 1) { //note
            seq.noteDown();
          }
          if(mode == 2) { //velo
            seq.setVelocityDown();
          }
          if (mode == 0){//cursor
            seq.prevStep();
          }
        }
    }
    oldPosition = newPosition;
    updateDisplay();
  }
  if (buttonState == HIGH && buttonPressed == false) {
    seq.setGate();
    seq.setNote();
    updateDisplay();
    buttonPressed = true;
  } else if(buttonState == LOW) {
    buttonPressed = false;
  }
}

void displaySequencer::startingAnimation(){
  // initialize and clear display
  if(debugDisplay) {
    Serial.print("startingAnimation");
  }
  // display a line of text
  lcd.setCursor(0,0);
  lcd.print("SEQ v3.0.3 #23.23");
  lcd.setCursor(0,1);
  lcd.print("MIDI-channel ");
  lcd.print(MIDI_CHANNEL);

  // update display with all of the above graphics
  //display.display();
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
