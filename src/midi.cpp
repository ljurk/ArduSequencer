#include "..\lib\midi.hpp"

void sendMidi(byte channel, byte command, byte arg1, byte arg2) {
  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }
  // send MIDI data

    Serial.println("MIDI");
    Serial.println(command);
    Serial.println(arg1);
    Serial.println(arg2);
}
