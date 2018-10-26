#include "../lib/seq.hpp"

sequencer::sequencer(bool dbg) {
  if(dbg == true) {
    seqDebug = true;
  }
  //initialize arrays
  for(int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(int i = 0; i  < STEP_LENGTH; i++) {
      chan[y].gate[i] = false;
      //chan[y].notes[i] = getDefaultNote();
      chan[y].slide[i] = false;
      chan[y].velocity[i] = DEFAULT_VELOCITY;
      chan[y].length = STEP_LENGTH;
    }
  }
  chan[0].note = 38;
  chan[0].noteText = "C1";
  chan[1].note = 39;
  chan[1].noteText = "E1";
  chan[2].note = 43;
  chan[2].noteText = "G#1";
  chan[3].note = 45;
  chan[3].noteText = "A#1";

  chan[1].activeStep = 0;
  chan[2].activeStep = 0;
  chan[3].activeStep = 0;
}
byte sequencer::getOldMenuStep(){
  return oldMenuStep;
}
bool sequencer::getSlideActive() {
  return slideActive;
}

bool sequencer::getStopped(){
  return stopped;
}

int sequencer::getDefaultNote() {
  return defaultNote;
}

byte sequencer::getActiveMenuStep(){
  return activeMenuStep;
}

byte sequencer::getActiveChannel(){
  return activeChannel;
}

byte sequencer::getActiveStep(byte channel){
  return chan[channel].activeStep;
}

bool sequencer::getGate(byte channel, byte pos){
  return chan[channel].gate[pos];
}

byte sequencer::getNote(byte channel/*,int pos*/) {
  return chan[channel].note/*s[pos]*/;
}

String sequencer::getNoteText(byte channel/*,int pos*/) {
  return chan[channel].noteText/*s[pos]*/;
}

byte sequencer::getNoteTextLength(byte channel/*,int pos*/) {
  return chan[channel].noteText.length();/*s[pos]*/;
}
byte sequencer::getVelocity(byte channel, byte pos) {
  return chan[channel].velocity[pos];
}

byte sequencer::getLength(byte channel) {
  return chan[channel].length;
}

void sequencer::defaultNoteUp(){
  if(defaultNote != 0) {
    defaultNote -= 1;
  }
}

void sequencer::defaultNoteDown(){
    defaultNote += 1;
}

void sequencer::setNoteDown(){
  if(chan[activeChannel].note != 0) {
    chan[activeChannel].note -= 1;
  }
}

void sequencer::setNoteUp(){
  chan[activeChannel].note += 1;
}

void sequencer::setVelocityUp(int steps) {
  if(chan[activeChannel].velocity[activeMenuStep] >= 127) {
    chan[activeChannel].velocity[activeMenuStep] = 0;
  } else {
    chan[activeChannel].velocity[activeMenuStep] += steps;
  }
}

void sequencer::setVelocityDown(int steps) {
  if(chan[activeChannel].velocity[activeMenuStep] == 0 || (chan[activeChannel].velocity[activeMenuStep] <= 255 && chan[activeChannel].velocity[activeMenuStep] > 127) ) {
    chan[activeChannel].velocity[activeMenuStep] = 127;
  } else{
    chan[activeChannel].velocity[activeMenuStep] -= steps;
  }
}

void sequencer::setLength(int steps) {
  chan[activeChannel].length = steps;
}
void sequencer::setGate() {
  chan[activeChannel].gate[activeMenuStep] = ! chan[activeChannel].gate[activeMenuStep];
}

void sequencer::setNote(){
  //chan[channel].notes[activeStep] = defaultNote;
}
void sequencer::setSlide(){

}
void sequencer::setActiveChannel(byte channel){
  activeChannel = channel;
}
void sequencer::resetSequence(){
  for(byte y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(byte i = 0; i < STEP_LENGTH;i++) {
      chan[activeChannel].gate[i] = false;
      chan[activeChannel].slide[i] = false;
      //chan[channel].notes[i] =   getDefaultNote();
    //  digitalWrite(ledPins[i], LOW);
    }
  }
}
void sequencer::setCursor(bool direction) {//true = forwards, false = backwards
  oldMenuStep = activeMenuStep;
  if(direction == NEXT) {
    if(activeMenuStep == STEP_LENGTH - 1) {
      activeMenuStep = 0;
    } else {
      activeMenuStep++;
    }
  } else {
    if(activeMenuStep == 0) {
      activeMenuStep = STEP_LENGTH - 1;
    } else {
      activeMenuStep--;
    }
  }
  /*if(seqDebug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }*/
}


// will be called from clock
void sequencer::step(byte channel) {
  if(chan[channel].activeStep == chan[channel].length - 1 ) {
    chan[channel].activeStep = 0;
  } else {
    chan[channel].activeStep++;
  }
  if(chan[channel].activeStep == 0) {
    oldStep = chan[channel].length - 1;
  } else {
    oldStep = chan[channel].activeStep - 1;
 }

 //send noteOff if previous step sended Note
  if(chan[channel].gate[oldStep]) {
    sendNoteOff(chan[channel].note);
    if (seqDebug) {
      Serial.print(oldStep);
      Serial.print("OFF");
    }
   }
   if(chan[channel].gate[chan[channel].activeStep] == true) {
     if (seqDebug) {
       Serial.print(chan[channel].activeStep);
       Serial.print("ON");
     }
     sendNoteOn(chan[channel].note,chan[channel].velocity[chan[channel].activeStep]);
   }
}

//sequencer functions that should connected to midi sginals
void sequencer::start() {
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    chan[i].activeStep = 0;
  }
  stopped = false;
}

void sequencer::stop(){
  stopped = true;
  /*if(gate[oldStep]) {
    sendNoteOff(notes[oldStep]);
  }
  if(gate[activeStep]) {
    sendNoteOff(notes[activeStep]);
  }*/
  for(int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    chan[i].activeStep = 0;
  }
  count = 0;
}

void sequencer::cont() {
  stopped = false;
}

void sequencer::clock() {
  if(!stopped) {
    count++;
    if(count == (24 / speedDivider)) {
      for(int i= 0; i< NUMBER_OF_CHANNELS; i++) {
        step(i);
      }
      count = 0;
    }
  }
}
