/*#include <Arduino.h>
//////////////////////////////////////////////
//
//  simplenZAR v 0.8.16
//
/////////////////////////////////////////////
//
//  Small manual:
//
//  pushbutton 1 is select, pushbutton 2 enter0
//
//  scroll trough the steps with select
//
//  pressing enter will take you to change mode selection
//  select note on/off, pitch, velocity by pressing select and enter afterwards
//  the mode LEDs will show you the mode
//  LED1 = note on/off
//  LED2 = pitch
//  LED3 = velocity
//  No LED = leave change mode, back to step scolling
//  Blinking mode LED shows that you can edit with Pot
//  If you are in 16-Steps, mode LED will be inverted
//
//  Pressing moth buttons will take you to the Menu
//  Two menu options, indicated by the first two LEDs
//  1st = change BPM
//  2nd = change play mode
//  choose with enter
//
//  Change BPM:
//  You can change the BPM with the pot
//
//  Change Play Mode:
//  3 Playmodes available
//  1st - 8-Steps (indicated by 6th LED)
//  2nd - 16-Steps (indicated by 7th LED)
//  3rd - 8-Steps with 8 Shiftings (indicated by 8th LED)
//
//  If you want to edit the 2nd 8 steps or the 8 shiftings,
//  just scroll to the next 8 Steps



const int stepLed1 = 6;
const int stepLed2 = 7;
const int stepLed3 = 8;
const int stepLed4 = 9;
const int stepLed5 = 10;
const int stepLed6 = 11;
const int stepLed7 = 12;
const int stepLed8 = 13;

const int modeLed1 = 3;
const int modeLed2 = 4;
const int modeLed3 = 5;


  int pos1 = 1;
  int pos2 = 1;
  int pos3 = 1;

  int stepPosi = 1;
  int workPosi = 1;
  int stepLength = 8;

  int beat = 90; // Time to wait for each step

  int note[17];  //0 off / 1 on
  int pitch[17]; //starting at C3 -> Hex 30
  int velo[17];  // 0 - 127

  int oldnote[17];  //To stop old note

  int shift[9];  // amount of shift from the basenote

  int mode = 4; //mode 1 = noteon/off
                //mode 2 = Pitch
                //mode 3 = velo
                //mode 4 = back
  int button1 = 0; //Analogpin 0
  bool button1Pressed = false; //Shows if Button 1 was pressed
  bool button1State = false;  //Was there a statechange ?
  int button2 = 1; //Analogpin 1
  bool button2Pressed = false; //Shows if Button 2 was pressed
  bool button2State = false;  //Was there a statechange ?
  int pot1 = 2; // Analog A2
  int potposition;
  int programpos = 1;
  int buttonmode = 0;
  bool edit = false;  // LED will blink in Edit mode

  int menupos = 0;     //menu  0=normal play&edit
                       //1=change tempo
                       //2=change steps (1-16-shift)

  //int stepdirection =1;  //1 = forward, 2 reverse, 3 bounce MAYBE IN THE FUTURE

  int shiftpos = 1;            //position in incremented after each step
  bool menuactive = false;  // If Menu Active, read Poti

  bool menuon = false; //  Menumodus

  bool shifting = false; //shift modus on/off

  int basenote;

  int shift816 = 1;            //indilator for 8/16 steps or Shifting, default 8

void setup() {
  // set the digital pin for the 8 step leds
  pinMode(stepLed1, OUTPUT);
  pinMode(stepLed2, OUTPUT);
  pinMode(stepLed3, OUTPUT);
  pinMode(stepLed4, OUTPUT);
  pinMode(stepLed5, OUTPUT);
  pinMode(stepLed6, OUTPUT);
  pinMode(stepLed7, OUTPUT);
  pinMode(stepLed8, OUTPUT);

  pinMode(modeLed1, OUTPUT);
  pinMode(modeLed2, OUTPUT);
  pinMode(modeLed3, OUTPUT);

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(pot1, INPUT);



  // initialisation of step parameter
  for (int a = 1; a <= 16; a++)
  {
    note[a]=0;
    pitch[a]=0;
    velo[a]=60;
  }

   for (int a = 1; a <= 8; a++)
    shift[a]=0;


  // 1st Note as index
  note[1]=1;
  pitch[1]=0;
  velo[1]=55;

 Serial.begin(31250); //Baudrate for Midi
 //Serial.begin(9600); //Baudrate for Serial communication = Debugging


}

void ledSet(int steppos, int workpos, int mode, int meunpos, int shiftpos, boolean shifting, int shift816)
{
  int workpos2, steppos2;
  int invert = 0;                           // if workpos >8 invertiert modes
   // All LEDs OFF
  for(int x = 3; x < 14; x++)               //digital outputs 6 - 14 for step leds, 3-5 mode leds
    digitalWrite(x, LOW);

  if (menupos > 0)                          //Menumodus is turned on
  {                                          //Menuindication
     digitalWrite(2+mode, HIGH);            //Sets Mode runlight
     digitalWrite(5+menupos, HIGH);         //Sets Menuposition

     switch (shift816)
     {
       case 1:
       digitalWrite(11, HIGH);              //6th LED for 8 Steps
       break;

       case 2:
       digitalWrite(12, HIGH);              //7th LED for 16 setps
       break;

       case 3:
       digitalWrite(13, HIGH);              //8th LED for Shifting
       break;

     }
  }
  else //Normal work -> no Menu aktiv
  {

    //set Step LED
    if (((steppos > 8) && (workpos > 8))||((steppos < 9)&&(workpos < 9))) // if Step and work are in the same 8 steps
    {
      if(steppos > 8)
        steppos = steppos-8;

      digitalWrite(5+steppos, HIGH);
    }

    // Shift LED on
    if (workpos > 8 && shifting == true)
    {
       digitalWrite(5+shiftpos, HIGH);
    }

    // workpos setzen
    if(workpos > 8)
    {
        workpos = workpos-8;
        invert = 1;
    }

    digitalWrite(5+workpos, HIGH);






    // mode setzen
    if(programpos==3||programpos==4||programpos==5)
    {
       if(edit == true)
       {
          if (((mode == 1) && (invert ==0)) || (( mode == 2) && (invert == 1)) || (( mode == 3) && (invert == 1)))            // note on off
            digitalWrite(5, HIGH);   // pin 5
          if (((mode == 2) && (invert ==0)) || (( mode == 1) && (invert == 1)) || (( mode == 3) && (invert == 1)))            // pitch
            digitalWrite(4, HIGH);   // pin 4
          if (((mode == 3) && (invert ==0)) || (( mode == 1) && (invert == 1)) || (( mode == 2) && (invert == 1)))            // velocity
            digitalWrite(3, HIGH);   // pin 3
          edit = false;
       }
       else
         edit = true;
    }
    else
    {
      if (((mode == 1) && (invert ==0)) || (( mode == 2) && (invert == 1)) || (( mode == 3) && (invert == 1)))            // note on off
         digitalWrite(5, HIGH);   // pin 5
      if (((mode == 2) && (invert ==0)) || (( mode == 1) && (invert == 1)) || (( mode == 3) && (invert == 1)))            // pitch
        digitalWrite(4, HIGH);   // pin 4
      if (((mode == 3) && (invert ==0)) || (( mode == 1) && (invert == 1)) || (( mode == 2) && (invert == 1)))            // velocity
        digitalWrite(3, HIGH);   // pin 3
      if ((mode==4)&&(invert ==1)) //if no mode choosen and workpos >8, invert
      {
        digitalWrite(5, HIGH);   // pin 5
        digitalWrite(4, HIGH);   // pin 4
        digitalWrite(3, HIGH);   // pin 3
      }
    }
  }
}




void noteOn(byte cmd, byte pitch, byte velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void checkbuttons() {
  //Button 1
//  Serial.println("--");
  if(analogRead(button1)&&button1Pressed==false)
  {
    //)Serial.println("SELECT");
    button1Pressed = true;
    button1State = true;
  }
  else if (button1Pressed == true&&analogRead(button1)==false)
    button1Pressed = false;
  //Button2
  if(analogRead(button2)&&button2Pressed==false)
  {
        //Serial.println("Enter");
    button2Pressed = true;
    button2State = true;

  }
  else if (button2Pressed == true&&analogRead(button2)==false)
    button2Pressed = false;

  // Read Pot

}

void loop() {


//---------------------------------------------
//--- First thing to do, play the next note ---
//---------------------------------------------

  noteOn(0x80, oldnote[stepPosi], 0);        //Turn note off
                                              //Next step calculation
  stepPosi++;
  if (stepPosi > stepLength)
  {
    stepPosi = 1;
    shiftpos++;                               //after 8 Steps shiftposition +1
    if (shiftpos > 8)                         // There are 8 Schiftpositions
      shiftpos=1;
  }

                                              //Check if shift is on
  if(shifting)
  {
     basenote = 0x30+shift[shiftpos];         //Basenote incremented by Shift value
  }
  else
    basenote = 0x30;                          //Basenote stays at hex30


                                              // Now we Play the note of the step

  if (note[stepPosi])                         // If Note if on, we play the note
  {

    noteOn(0x90, basenote+pitch[stepPosi], velo[stepPosi]);
    oldnote[stepPosi]=basenote+pitch[stepPosi];  // Save the note, to turn it off for the next step
  }

    //tone(2,basenote*pitch[stepPosi]);      //Wenn Piezo angeschlossen ist

//------------------------------------------------



//------------------------------------------------
//--- Check the buttons and process them ---------
//------------------------------------------------

                                              //Checking Buttons
  checkbuttons();

                                              //If both buttons are pressed, toggle menu
  if (button1Pressed==true && button1State==true && button2Pressed==true && button2State==true)
  {
    if (menuon==false)                        // toggel menu
    {
      menuon=true;
      menupos = 1;                            //Menuposition to 1
    }
    else                                      // Was already on so will we off now
    {
      menuon=false;
      menupos = 0;                            // Menupos to 0
      mode=4;                                 // Mode to 4 (no LED)
      programpos=1;                           //Programmpos to 1
      menuactive=false;                       //There is no Menu
    }
   button1State=false;                        //both Buttons are processed
   button2State=false;
  }


//----------------------------------------------Button 1 - <SELECT>
  if (button1Pressed==true && button1State==true)
  {

    if (menuon == true && menuactive == false)  //If in menumode and no menu is active
    {
       menupos++;                             //<SELECT> will scoll trough the menus
       if (menupos > 2)                       //Just 2 Menupos (Speed and 8/16/Shift)
         menupos = 1;

       programpos=0;
    }


                                              // 1st programm level - select moves the cursor
    if (programpos==1)
    {
      workPosi++;                             //If Shifing is off and workposi > steplengt back to 1
      if (shifting==false && workPosi > stepLength)
        workPosi = 1;                         //If Shifting is on, there are 8 steps, if workposi > 8, Workposi is for Shifting
      if (shifting == true && workPosi >16)
        workPosi = 1;
    }

                                               // 2nd program level - select changes the mode +1
    if (programpos==2)
    {
      mode++;
      if (mode>4)
        mode=1;
    }

                                                 // 3rd,4th,5th Program level - select moves back to 2 level
    if (programpos==3||programpos==4||programpos==5)
    {
      programpos=2;
    }

                                                  //Buttonstate to false, button is processed
    button1State=false;
  }


//-------------------------------------------------- Button2 - <ENTER>
  if (button2Pressed==true && button2State==true)
  {

     if (menuon == true)                          //If in menu level
    {
                                                  // Menu is turned active
       if (menuactive==false)                     // menuparameter is changed by Pot value
       {                                          // value is changed according menupos
          menuactive=true;
          //noteOn(0x90, 0x30+60, 127);
       }
       else                                      // Menu is turned inactive
       {
          menuactive=false;
          //noteOn(0x90, 0x30+70, 127);
       }

       button2State=false;
    }
                                                   // 1st programm level - ENTER changes to Program level 2
    if (programpos==1 && button2State==true)
    {
       programpos=2;
       mode=1;
       button2State=false;
    }

                                                    // 2nd program level - Enter changes to program level 3,4,5 aus (Note on/off, Pitch, Velocity)
    if (programpos==2&&button2State==true)
    {
      if (mode == 1)
        programpos = 3;                             //Note on/off
      if (mode == 2)
        programpos = 4;                             // Pitch
      if (mode == 3)
        programpos = 5;                             // Velovity
      if (mode == 4)                                // there is no Mode 4, is used to go back
      {
        programpos = 1;                             //back
        mode=4;                                     //mode set back to 1
      }
      button2State=false;                           //Button is processed
    }

                                                    // 3rd,4th,5th Program level - ENTER will go back to level 2
    if ((programpos==3&&button2State==true)||(programpos==4&&button2State==true)||(programpos==5&&button2State==true))
    {
      programpos=2;
      button2State=false;                          //Button is processed
    }

  }

//---------------------------------------------------


//---------------------------------------------------
//--- Potiabfragen ----------------------------------
//---------------------------------------------------
  if(programpos==3 && menupos==0 && workPosi <= stepLength)    //Note on/off mode
    note[workPosi]=analogRead(pot1)/512;

  if(programpos==3 && menupos==0 && workPosi > stepLength)    //ATTENTION this is the change for basenote shifting
    shift[workPosi-8]=analogRead(pot1)/28;

  if(programpos==4 && menupos==0)                             //Pitch mode
    pitch[workPosi]=analogRead(pot1)/28;

  if(programpos==5 && menupos==0)                            //Velocity mode
    velo[workPosi]=analogRead(pot1)/8;

  if(menupos==1 && menuactive == true)                       //If menuactive and menupunkt 1 choosen
    beat =60+ 30*(analogRead(pot1)/256);                     //change BPM

  if(menupos==2 && menuactive == true)                      //If menuactive and menupunkt 2 choosen
  {
    if(analogRead(pot1) < 341)                                // Change play mode to 8-Steps
    {
      stepLength = 8;
      shifting = false;
      shift816=1;
    }

    if(analogRead(pot1) > 341 && analogRead(pot1) < 682)      // Change play mode to 16-Steps
    {
      stepLength = 16;
      shifting = false;
      shift816=2;
    }

    if(analogRead(pot1) > 682)                                // Change play mode to Shifting
    {
      shifting=true;
      stepLength = 8;
      shift816=3;      //Steplenght set to 8 . There are 8 steps plus 8 shifts
      //noteOn(0x90, 0x30+60, 127);
    }
  }

//-----------------------------------------------------






//----------------------------------------------------
//--- Final --- setting LEDs and wait ---------
//----------------------------------------------------

if(menupos >0)                                                //running light active if menu active
{
     mode++;
     if (mode>3)
        mode=1;
}

  ledSet(stepPosi, workPosi, mode, menupos, shiftpos, shifting, shift816);

  delay(beat);

}
*/
