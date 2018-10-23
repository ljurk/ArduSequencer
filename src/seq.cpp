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
    }
  }
  chan[0].note = 0;
  chan[1].note = 1;
  chan[2].note = 2;
  chan[3].note = 3;
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

int sequencer::getActiveStep(){
  return activeStep;
}

bool sequencer::getGate(byte channel, byte pos){
  return chan[channel].gate[pos];
}

byte sequencer::getNote(byte channel/*,int pos*/) {
  return chan[channel].note/*s[pos]*/;
}
byte sequencer::getVelocity(byte channel, byte pos) {
  return chan[channel].velocity[pos];
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

void sequencer::setVelocityUp() {
  if(chan[activeChannel].velocity[activeMenuStep] == 128) {
    chan[activeChannel].velocity[activeMenuStep] = 0;
  } else {
    chan[activeChannel].velocity[activeMenuStep]++;
  }
}

void sequencer::setVelocityDown() {
  if(chan[activeChannel].velocity[activeMenuStep] == 255) {
    chan[activeChannel].velocity[activeMenuStep] = 127;
  } else{
    chan[activeChannel].velocity[activeMenuStep]--;
  }
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
  if(activeStep == STEP_LENGTH - 1) {
    activeStep = 0;
  } else {
    activeStep++;
  }
  if(activeStep == 0) {
    oldStep = STEP_LENGTH - 1;
  } else {
    oldStep = activeStep - 1;
 }
 //send noteOff if previous step sended Note
  if(chan[channel].gate[oldStep]) {
    sendNoteOff(chan[channel].note);
    if (seqDebug) {
      Serial.print(oldStep);
      Serial.print("OFF");
    }
   }
   if(chan[channel].gate[activeStep] == true) {
     if (seqDebug) {
       Serial.print(activeStep);
       Serial.print("ON");
     }
     sendNoteOn(chan[channel].note,chan[channel].velocity[activeStep]);
   }
}

//sequencer functions that should connected to midi sginals
void sequencer::start() {
  activeStep = 0;
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
  activeStep=0;
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
