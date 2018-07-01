/*short datapin  = 2;   // SER Pin (14)
short clockpin = 3;   // SRCLK Pin 11
short latchpin = 4;   // RCLK Pin 12
                     // über Pin 9 (Qh') werden Register
                     // aneinandergehängt
                     // Pin 10 Resetpin

byte Steps [9] = {
  0b00000000,
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b00000001,
}; // Array für die Pinauswahl Shift-Register

short s3 = 8; // Multiplexer Pins
short s2 = 7;
short s1 = 6;
short s0 = 5;

//für midi sync
byte midi_start = 0xfa;//250
byte midi_stop = 0xfc;//252
byte midi_clock = 0xf8;//248
byte midi_continue = 0xfb;//251
int play_flag = 0;
byte data;
short count = 0;

short tempPin = 4;

byte Test = 0b11111111;

byte clr = 0b00000000;

short a = 1; //CaseSwitch variable für Ping-Pong Modus

short syncPin = 9;
short midi_sync = 0;// midi sync variable für an / aus

short fwd_pin = 3;
short dir = 0;// Variable für Richtung
              // 0 -> vorwärts
              // 1 -> Ping Pong
              // 2 -> rückwärts

short lengthPin = 5;
short clkPin = 10;

int i = 1;

void setup() {
 // Shift Register Pins
 pinMode (datapin  ,OUTPUT);
 pinMode (clockpin ,OUTPUT);
 pinMode (latchpin ,OUTPUT);

 // Multiplexer Pins
 pinMode (s3, OUTPUT);
 pinMode (s2, OUTPUT);
 pinMode (s1, OUTPUT);
 pinMode (s0, OUTPUT);

 i = 1;
 count = 0;

 // Lauflicht initialisieren
digitalWrite(latchpin, LOW);
shiftOut(datapin, latchpin, LSBFIRST, clr);
shiftOut(datapin, latchpin, LSBFIRST, clr);
digitalWrite(latchpin, HIGH);

//pinMode (midi_in, INPUT);
Serial.begin(31250);

}//setup

void loop() {
  // put your main code here, to run repeatedly:

midi_sync = digitalRead(syncPin);

dir = dir_check();

if (midi_sync == HIGH) {

  int count_max = divide_temp();

  if (Serial.available() > 0) {
    data = Serial.read();



    if (data == midi_start) {
      play_flag = 1;
    }//Startsignal

    if (data == midi_stop) {
      play_flag = 0;
      i = 1;
      count = 0;
    }// stopsignal

    if (data == midi_continue) {
      play_flag = 1;
    }//Continuesignal

    if (data == midi_clock && play_flag == 1) {
      if (count == divide_temp()){

        digitalWrite (clkPin, HIGH);

        switch (dir) {

        case 0:
         seq_forward();
         //delay(Tempo);
         break;

        case 1:
         seq_pingpong();
         //delay(Tempo);
         break;

        case 2:
         seq_backward();
         //delay(Tempo);
         break;


        }

          count = 0;//count zurücksetzen



      }//schritt vorwärts entsprechend der Teilung


      count++;
      digitalWrite (clkPin, LOW);

      if (count > divide_temp()){
        count = 0;
      }

    }// midi tick
  }
}//Midi Sync

if (midi_sync == LOW) {

 int  Tempo = analogRead(tempPin);

  switch (dir) {

    case 0:
     digitalWrite (clkPin, LOW);
     seq_forward();
     delay(Tempo);
     digitalWrite(clkPin, HIGH);
     //delay(Tempo/2);
     break;

    case 1:
     digitalWrite(clkPin, LOW);
     seq_pingpong();
     delay(Tempo);
     digitalWrite(clkPin, HIGH);
     //delay(Tempo/2);
     break;

    case 2:
     digitalWrite(clkPin, LOW);
     seq_backward(),
     delay(Tempo);
     digitalWrite(clkPin, HIGH);
     //delay(Tempo/2);
     break;
  }
}//interne Clock

}//loop


void seq_forward() {
  short lngth = seq_length();

  if (i > lngth) {
    i = 1;
    //a = 1;
  }

  if(i<=8)
  {
    digitalWrite(latchpin,LOW);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[0]);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[i]);
    digitalWrite(latchpin, HIGH);
    multi_chan(i);
  }//Steps 1-8

  if(i>8 and i<=lngth)
  {
    digitalWrite(latchpin,LOW);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[i-8]);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[0]);
    digitalWrite(latchpin, HIGH);
    multi_chan(i);
  }//Steps 9-16

  i++;
}// vorwärts


void seq_backward() {

  short lngth = seq_length();

  if (i<=0) {
    i=seq_length();
    //a = 0;
  }

  if(i<=8)
  {
    digitalWrite(latchpin,LOW);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[0]);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[i]);
    digitalWrite(latchpin, HIGH);
    multi_chan(i);
  }//Steps 1-8

    if(i>8 and i<=lngth)
  {
    digitalWrite(latchpin,LOW);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[i-8]);
    shiftOut(datapin, clockpin, LSBFIRST, Steps[0]);
    digitalWrite(latchpin, HIGH);
    multi_chan(i);
  }//Steps 9-16

  i--;
}//rückwärts

void seq_pingpong() {

  switch(a) {

    case 0:
     seq_forward();
     if (i>seq_length() - 1){
      a=1;
     }
     break;

    case 1:
     seq_backward();
     if (i<2) {
      a=0;
     }
     break;
  }
}//ping Pong

int divide_temp() {
  int val = analogRead(tempPin);

  if (val >= 0 && val <=127) {
    return 3;
  } // 32tel

  if (val >= 128 && val <= 255) {
    return 6;
  }// 16tel

  if (val >= 256 && val <= 383) {
    return 8;
  }//Triole

  if (val >= 384 && val <= 511) {
    return 12;
  }//8tel

  if (val >= 512 && val <= 639) {
    return 24;
  }//4tel

  if (val >= 640 && val <= 767) {
    return 48;
  }//2tel

  if (val >= 768 && val <= 895) {
    return 32;
  }// triole zwischen ganzen Noten

  if (val >= 896 && val <= 1023) {
    return 96;
  }//ganze note
}// clock divide funktion für MIDI sync

short dir_check() {
  int val = analogRead(fwd_pin);

  if (val >= 0 && val <= 341) {
    return 0;
  }

  if (val >= 342 && val <= 682) {
    return 1;
  }

  if (val >= 343 && val <= 1023) {
    return 2;
  }
}//check für vorwärts / rückwärts / ping pong

int seq_length() {

 int adc_val = analogRead(lengthPin);

 //delayMicroseconds(10);

 if (adc_val>=0 and adc_val <=63) {

  return 1;

  }

 if (adc_val>=64 and adc_val <=128) {

  return 2;

  }

 if (adc_val>=129 and adc_val <=192) {

  return 3;

  }

   if (adc_val>=193 and adc_val <=256) {

  return 4;

  }

   if (adc_val>=257 and adc_val <=320) {

  return 5;

  }

   if (adc_val>=321 and adc_val <=384) {

  return 6;

  }

   if (adc_val>=385 and adc_val <=448) {

  return 7;

  }

   if (adc_val>=449 and adc_val <=512) {

  return 8;

  }

   if (adc_val>=513 and adc_val <=576) {

  return 9;

  }

   if (adc_val>=577 and adc_val <=640) {

  return 10;

  }

   if (adc_val>=641 and adc_val <=704) {

  return 11;

  }

   if (adc_val>=705 and adc_val <=768) {

  return 12;

  }

   if (adc_val>=769 and adc_val <=832) {

  return 13;

  }

   if (adc_val>=833 and adc_val <=896) {

  return 14;

  }

   if (adc_val>=897 and adc_val <=960) {

  return 15;

  }

   if (adc_val>=961 and adc_val <=1023) {

  return 16;

  }
}// Sequenzlänge


void multi_chan (short Kanal) {

/*
InputsChannel ON Multiplexer

step S3  S2  S1  S0
 1   0   0   0   0    Y0 to Z
 2   0   0   0   1    Y1 to Z
 3   0   0   1   0    Y2 to Z
 4   0   0   1   1    Y3 to Z
 5   0   1   0   0    Y4 to Z
 6   0   1   0   1    Y5 to Z
 7   0   1   1   0    Y6 to Z
 8   0   1   1   1    Y7 to Z
 9   1   0   0   0    Y8 to Z
10   1   0   0   1    Y9 to Z
11   1   0   1   0    Y10 to Z
12   1   0   1   1    Y11 to Z
13   1   1   0   0    Y12 to Z
14   1   1   0   1    Y13 to Z
15   1   1   1   0    Y14 to Z
16   1   1   1   1    Y15 to Z
*

switch(Kanal) {

  case 1:
    digitalWrite (s3, LOW);
    digitalWrite (s2, LOW);
    digitalWrite (s1, LOW);
    digitalWrite (s0, LOW);
    break;

  case 2:

    digitalWrite (s3, LOW);
    digitalWrite (s2, LOW);
    digitalWrite (s1, LOW);
    digitalWrite (s0, HIGH);
    break;

  case 3:

    digitalWrite (s3, LOW);
    digitalWrite (s2, LOW);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, LOW);
    break;

  case 4:

    digitalWrite (s3, LOW);
    digitalWrite (s2, LOW);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, HIGH);
    break;

  case 5:

    digitalWrite (s3, LOW);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, LOW);
    digitalWrite (s0, LOW);
    break;

  case 6:

    digitalWrite (s3, LOW);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, LOW);
    digitalWrite (s0, HIGH);
    break;

  case 7:

    digitalWrite (s3, LOW);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, LOW);
    break;

  case 8:

    digitalWrite (s3, LOW);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, HIGH);
    break;

  case 9:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, LOW);
    digitalWrite (s1, LOW);
    digitalWrite (s0, LOW);
    break;

  case 10:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, LOW);
    digitalWrite (s1, LOW);
    digitalWrite (s0, HIGH);
    break;

  case 11:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, LOW);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, LOW);
    break;

  case 12:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, LOW);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, HIGH);
    break;

  case 13:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, LOW);
    digitalWrite (s0, LOW);
    break;

  case 14:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, LOW);
    digitalWrite (s0, HIGH);
    break;

  case 15:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, LOW);
    break;

  case 16:

    digitalWrite (s3, HIGH);
    digitalWrite (s2, HIGH);
    digitalWrite (s1, HIGH);
    digitalWrite (s0, HIGH);
    break;
}





} //Multiplexer ende
*/
