//#include "pitches.h"  //add note library
#include <Tone32.h>  //add note library

//BUZZER
int PinBuzzer = 17;
int HeureFinish;
//notes in the melody
int melody[]={
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_B4, 0,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D5, NOTE_C5, 0,
  NOTE_G4, NOTE_G4, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_B4, NOTE_A4, 0,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_C5
  };
//note durations. 4=quarter note / 8=eighth note
int noteDurations[]={4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4};

void setup() {
  // put your setup code here, to run once:
  for (int thisNote=0; thisNote <28; thisNote++){

     //to calculate the note duration, take one second. Divided by the note type
    int noteDuration = 1000 / noteDurations [thisNote];
    tone(PinBuzzer, melody[thisNote], noteDuration, 0);
    //tone(PinBuzzer, NOTE_F4, 500, 0);
    noTone(PinBuzzer, 0);

    //to set the speed of the song set a minimum time between notes
    //int pauseBetweenNotes = noteDuration * 1.30;
    int pauseBetweenNotes = int(noteDuration /4);
    delay(pauseBetweenNotes);      
  }
}

void loop() {

}
