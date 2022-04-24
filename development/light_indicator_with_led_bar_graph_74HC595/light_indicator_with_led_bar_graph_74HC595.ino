/*
 *  Light Indicator (LEDs Bar Graph & 74HC595 8-bit SR).
 *
 *  Copyright (C) 2010 Efstathios Chatzikyriakidis (stathis.chatzikyriakidis@gmail.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include notes' frequencies.
#include "pitches.h"

const int piezoPin = 9;  // the pin number of the piezo speaker.
const int sensorPin = 0; // the pin number of the input sensor.

// the two bounds of the input light sensor (analog input).
const int sensorMin = 0;
const int sensorMax = 1023;

// number of leds to control.
const int NUM_LEDS = 8;

// threshold value for the music to play.
const int ledsThreshold = NUM_LEDS-1;

// default value for leds bar graph.
int ledsRange = -1;

// used for single LED manipulation.
int ledState = LOW;

// set 74HC595 8-bit shift register control pin numbers.
const int data = 7;
const int clock = 3; // PWM pin.
const int latch = 4;

// these are used in the bitwise math that we use to change individual LEDs.

// each specific bit set to 1.
const int bits[] = { B00000001, B00000010,
                     B00000100, B00001000,
                     B00010000, B00100000,
                     B01000000, B10000000 };

// each specific bit set to 0.
const int masks[] = { B11111110, B11111101,
                      B11111011, B11110111,
                      B11101111, B11011111,
                      B10111111, B01111111 };

// notes of the melody.
const int notesMelody[] = {
  NOTE_G4, NOTE_C4, NOTE_G3, NOTE_G3, NOTE_C4, NOTE_G3, NOTE_C4
};

// calculate the number of the notes in the melody in the array.
const int NUM_NOTES = (int) (sizeof (notesMelody) / sizeof (const int));

// note durations: 4 = quarter note, 8 = eighth note, etc.
const int noteDurations[] = {
  4, 8, 8, 2, 4, 2, 4
};

// startup point entry (runs once).
void setup () {
  // set sensor as input.
  pinMode(sensorPin, INPUT);

  // set piezo as output.
  pinMode(piezoPin, OUTPUT);

  // set 74HC595 control pins as output.
  pinMode (data, OUTPUT);
  pinMode (clock, OUTPUT);  
  pinMode (latch, OUTPUT);  

  // dark all the leds.
  for (int i = 0; i < NUM_LEDS; i++)
    changeLED (i, LOW);
}

// loop the main sketch.
void loop () {
  // get the value from the input sensor.
  ledsRange = analogRead(sensorPin);

  // map the value for the leds bar graph.
  ledsRange = map(ledsRange, sensorMin, sensorMax, -1, NUM_LEDS-1);

  // light or dark the appropriate leds from the bar graph.
  for (int thisLed = 0; thisLed < NUM_LEDS; thisLed++) {
    if (thisLed <= ledsRange) {
      // light the led.
      changeLED (thisLed, HIGH);
    }
    else {
      // dark the led.
      changeLED (thisLed, LOW);
    }
  }

  // if the bar graph leds is completed play a melody.
  if(ledsRange >= ledsThreshold)
    playMelody ();
}

// play a melody and return immediately.
void playMelody () {
  // iterate over the notes of the melody.
  for (int thisNote = 0; thisNote < NUM_NOTES; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000/4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];

    // play the tone.
    tone(piezoPin, notesMelody[thisNote], noteDuration);

    // to distinguish notes, set a minimum time between them.
    // the note's duration plus 30% seems to work well enough.
    int pauseBetweenNotes = noteDuration * 1.30;

    // delay some time.
    delay(pauseBetweenNotes);
  }
}

// sends the LED states set in ledStates to the 74HC595 sequence.
void updateLEDs (int value) {
  // pulls the chips latch low.
  digitalWrite (latch, LOW);

  // shifts out the 8 bits to the shift register.
  shiftOut (data, clock, MSBFIRST, value);

  // pulls the latch high displaying the data.
  digitalWrite (latch, HIGH);
}

// changes an individual LED.
void changeLED (int led, int state) {
  // clears ledState of the bit we are addressing.
  ledState = ledState & masks[led];

  // if the bit is on we will add it to ledState.
  if (state == HIGH) {
    ledState = ledState | bits[led];
  }

  // send the new LED state to the shift register.
  updateLEDs(ledState);
}
