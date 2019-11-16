#include "../lib/seq.hpp"

sequencer::sequencer(bool dbg) {
    if (dbg == true) {
        seqDebug = true;
    }
    // initialize arrays
    for (int y = 0; y < NUMBER_OF_CHANNELS; y++) {
        for (int i = 0; i  < STEP_LENGTH; i++) {
            chan[y].gate[i] = false;
            chan[y].slide[i] = false;
            chan[y].velocity[i] = DEFAULT_VELOCITY;
            chan[y].length = STEP_LENGTH;
            chan[y].numberOfGates = 0;
        }
        chan[y].activeStep = 0;
        chan[y].oldStep = 0;
        chan[y].mute = false;
    }
    // mfb
    chan[0].note = 36;
    chan[0].noteText = "KK";
    chan[1].note = 39;
    chan[1].noteText = "CP";
    chan[2].note = 46;
    chan[2].noteText = "HH";
    chan[3].note = 38;
    chan[3].noteText = "SD";
    // microgranny
    /*chan[0].note = 0;
    chan[0].noteText = "S1";
    chan[1].note = 1;
    chan[1].noteText = "S2";
    chan[2].note = 2;
    chan[2].noteText = "S3";
    chan[3].note = 3;
    chan[3].noteText = "S4";*/
}

bool sequencer::getSlideActive() {
    return slideActive;
}

bool sequencer::getStopped() {
    return stopped;
}

int sequencer::getDefaultNote() {
    return defaultNote;
}

byte sequencer::getCursorPos() {
    return cursorPos;
}

byte sequencer::getActiveChannel() {
    return activeChannel;
}

byte sequencer::getActiveStep(byte channel) {
    return chan[channel].activeStep;
}

byte sequencer::getOldStep(byte channel) {
    return chan[channel].oldStep;
}

bool sequencer::getGate(byte channel, byte pos) {
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

bool sequencer::getMute(byte channel) {
    return chan[channel].mute;
}

void sequencer::defaultNoteUp() {
    if (defaultNote != 0) {
        defaultNote -= 1;
    }
}

void sequencer::defaultNoteDown() {
    defaultNote += 1;
}

void sequencer::setNoteDown() {
    if (chan[activeChannel].note != 0) {
        chan[activeChannel].note -= 1;
    }
}

void sequencer::setNoteUp() {
    chan[activeChannel].note += 1;
}

void sequencer::setVelocityUp(int steps) {
    if (chan[activeChannel].velocity[cursorPos] >= 127) {
        chan[activeChannel].velocity[cursorPos] = 0;
    } else {
        chan[activeChannel].velocity[cursorPos] += steps;
    }
}

void sequencer::setVelocityDown(int steps) {
    if (chan[activeChannel].velocity[cursorPos] == 0 || \
       (chan[activeChannel].velocity[cursorPos] <= 255 && \
        chan[activeChannel].velocity[cursorPos] > 127) ) {
    chan[activeChannel].velocity[cursorPos] = 127;
    } else{
        chan[activeChannel].velocity[cursorPos] -= steps;
    }
}

void sequencer::setLength(int steps) {
    chan[activeChannel].length = steps;
}

void sequencer::setGate() {
    chan[activeChannel].gate[cursorPos] = ! chan[activeChannel].gate[cursorPos];
}

void sequencer::euclidAddGate(){
    if (seqDebug) {
        Serial.print("add euclidean note");
    }
    resetSequence();
    chan[activeChannel].numberOfGates += 1;
    for (byte i = 0; i < chan[activeChannel].numberOfGates; i++) {
        byte step = i * ( chan[activeChannel].length / chan[activeChannel].numberOfGates);
        chan[activeChannel].gate[step] = true;
    }

}
void sequencer::euclidRemoveGate(){
    if (seqDebug) {
        Serial.print("add euclidean note");
    }
    resetSequence();
    chan[activeChannel].numberOfGates -= 1;
    for (byte i = 0; i < chan[activeChannel].numberOfGates; i++) {
        byte step = i * ( chan[activeChannel].length / chan[activeChannel].numberOfGates);
        chan[activeChannel].gate[step] = true;
    }
}

void sequencer::setNote() {
  // chan[channel].notes[activeStep] = defaultNote;
}

void sequencer::setSlide() {

}

void sequencer::setActiveChannel(byte channel) {
    activeChannel = channel;
}

void sequencer::setMute(byte channel) {
    chan[channel].mute = !chan[channel].mute;
}
void sequencer::resetSequence() {
    for (byte y = 0; y < NUMBER_OF_CHANNELS; y++) {
        for (byte i = 0; i < STEP_LENGTH;i++) {
            chan[activeChannel].gate[i] = false;
            chan[activeChannel].slide[i] = false;
        }
    }
}

void sequencer::setCursorPos(bool direction) {// true = forwards, false = backwards
    if (direction == NEXT) {
        if (cursorPos == chan[activeChannel].length - 1) {
            cursorPos = 0;
        } else {
            cursorPos++;
        }
    } else {
        if (cursorPos == 0) {
            cursorPos = chan[activeChannel].length - 1;
        } else {
            cursorPos--;
        }
    }
    if (seqDebug) {
        Serial.println("setCursor");
        Serial.println(cursorPos);
    }
}

void sequencer::setCursorPosDirect(byte pos) {
    cursorPos = pos;
}

// will be called from clock
void sequencer::step(byte channel) {
    chan[channel].oldStep  = chan[channel].activeStep;
    if (chan[channel].activeStep == chan[channel].length - 1 ) {
        chan[channel].activeStep = 0;
    } else {
        chan[channel].activeStep++;
    }
/*  if (chan[channel].activeStep == 0) {
    chan[channel].oldStep = chan[channel].length - 1;
  } else {
    chan[channel].oldStep = chan[channel].activeStep - 1;
 }*/

    // send noteOff if previous step sended Note
    if (chan[channel].mute == false) {
        if (chan[channel].gate[chan[channel].oldStep]) {
            sendNoteOff(chan[channel].note);
            if (seqDebug) {
                Serial.print(chan[channel].oldStep);
                Serial.print("OFF");
            }
        }
        if (chan[channel].gate[chan[channel].activeStep] == true) {
            if (seqDebug) {
                Serial.print(chan[channel].activeStep);
                Serial.print("ON");
            }
        sendNoteOn(chan[channel].note,chan[channel].velocity[chan[channel].activeStep]);
        }
    }
}

// sequencer functions that should connected to midi sginals
void sequencer::start() {
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        chan[i].activeStep = 0;
        chan[i].oldStep = 0;
    }
    stopped = false;
}

void sequencer::stop() {
    stopped = true;
    /*if (gate[oldStep]) {
      sendNoteOff(notes[oldStep]);
    }
    if (gate[activeStep]) {
      sendNoteOff(notes[activeStep]);
    }*/
    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        chan[i].activeStep = 0;
        chan[i].oldStep = 0;
    }
    count = 0;
}

void sequencer::cont() {
    stopped = false;
}

void sequencer::clock() {
    if (!stopped) {
        count++;
        if (count == (24 / speedDivider)) {
            for (int i= 0; i< NUMBER_OF_CHANNELS; i++) {
                step(i);
            }
            count = 0;
        }
    }
}
