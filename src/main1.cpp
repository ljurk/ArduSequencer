// ---------------------------------------------------------------------------
//
// basic.ino
//
// A MIDI sequencer example using a standard MIDI cable and a push button
// attached to pin 4.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "FifteenStep.h"
#include <MIDI.h>

// sequencer init
FifteenStep seq = FifteenStep();
MIDI_CREATE_DEFAULT_INSTANCE();
// save button state
bool debug = true;
byte setPin = 12;
byte btnPin = 13;
byte playPin = 11;
byte stepSize = 8;
bool nextButtonPressed = false;
bool setButtonPressed = false;
bool playButtonPressed = false;
bool stopped = false;
bool gate[8];
bool transported[8];

int activeStep= -1;
int oldcurrent=0;
char   buffer[20];

void midiEvent(byte channel, byte command, byte arg1, byte arg2) {

  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }
  // send MIDI data
  if(!debug) {
    Serial.write(command);
    Serial.write(arg1);
    Serial.write(arg2);
  } else {
    Serial.println("MIDI");
    Serial.println(command);
    Serial.println(arg1);
    Serial.println(arg2);
  }
  //MIDI.sendNoteOn(42, 127, channel);
}

void blinkPin(byte pin, byte maxPin) {
  digitalWrite(pin+2,HIGH);
  digitalWrite(maxPin+2,LOW);
  for(int i = 0; i < stepSize; i++) {
    if(gate[i] == true) {
      digitalWrite(i+2,HIGH);
    } else {
      if(i != activeStep) {
        digitalWrite(i+2,LOW);
      }
    }
  }
}

void nextStep() {
  int old = activeStep;
  if(activeStep < stepSize - 1) {
    activeStep++;
  }else{
    activeStep = 0;
  }
  if(debug){
    sprintf(buffer,"active %d",activeStep);
    Serial.println(buffer);
  }
  blinkPin(activeStep,old);
  if(gate[activeStep] && stopped) {
    if(debug){
      Serial.print(activeStep);
    }
    midiEvent(0x0, 0x9, 0x3C, 0x40);
  }
}
void step(int current, int last) {
  if(activeStep = (-1)) {
    activeStep = last;
  }
  // blink on even steps
  //blinkPin(current,last);
  if(debug){
    sprintf(buffer,"current %d",current);
    Serial.println(buffer);
  }
  if(current != oldcurrent){
    nextStep();
    if(transported[current] == false) {
      if(gate[current] == true) {
        if(debug){
          sprintf(buffer,"setNote %d",current);
          Serial.println(buffer);

        }
        seq.setNote(0x0, 0x3C, 0x40);
      }else {
        sprintf(buffer,"delNote %d",current);
        Serial.println(buffer);
        seq.setNote(0x0, 0x0, 0x0);
        //seq.setNote(0x0, 0x3C, 0x40);
      }
      transported[current]= true;
    }
    oldcurrent =current;
  }
}



void setup() {

  MIDI.begin(MIDI_CHANNEL_OMNI);
  // set MIDI baud
  if(debug) {
    Serial.begin(9600);
  }else{
    Serial.begin(31250);
  }
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6 ,OUTPUT);
  pinMode(7 ,OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

for(int i=0;i<stepSize;i++) {
  gate[i] = false;
  transported[i] = true;
}
  // initialize digital pin 4 as an input for a button
  pinMode(btnPin, INPUT);
  pinMode(setPin,INPUT);

  // start sequencer and set callbacks
  seq.begin(80,stepSize);
  seq.setMidiHandler(midiEvent);
  seq.setStepHandler(step);

}

void loop() {
  // read the state of the button
  int nextbutton = digitalRead(btnPin);
  int setbutton = digitalRead(setPin);
  int playbutton = digitalRead(playPin);
  // check for button press or release and
  // send note on or off to seqencer if needed
  if(playbutton == HIGH && playButtonPressed == false) {
    playButtonPressed = true;
    // button pressed. play middle C preview now
  //  midi(0x0, 0x9, 0x3C, 0x40);
    // store note in sequence
    //seq.setNote(0x0, 0x3C, 0x40);
    if(stopped == false) {
      seq.stop();
      stopped = true;
    } else {
      seq.start();
      stopped = false;
    }

  }
  if(playbutton == LOW) {
    playButtonPressed = false;
  }
  if(nextbutton == LOW && setbutton == HIGH && setButtonPressed == false) {
    midiEvent(0x0, 0x9, 0x3C, 0x40);
    gate[activeStep] = !gate[activeStep];
    transported[activeStep] = !transported[activeStep];
    if(debug) {
      Serial.println(activeStep);
      Serial.println(gate[activeStep]);
    }
    setButtonPressed = true;
    // button released. send middle C note off preview now

    // store note off in sequence
  //  seq.setNote(0x0, 0x3C, 0x0);

}
if(setbutton ==LOW) {
  if(setButtonPressed) {
    //midiEvent(0x0, 0x8, 0x3C, 0x0);
  }
  //
  setButtonPressed = false;
}
if(nextbutton == HIGH && setbutton == LOW && nextButtonPressed==false ) {
  nextButtonPressed = true;
  if(debug) {
    Serial.println("PRESS NEXT");
  }
  nextStep();
}
if(nextbutton == LOW) {
  nextButtonPressed = false;
}

  // save button state

  // this is needed to keep the sequencer
  // running. there are other methods for
  // start, stop, and pausing the steps
  seq.run();

}
