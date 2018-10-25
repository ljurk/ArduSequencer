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
   lcd.blink();
   pinMode(buttonPin,INPUT);
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
    //display.setTextSize(1);
}

void displaySequencer::updateValues(){
  //display.setTextSize(1);
  String outNote;
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
      lcd.setCursor(0,i);

      outNote = "C";
      outNote += String(seq.getNote(i));
      if(i == seq.getActiveChannel()) {
        outNote += ">";
        //outNote= "V";
        //outNote += String(seq.getVelocity(seq.getActiveChannel(),seq.getActiveMenuStep()));
      } else {
        outNote += " ";
      }

      lcd.print(outNote);
  }
  /*byte veloPos = seq.getActiveChannel() + 1;
  if(veloPos == NUMBER_OF_CHANNELS) {
    veloPos = 0;
  }
  String outVelo= " VELO ";
  outVelo += String(seq.getVelocity(seq.getActiveChannel(),seq.getActiveMenuStep()));
  //display.setCursor(0,48);

  lcd.setCursor(10,veloPos);
  lcd.print(outVelo);*/
  //display.setTextColor(WHITE);
}

void displaySequencer::updateActiveStep(){

}
void displaySequencer::updateSequence(){
  //displays sequence for each channel
  for(int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(int i = 0; i  <STEP_LENGTH; i++) {
        if(seq.getGate(y, i) == true) {
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
/*  if(debugDisplay) {
    Serial.print("update Display");
  }
*/
  //updateCursor();
  updateSequence();
  updateValues();
  //after everything ist displayed change the cursor position
  //to active element
  //if(mode == 0) {//sequence 0
    lcd.setCursor(seq.getActiveMenuStep() + DISPLAY_OFFSET,seq.getActiveChannel());
  /*}*else if(mode == NUMBER_OF_CHANNELS) {//note
    lcd.setCursor(0,3);
  } *else */if (mode == NUMBER_OF_CHANNELS + 1) { //velocity
    lcd.setCursor(0,seq.getActiveChannel());
  }
  delay(100);
}

void displaySequencer::checkInputs(){
  newPosition = myEnc.read();
  encoderButtonState = digitalRead(encoderButtonPin);
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    channelButtonStates[i] = digitalRead(channelPins[i]);
  }
  if (newPosition != oldPosition) {
    if(newPosition > oldPosition + 1) {
      //turn right
    /*  if (buttonState == HIGH) {
        mode++;
        if(mode == NUMBER_OF_CHANNELS) {//mode
          mode = 0;
        }
        if(mode < NUMBER_OF_CHANNELS) {//sequence 0
          seq.setActiveChannel(mode);
          lcd.setCursor(seq.getActiveMenuStep(),seq.getActiveChannel());
        }/*else if(mode == NUMBER_OF_CHANNELS) {//note
          lcd.setCursor(0,3);
        } else if (mode == NUMBER_OF_CHANNELS + 1) { //velocity
          lcd.setCursor(11,3);
        }*
      } else{*/
        /*if(mode == NUMBER_OF_CHANNELS) { //note
          seq.setNoteUp();
        }
        if(mode == NUMBER_OF_CHANNELS + 1) { //velo
          seq.setVelocityUp();
        }*/
        if (mode == 0 ){//cursor
          seq.setCursor(NEXT);
        }
    //  }
    } else if(newPosition < oldPosition - 1){
        //turn left
        /*if (buttonState == HIGH){
          mode--;
          if(mode == 255) {//mode
            mode = 2;
          }
        } else{
          if(mode == NUMBER_OF_CHANNELS) { //note
            seq.setNoteDown();
          }
          if(mode == NUMBER_OF_CHANNELS + 1) { //velo
            seq.setVelocityDown();
          }*/
          if (mode == 0){//cursor
            seq.setCursor(PREV);
          }
        //}
    }
    oldPosition = newPosition;
  }
  if (encoderButtonState == HIGH && encoderButtonPressed == false) {
    Serial.println("PRESS");
    seq.setGate();
    seq.setNote();
    //updateDisplay();
    encoderButtonPressed = true;
  } else if(encoderButtonState == LOW) {
    encoderButtonPressed = false;
  }
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    if (channelButtonStates[i] == HIGH && channelButtonPressed[i] == false) {
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
