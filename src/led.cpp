/*<><><><><><><><><<<code by Lukas Jurk>>><><><><><><><><>*/
/*<><><><><><><><><<<version 0.23>>><><><><><><><><>*/
/*<><><><><><><><><<<303>>><><><><><><><><>*/

/*

#include "..\lib\led.hpp"

#define STEP_LENGTH 8
//#define MIDI_CHANNEL 10
//buttons
#define SET_SLIDE_PIN  11
#define FUNC_PIN 12
#define  NOTE_UP_DOWN_PIN 13
#define NEXT_PREV_PIN 10
#define BLINK_TIME 150

led::led(bool debug){
  seq = sequencer(debug);
  time =millis();
  oldTime=time;
  for(int i = 0; i  <STEP_LENGTH; i++) {
    pinMode(ledPins[i],OUTPUT);

  }
  // initialize button pins
  pinMode(NEXT_PREV_PIN, INPUT);
  pinMode(SET_SLIDE_PIN,INPUT);
  pinMode(NOTE_UP_DOWN_PIN, INPUT);
  pinMode(FUNC_PIN, INPUT);
  lastDebounceTime = millis();

}
void led::startingAnimation(){
  if(debugLed) {
    Serial.print("starting...");
  }
  for(int i = 0; i  <STEP_LENGTH; i++) {
    digitalWrite(ledPins[i],HIGH);
    delay(500);
  }
  for(int h = 0; h  <5; h++) {
    for(int i = 0; i  <STEP_LENGTH; i++) {
      digitalWrite(ledPins[i],HIGH);
    }
    delay(200);
    for(int i = 0; i  <STEP_LENGTH; i++) {
      digitalWrite(ledPins[i],LOW);
    }
    delay(200);
  }
}
void led::pressNext() {
  if(debugLed) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getActiveChannel(), seq.getOldMenuStep())) {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void led::pressPrev(){
  if(debugLed) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getActiveChannel(), seq.getOldMenuStep()))  {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void led::showSequence() {
  for(int i = 0; i < STEP_LENGTH; i++) {
   if(i != seq.getActiveMenuStep()) {
      if(seq.getGate(seq.getActiveChannel(), i)) {
        if(i == seq.getActiveStep()) {
            digitalWrite(ledPins[i], LOW);
          } else {
            digitalWrite(ledPins[i],HIGH);
          }
      } else {
        if(i == seq.getActiveStep()) {
            digitalWrite(ledPins[i], HIGH);
          } else {
            digitalWrite(ledPins[i],LOW);
          }
      }
    }
  }
}
void led::blinkPin(byte blink, byte unblink) {
  digitalWrite(ledPins[blink],HIGH);
  if(seq.getGate(seq.getActiveChannel(), unblink) == false) {
    digitalWrite(ledPins[unblink],LOW);
  }
}

void led::activeMenuBlink(){
  time = millis();
  if(time > oldTime + BLINK_TIME) {
    if(debugLed) {
      seq.clock();
    }
    if(debugLed) {
      //  Serial.println(seq.getActiveMenuStep());
    }

    if(activeMenuLedState == true) {
        digitalWrite(ledPins[seq.getActiveMenuStep()],LOW);
        activeMenuLedState = false;
    } else {
      digitalWrite(ledPins[seq.getActiveMenuStep()],HIGH);
      activeMenuLedState = true;
    }
    oldTime = time;
  }
}

void led::checkButtons(){
  if(millis() - debounceDelay > lastDebounceTime) {
    // read the state of the buttons
    nextPrevButtonState = digitalRead(NEXT_PREV_PIN);
    setSlideButtonState = digitalRead(SET_SLIDE_PIN);
    noteUpDownButtonState = digitalRead(NOTE_UP_DOWN_PIN);
    funcButtonState = digitalRead(FUNC_PIN);

    //if all buttons are pressed reset the seuqence
    if(nextPrevButtonState && setSlideButtonState && noteUpDownButtonState && funcButtonPressed) {
      seq.resetSequence();
      lastDebounceTime =millis();
    }
    //check noteUpDown
    if(noteUpDownButtonState == HIGH && noteUpDownButtonPressed == false) {
      lastDebounceTime = millis();
      if(funcButtonState == true) {
        //NoteDown
        if(debugLed) {
          Serial.println("NoteDown");
        }
        if(seq.getStopped()) {
          seq.defaultNoteDown();
        }
        seq.setNoteDown();
      } else {
        //NoteUp
        if(debugLed) {
          Serial.println("NoteUp");
        }
        if(seq.getStopped()) {
          seq.defaultNoteUp();
        }
        seq.setNoteUp();
      }
      noteUpDownButtonPressed = true;
    }
    if(noteUpDownButtonState == LOW) {
      noteUpDownButtonPressed = false;
    }
    //check setSlideButton
    if(setSlideButtonState == HIGH && setSlideButtonPressed == false) {
      lastDebounceTime = millis();
      if(funcButtonState == true && seq.getSlideActive() == true) {
        //set slide for activeMenuStep
        seq.setSlide();
        //slide[activeMenuStep] = !slide[activeMenuStep];
      }else{
        //set
        if(seq.getStopped()) {
          if(debugLed) {
            Serial.println("NOTE ON");
          }
          sendMidi(MIDI_CHANNEL, NOTE_ON, seq.getDefaultNote(), DEFAULT_VELOCITY);
          showSequence();
        }
        showSequence();
        seq.setGate();
        digitalWrite(ledPins[seq.getActiveMenuStep()],seq.getGate(seq.getActiveChannel(), seq.getActiveMenuStep()));
        seq.setNote();
      }
      setSlideButtonPressed = true;
    }
    if(seq.getStopped() && setSlideButtonState == LOW && setSlideButtonPressed == true) {
      lastDebounceTime = millis();
      if(debugLed) {
        Serial.println("NOTE OFFN");
      }
      sendMidi(MIDI_CHANNEL, NOTE_ON, seq.getDefaultNote(), 0);
      setSlideButtonPressed = false;
    }
    if(setSlideButtonState == LOW) {
      setSlideButtonPressed = false;
    }
    //check nextPrev
    if(nextPrevButtonState == HIGH && nextPrevButtonPressed == false ) {
      lastDebounceTime = millis();
      if(funcButtonState == true) {
        //previous Step
        if(debugLed) {
          Serial.println("PRESS PREV");
        }
        seq.setCursor(PREV);
        pressPrev();
      }else{
        //next Step
        if(debugLed) {
          if(debugLed) {
            sprintf(buffer,"next %ld",lastDebounceTime);
            Serial.println(buffer);
          }
        }
        seq.setCursor(NEXT);
        pressNext();
      }
      nextPrevButtonPressed = true;
    }
    if(nextPrevButtonState == LOW) {
      nextPrevButtonPressed = false;
    }
  }
}

void led::run(){
  checkButtons();
  activeMenuBlink();

  if(Serial.available()  > 0) {
    byte byte_read = Serial.read();
    switch(byte_read) {
      case MIDI_START:
          seq.start();
        break;
      case MIDI_STOP:
        seq.stop();
        blinkPin(0, seq.getActiveStep());
        break;
      case MIDI_CONT:
        seq.cont();
        break;
      case MIDI_CLOCK:
          showSequence();
          seq.clock();
        break;
      default:
        break;
    }
  }
}
*/
