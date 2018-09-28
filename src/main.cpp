//#include "..\lib\seq.hpp"
//#include "..\lib\midi.hpp"
//#include "..\lib\led.hpp"
#include "..\lib\display.hpp"

bool mainDebug = false;
//sequencer seq = sequencer(mainDebug);
//led ledSeq = led(mainDebug);
displaySequencer mainSeq = displaySequencer(mainDebug);

void setup() {
  if(mainDebug) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }

  mainSeq.startingAnimation();


}

void loop() {
  mainSeq.run();

}
