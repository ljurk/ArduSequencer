#include <Encoder.h>
//#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


class displaySequencer{
private:
  bool debugDisplay = false;

  const int buttonPin = 10;
  Encoder myEnc(2,3);
  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  int debounceDelay = 300;
  int buttonState = 0;
  bool buttonPressed = false;
  long oldPosition  = -999;
  long newPosition =  -999;
  String seqString ="00000000";
  String cursorString ="00000000";
  byte mode = 0;
  sequencer seq;
public:
  displaySequencer(debug);
  void step(byte pos);
  void startingAnimation();
  void updateSequence();
  void updateValues();
  void updateCursor();
  void updateSequence();
  void run();

}
