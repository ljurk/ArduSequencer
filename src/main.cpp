#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"
//#include "..\lib\led.hpp"

bool mainDebug = false;
sequencer seq = sequencer(mainDebug);
//led ledSeq = led(mainDebug);

void setup() {
  if(mainDebug) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }

  //ledSeq.startingAnimation();

  if(mainDebug){
    seq.start();
  }
}

void loop() {

  //ledseq.checkButtons();
  //ledseq.activeMenuBlink();

  if(Serial.available()  > 0) {
    byte byte_read = Serial.read();
    switch(byte_read) {
      case MIDI_START:
          seq.start();
        break;
      case MIDI_STOP:
        seq.stop();
        //ledseq.blinkPin(0, seq.getActiveStep());
        break;
      case MIDI_CONT:
        seq.cont();
        break;
      case MIDI_CLOCK:
          //ledseq.showSequence();
          seq.clock();
        break;
      default:
        break;
    }
  }
}
