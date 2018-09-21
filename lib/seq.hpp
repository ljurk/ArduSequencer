#include <Arduino.h>
#include "..\lib\midi.hpp"

#define NOTE_OFF 8
#define NOTE_ON 9
#define DEFAULT_VELOCITY 64
#define STEP_LENGTH 8

class sequencer {
private:

public:
  sequencer();
  int getDefaultNote();
  int getActiveStep();
  bool getStopped();
  bool getGate(int pos);
  byte getNote(int pos);
  bool getSlide(int pos);
  byte getOldStep();
  byte getOldMenuStep();
  bool getSlideActive();
  byte getActiveMenuStep();

  void raiseDefaultNote();
  void lowerDefaultNote();
  void resetSequence();
  void nextStep();
  void prevStep();
  void step();

  void noteDown();
  void noteUp();
  void setSlide();
  void setGate();
  void setNote();

  void start();
  void stop();
  void cont();
  void clock();
};
