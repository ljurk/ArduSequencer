#include <Arduino.h>
#include "..\lib\midi.hpp"

#ifdef STEP_LENGTH
#else
  #define STEP_LENGTH 16
#endif

#ifdef NUMBER_OF_CHANNELS
#else
  #define NUMBER_OF_CHANNELS 4
#endif

#define DEFAULT_VELOCITY 64

class sequencer {
private:
  int count = 0; // counter for midi ticks, 24 ticks are one quarter note
  byte speedDivider = 1; //1=24ticks,2=12ticks,4=6ticks
  byte defaultNote =  0;
  bool stopped = true;
  struct seqChannel {
  	bool gate[STEP_LENGTH];
  	byte notes[STEP_LENGTH];
  	bool slide[STEP_LENGTH];
  	byte velocity[STEP_LENGTH];
  };
  seqChannel chan[NUMBER_OF_CHANNELS];
  byte activeStep = 0;
  byte oldStep = 0;
  byte oldMenuStep = 0;
  byte activeMenuStep = 0;
  byte activeChannel = 0;
  bool slideActive = false;
  bool seqDebug = false;
public:
  sequencer(bool dbg = false);
  int getDefaultNote();
  int getActiveStep();
  bool getStopped();
  bool getGate(byte channel,int pos);
  byte getNote(byte channel,int pos);
  bool getSlide(byte channel,int pos);
  byte getVelocity(byte channel,int pos);
  byte getOldStep();
  byte getOldMenuStep();
  bool getSlideActive();
  byte getActiveMenuStep();

  void defaultNoteUp();
  void defaultNoteDown();
  void resetSequence(byte channel);
  void nextStep();
  void prevStep();
  void step(byte channel);

  void noteDown(byte channel);
  void noteUp(byte channel);
  void setVelocityUp(byte channel);
  void setVelocityDown(byte channel);
  void setSlide(byte channel);
  void setGate(byte channel);
  void setNote(byte channel);

  void start();
  void stop();
  void cont();
  void clock();
};
