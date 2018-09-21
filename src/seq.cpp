#include "../lib/seq.hpp"

int count = 0; // counter for midi ticks, 24 ticks are one quarter note
byte speedDivider = 1; //1=24ticks,2=12ticks,4=6ticks
byte defaultNote =  0;
bool stopped = true;
bool gate[STEP_LENGTH];
byte notes[STEP_LENGTH];
bool slide[STEP_LENGTH];
byte activeStep= 0;
byte oldStep= 0;
byte oldMenuStep= 0;
byte activeMenuStep=0;
bool slideActive = false;

sequencer::sequencer() {
  //initialize arrays
  for(int i = 0; i  <STEP_LENGTH; i++) {
    gate[i] = false;
    notes[i] = getDefaultNote();
    slide[i] = false;
  }
}

bool sequencer::getSlideActive() {
  return slideActive;
}

bool sequencer::getStopped(){
  return stopped;
}

void sequencer::lowerDefaultNote(){
  if(defaultNote != 0) {
    defaultNote -= 1;
  }
}
void sequencer::raiseDefaultNote(){
    defaultNote += 1;
}

void sequencer::noteDown(){
  if(notes[activeStep] != 0) {
    notes[activeStep] += 1;
  }
}
void sequencer::noteUp(){
  notes[activeStep] += 1;
}
byte sequencer::getActiveMenuStep(){
  return activeMenuStep;
}
int sequencer::getActiveStep(){
  return activeStep;
}

bool sequencer::getGate(int pos){
  return gate[pos];
}

void sequencer::setGate() {
  gate[activeStep] = ! gate[activeStep];
}
void sequencer::setNote(){
  notes[activeStep] = defaultNote;
}

int sequencer::getDefaultNote() {
  return defaultNote;
}


void sequencer::resetSequence(){
  for(byte i = 0; i < STEP_LENGTH;i++) {
    gate[i] = false;
    slide[i] = false;
    notes[i] =   getDefaultNote();
  //  digitalWrite(ledPins[i], LOW);
  }
}

void sequencer::nextStep() {
  oldMenuStep= activeMenuStep;
  if(activeMenuStep == STEP_LENGTH - 1) {
    activeMenuStep = 0;
  } else {
    activeMenuStep++;
  }
  /*
  if(debug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }
  if(gate[oldMenuStep]) {
    digitalWrite(ledPins[oldMenuStep],HIGH);
  } else {
    digitalWrite(ledPins[oldMenuStep],LOW);
  }*/
}

void sequencer::prevStep() {
  oldMenuStep= activeMenuStep;
  if(activeMenuStep == 0) {
    activeMenuStep = STEP_LENGTH - 1;
  } else {
    activeMenuStep--;
  }
  /*if(debug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }
  if(gate[oldMenuStep]) {
    digitalWrite(ledPins[oldMenuStep],HIGH);
  } else {
    digitalWrite(ledPins[oldMenuStep],LOW);
  }*/
}

void sequencer::step() {
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
 if(slideActive) {
   if(gate[oldStep] && !slide[activeStep]) {
     //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[oldStep], 0);
   }
 } else {
   if(gate[oldStep]) {
     //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[oldStep], 0);
   }
 }
  /* if(sequencer::slide[oldStep]) {
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
 }

 //blinkPin(activeStep,oldStep);
 //if(sequencer::gate[activeStep] == true) {
   //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[activeStep], DEFAULT_VELOCITY);
// }
//}

void sequencer::start() {
  activeStep = 0;
  stopped = false;
}

void sequencer::stop(){
  stopped = true;
  if(gate[oldStep]) {
    sendMidi(MIDI_CHANNEL, NOTE_ON, notes[oldStep], 0);
  }
  if(gate[activeStep]) {
    sendMidi(MIDI_CHANNEL, NOTE_ON, notes[activeStep], 0);
  }
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
      step();
      count = 0;
    }
  }
}
