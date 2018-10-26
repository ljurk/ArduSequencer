/*#include <Arduino.h>
//#include <Wire.h>
#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"

class led {
private:
  //true disables midi, and writes debug messages on 9600 baud
  bool debugLed = false;

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
  sequencer seq;
public:
  led(bool debug);
  void startingAnimation();
  void pressNext();
  void pressPrev();
  void showSequence();
  void blinkPin(byte blink, byte unblink);
  void checkButtons();
  void activeMenuBlink();
  void run();
};
*/
