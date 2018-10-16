#include "../lib/seq.hpp"

sequencer::sequencer(bool dbg) {
  if(dbg == true) {
    seqDebug = true;
  }
  //initialize arrays
  for(int y = 0; y < NUMBER_OF_CHANNELS; y++) {
    for(int i = 0; i  < STEP_LENGTH; i++) {
      chan[y].gate[i] = false;
      chan[y].notes[i] = getDefaultNote();
      chan[y].slide[i] = false;
      chan[y].velocity[i] = DEFAULT_VELOCITY;
    }
  }
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

int sequencer::getActiveStep(){
  return activeStep;
}

bool sequencer::getGate(byte channel, int pos){
  return chan[channel].gate[pos];
}

byte sequencer::getNote(byte channel,int pos) {
  return chan[channel].notes[pos];
}
byte sequencer::getVelocity(byte channel,int pos) {
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

void sequencer::noteDown(byte channel){
  if(chan[channel].notes[activeMenuStep] != 0) {
    chan[channel].notes[activeMenuStep] -= 1;
  }
}

void sequencer::noteUp(byte channel){
  chan[channel].notes[activeMenuStep] += 1;
}

void sequencer::setVelocityUp(byte channel) {
  if(chan[channel].velocity[activeMenuStep] == 128) {
    chan[channel].velocity[activeMenuStep] = 0;
  } else {
    chan[channel].velocity[activeMenuStep]++;
  }
}

void sequencer::setVelocityDown(byte channel) {
  if(chan[channel].velocity[activeMenuStep] == 255) {
    chan[channel].velocity[activeMenuStep] = 127;
  } else{
    chan[channel].velocity[activeMenuStep]--;
  }
}
void sequencer::setGate(byte channel) {
  chan[channel].gate[activeMenuStep] = ! chan[channel].gate[activeMenuStep];
}

void sequencer::setNote(byte channel){
  chan[channel].notes[activeStep] = defaultNote;
}
void sequencer::setSlide(byte channel){

}

void sequencer::resetSequence(byte channel){
  for(byte i = 0; i < STEP_LENGTH;i++) {
    chan[channel].gate[i] = false;
    chan[channel].slide[i] = false;
    chan[channel].notes[i] =   getDefaultNote();
  //  digitalWrite(ledPins[i], LOW);
  }
}
//navigation
void sequencer::nextStep() {
  oldMenuStep= activeMenuStep;
  if(activeMenuStep == STEP_LENGTH - 1) {
    activeMenuStep = 0;
  } else {
    activeMenuStep++;
  }

  /*if(seqDebug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }*/
}

void sequencer::prevStep() {
  oldMenuStep= activeMenuStep;
  if(activeMenuStep == 0) {
    activeMenuStep = STEP_LENGTH - 1;
  } else {
    activeMenuStep--;
  }
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

  if(chan[channel].gate[oldStep]) {
    sendNoteOff(chan[channel].notes[oldStep]);
    if (seqDebug) {
      Serial.print(oldStep);
      Serial.print("OFF");
    }
   }
 /* SLIDE IS COMING LATER
 if(slideActive) {
   if(gate[oldStep] && !slide[activeStep]) {
     //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[oldStep], 0);
   }
 } else {
 if(sequencer::slide[oldStep]) {
    sequencer::lastNoteStep = sequencer::oldStep - 1;
    while(sequencer::gate[sequencer::lastNoteStep] == false && sequencer::slide[sequencer::lastNoteStep + 1] == true) {
      if(sequencer::lastNoteStep == 0) {
        sequencer::lastNoteStep = STEP_LENGTH - 1;
      } else {
        sequencer::lastNoteStep -= 1;
      }
    }
    //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[lastNoteStep], 0);
  */
 if(chan[channel].gate[activeStep] == true) {
   if (seqDebug) {
     Serial.print(activeStep);
     Serial.print("ON");
   }
   sendNoteOn(chan[channel].notes[activeStep],DEFAULT_VELOCITY);
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
