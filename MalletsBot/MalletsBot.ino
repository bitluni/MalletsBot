//mallets bot v1.0 by bitluni
//code is public domain, but I don't mind a shout out ;-)

#include "MIDIUSB.h"
#include <Adafruit_PWMServoDriver.h>

/****** configuration ******/
const int startingNote = 66;  //F4# in my case
const bool wrapAround = true; //wrap around octaves. this will play notes that are too high or too low in your actave
const int malletCount = 12;   //total count of connected mallets
const int noteOffDelay = 100; //ms. make this high enough that the instrument works but low enough to be able to play fast notes
/*******************/

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40 /*default*/);

//this arry hold time outs for each note... once time() passes this time out the note is turned off again
long mallets[malletCount] = {0};

void setup() {
  Serial.begin(115200);
  
  //initialize PWM board
  pwm.begin();  
  pwm.setPWMFreq(1000);
  //Wire.setClock(400000);
}

void loop() 
{
  long t = millis();
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) 
    {
      /*Serial.print("Received: ");
      Serial.print(rx.header, HEX); Serial.print(" "); 
      Serial.print(rx.byte1, HEX); Serial.print(" ");
      Serial.print(rx.byte2, HEX); Serial.print(" ");
      Serial.println(rx.byte3, HEX);*/
	  
	  //check if note on message has been received. Note on with velocity 0 is also a note off 
      if(rx.header == 9 && rx.byte3 > 0)
      {
        //note on
        int mallet = (rx.byte2 - startingNote + malletCount * 126) % (wrapAround ? malletCount : 127);
        if(mallet >= 0 && mallet < malletCount)
        {
			//sets PWM according to velocity value
			pwm.setPWM(mallet, 4095 - rx.byte3 * 32, 4095);
			//sets the time out of this note
			mallets[mallet] = t + noteOffDelay;
        }
      }
	  //check for a note off message or note on with velocity 0
      /*else if((rx.header == 9 && rx.byte3 == 0) || rx.header == 8)
      {
        //note off
        int mallet = (rx.byte2 - startingNote + malletCount * 126) % (wrapAround ? malletCount : 127);
        if(mallet >= 0 && mallet < malletCount)
          pwm.setPWM(rx.byte2 % malletCount, 0, 4096);
      }*/
    }
  } while (rx.header != 0);
  
  //this loop checks if time out on a note has been reached and releases it 
  for(int i = 0; i < malletCount; i++)
    if(mallets[i] != 0 && mallets[i] <= t)
	{
      pwm.setPWM(i, 0, 4096);
	  mallets[i] = 0;
	}
}
