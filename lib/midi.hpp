#include <Arduino.h>
//for MIDI IN
#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
//for MIDI OUT
#define MIDI_CHANNEL 10 //0-15 represents channels 1-16

//buttons
#define SET_SLIDE_PIN  11
#define FUNC_PIN 12
#define  NOTE_UP_DOWN_PIN 13
#define NEXT_PREV_PIN 10
#define BLINK_TIME 150


void sendMidi(byte channel, byte command, byte arg1, byte arg2);
