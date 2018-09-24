/*<><><><><><><><><<<code by Lukas Jurk>>><><><><><><><><>*/
/*<><><><><><><><><<<version 0.23>>><><><><><><><><>*/
/*<><><><><><><><><<<303>>><><><><><><><><>*/
#include <Arduino.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"

#define STEP_LENGTH 8
//for midi in

//true disables midi, and writes debug messages on 9600 baud
bool debugON = false;

//buttons
bool nextPrevButtonPressed = false;
bool setSlideButtonPressed = false;
bool noteUpDownButtonPressed = false;
bool funcButtonPressed = false;

bool nextPrevButtonState = false;
bool setSlideButtonState = false;
bool noteUpDownButtonState = false;
bool funcButtonState = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
int debounceDelay = 150;
//time to let choosen step blink without delay
unsigned int time;
unsigned int oldTime;

byte ledPins[STEP_LENGTH] = {9,8,7,6,5,4,3,2};

bool activeMenuLedState = true;
char buffer[20];
sequencer seq = sequencer(debugON);
void startingAnimation(){
  if(debugON) {
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
void pressNext() {
  if(debugON) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getOldMenuStep())) {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void pressPrev(){
  if(debugON) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getOldMenuStep()))  {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void showSequence() {
  for(int i = 0; i < STEP_LENGTH; i++) {
   if(i != seq.getActiveMenuStep()) {
      if(seq.getGate(i)) {
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
void blinkPin(byte blink, byte unblink) {
  digitalWrite(ledPins[blink],HIGH);
  if(seq.getGate(unblink) == false) {
    digitalWrite(ledPins[unblink],LOW);
  }
}

void activeMenuBlink(){
  time = millis();
  if(time > oldTime + BLINK_TIME) {
    if(debugON) {
      seq.clock();
    }
    if(debugON) {
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

void checkButtons(){
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
      if(debugON) {
        Serial.println("NoteDown");
      }
      if(seq.getStopped()) {
        seq.defaultNoteDown();
      }
      seq.noteDown();
    } else {
      //NoteUp
      if(debugON) {
        Serial.println("NoteUp");
      }
      if(seq.getStopped()) {
        seq.defaultNoteUp();
      }
      seq.noteUp();
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
        if(debugON) {
          Serial.println("NOTE ON");
        }
        sendMidi(MIDI_CHANNEL, NOTE_ON, seq.getDefaultNote(), DEFAULT_VELOCITY);
        showSequence();
      }
      showSequence();
      seq.setGate();
      digitalWrite(ledPins[seq.getActiveMenuStep()],seq.getGate(seq.getActiveMenuStep()));
      seq.setNote();
    }
    setSlideButtonPressed = true;
  }
  if(seq.getStopped() && setSlideButtonState == LOW && setSlideButtonPressed == true) {
    lastDebounceTime = millis();
    if(debugON) {
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
      if(debugON) {
        Serial.println("PRESS PREV");
      }
      seq.prevStep();
      pressPrev();
    }else{
      //next Step
      if(debugON) {
        if(debugON) {
          sprintf(buffer,"next %ld",lastDebounceTime);
          Serial.println(buffer);
        }
      }
      seq.nextStep();
      pressNext();
    }
    nextPrevButtonPressed = true;
  }
  if(nextPrevButtonState == LOW) {
    nextPrevButtonPressed = false;
  }
}

void setup() {
  time =millis();
  oldTime=time;
  if(debugON) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }
  for(int i = 0; i  <STEP_LENGTH; i++) {
    pinMode(ledPins[i],OUTPUT);

  }
  startingAnimation();

  // initialize button pins
  pinMode(NEXT_PREV_PIN, INPUT);
  pinMode(SET_SLIDE_PIN,INPUT);
  pinMode(NOTE_UP_DOWN_PIN, INPUT);
  pinMode(FUNC_PIN, INPUT);
  lastDebounceTime = millis();
  if(debugON){
    seq.start();
  }

}

void loop() {
  if(millis() - debounceDelay > lastDebounceTime) {
    checkButtons();
  }
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
