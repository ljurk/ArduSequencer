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
#define NEXT 1
#define PREV 0

struct seqChannel {
  bool gate[STEP_LENGTH];
  //byte notes[STEP_LENGTH];
  byte note;
  String noteText;
  bool slide[STEP_LENGTH];
  byte velocity[STEP_LENGTH];
  byte length;
};

class sequencer {
private:
  int count = 0; // counter for midi ticks, 24 ticks are one quarter note
  byte speedDivider = 1; //1=24ticks,2=12ticks,4=6ticks
  byte defaultNote =  0;
  bool stopped = true;

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

  //get
  int getDefaultNote();
  int getActiveStep();
  bool getStopped();
  bool getGate(byte channel,byte pos);
  byte getNote(byte channel/*,int pos*/);
  String getNoteText(byte channel/*,int pos*/);
  byte getNoteTextLength(byte channel/*,int pos*/);
  bool getSlide(byte channel,byte pos);
  byte getVelocity(byte channel,byte pos);
  byte getLength(byte channel);
  byte getOldStep();
  byte getOldMenuStep();
  bool getSlideActive();
  byte getActiveMenuStep();
  byte getActiveChannel();

  void defaultNoteUp();
  void defaultNoteDown();
  void resetSequence();
  void step(byte channel);

  //set, always depends on activeChannel and activeMenuStep
  void setNoteDown();
  void setNoteUp();
  void setVelocityUp(int steps = 1);
  void setVelocityDown(int steps = 1);
  void setLength(int steps);
  void setSlide();
  void setGate();
  void setNote();
  //navigation
  void setActiveChannel(byte channel);
  void setCursor(bool direction); //true = forwards, false = backwards


  void start();
  void stop();
  void cont();
  void clock();
};
