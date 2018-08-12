// autoinclude FastLED library
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>
#include <Wire.h>
#include <avr/wdt.h> // watchdogs... silly I2C issues.

// Communications with the Rio
#include "SirTipsDefines.h"

const uint8_t I2CAddress = 8;

const uint8_t numleds = numstrips * striplen;
CRGB leds[numleds];

uint8_t stripidx[numstrips];
bool stripdir[numstrips] = {clkwise, cntrclkwise, clkwise, clkwise};

unsigned long curmillis;
unsigned long strobetime;
unsigned long i2ctime;
unsigned long ledtimer;
bool blinkState = false;

uint8_t curTargetAngle = 0;
bool hasTarget = 0;


CRGB warningcol = {200,60,0};
CRGB battgoodcol = {0,200,0};
CRGB battwarncol = warningcol;
CRGB battcritcol = {200,0,0};

CRGB disabledcol = CRGB::Green;
CRGB autocol = CRGB::Blue;
CRGB teleopcol = CRGB::Red;
CRGB testcol = CRGB::White;

CRGB stalecol = {50,20,20};


uint8_t riocomms[COMMUNICATIONBUFFERLENGTH];
boolean newdata = false;

uint8_t curRobotOperatingMode = ROBOTDISABLED;
uint8_t curRobotStatus = ROBOTBATTERYSTOP;

void setup() {
  wdt_enable(WDTO_250MS);
  // put your setup code here, to run once:
  FastLED.addLeds<APA102, LEDDataPin, LEDClockPin, BGR>(leds, numleds);
  FastLED.show(); // immediately turn off LEDs.
  // setup stripidx for indexing later
  for (int i = 0; i < numstrips; i++){
    stripidx[i] = i * striplen;
  }

  curmillis = millis();
  strobetime = millis();

  for(int i =0; i < COMMUNICATIONBUFFERLENGTH; i++){
    riocomms[i] = 0;
  }
  
  Wire.begin(I2CAddress);
  Wire.onReceive(receiveI2CEvent);

  //setBinaryLed(FrontBottom, CRGB::Blue, 12, B10100001);
  setLED(FrontTop, 8, CRGB::Blue);
  setLED(FrontTop, 17, CRGB::Blue);
  setLED(FrontBottom, 8, CRGB::Blue);
  setLED(FrontBottom, 17, CRGB::Blue);
}

void(* resetFunc) (void) = 0; // declare reset function at address 0, from instructables

void setLED(uint8_t stripid, uint8_t ledid, CRGB color){
  if(stripdir[stripid]){
    // true = clockwise
    leds[stripidx[stripid]+ledid] = color;
  } else {
    // false = counterclockwise
    leds[stripidx[stripid]+map(ledid, 0, striplen, striplen, 0)-1] = color;
  }
}

void receiveI2CEvent(int numbytes){
  int bytenum = 0;
  while(0 < Wire.available()){
    uint8_t in = Wire.read();
    if(bytenum < COMMUNICATIONBUFFERLENGTH){
      riocomms[bytenum] = in;
    }
    bytenum++;
  }
  newdata = true;
}

void setBinaryLed(int stripid, CRGB color, int start, uint8_t data){
  for(int i = 0; i < 8; i++){
    if(data & B00000001){
      // note, currently outputs 'backwards' (LSB), but this is good for my clockwise oriented leds.
      setLED(stripid, start+i, color);
    } else {
      setLED(stripid, start+i, CRGB::Black);
    }
    data = data>>1;
  }
}

void setWarningLed(CRGB color){
  setLED(FrontTop, 1, color);
  setLED(FrontTop, striplen-2, color);
  setLED(RearTop, 1, color);
  setLED(RearTop, striplen-2, color);
  setLED(FrontTop, 3, color);
  setLED(FrontTop, striplen-4, color);
  setLED(RearTop, 3, color);
  setLED(RearTop, striplen-4, color);
  setLED(FrontTop, 5, color);
  setLED(FrontTop, striplen-6, color);
  setLED(RearTop, 5, color);
  setLED(RearTop, striplen-6,color);
}
void setBatteryLed(CRGB color){
  setLED(FrontBottom, 1, color);
  setLED(FrontBottom, striplen-2, color);
  setLED(RearBottom, 1, color);
  setLED(RearBottom, striplen-2, color);
  setLED(FrontBottom, 3, color);
  setLED(FrontBottom, striplen-4, color);
  setLED(RearBottom, 3, color);
  setLED(RearBottom, striplen-4, color);
  setLED(FrontBottom, 5, color);
  setLED(FrontBottom, striplen-6, color);
  setLED(RearBottom, 5, color);
  setLED(RearBottom, striplen-6,color);
}
void setModeLed(CRGB color){
  setLED(FrontTop, 0, color);
  setLED(FrontTop, striplen-1, color);
  setLED(RearTop, 0, color);
  setLED(RearTop, striplen-1, color);
  setLED(FrontTop, 2, color);
  setLED(FrontTop, striplen-3, color);
  setLED(RearTop, 2, color);
  setLED(RearTop, striplen-3, color);
  setLED(FrontTop, 4, color);
  setLED(FrontTop, striplen-5, color);
  setLED(RearTop, 4, color);
  setLED(RearTop, striplen-5,color);

  setLED(FrontBottom, 0, color);
  setLED(FrontBottom, striplen-1, color);
  setLED(RearBottom, 0, color);
  setLED(RearBottom, striplen-1, color);
  setLED(FrontBottom, 2, color);
  setLED(FrontBottom, striplen-3, color);
  setLED(RearBottom, 2, color);
  setLED(RearBottom, striplen-3, color);
  setLED(FrontBottom, 4, color);
  setLED(FrontBottom, striplen-5, color);
  setLED(RearBottom, 4, color);
  setLED(RearBottom, striplen-5,color);
}



void loop() {
  wdt_reset();
  // put your main code here, to run repeatedly:
  /*for(int i = 0; i < numleds; i++){
    leds[i] = CRGB::Green;
    FastLED.show();
    delay(30);
    leds[i] = CRGB::Black;
  }*/
  
  curmillis = millis();

  if(curmillis - strobetime > 500){
      strobetime = curmillis;
      blinkState = !blinkState;
  }

 
  if(newdata){
    setBinaryLed(FrontTop, CRGB::Red, 9, riocomms[0]);
    setBinaryLed(FrontBottom, CRGB::Red, 9, riocomms[1]);
    curRobotOperatingMode = riocomms[0];
    curRobotStatus = riocomms[1];
    newdata = false;
    i2ctime = curmillis;
  } else if(curmillis - i2ctime > 5000){
    // if it has been 5 seconds since we last saw an I2C packet, perform a hard reset of the arduino
    // just in case the I2C bus got noisy and crashed...
    delay(1000); // trigger the watchdog to reboot the MCU
  } else if(curmillis - i2ctime > 2000){
    for(int i = 0; i < 8; i++){
      setLED(FrontTop, 9+i, CRGB::Black);
      setLED(FrontBottom, 9+i, CRGB::Black);
    }
  } else if(curmillis - i2ctime > 500){
    setBinaryLed(FrontTop, stalecol, 9, riocomms[0]);
    setBinaryLed(FrontBottom, stalecol, 9, riocomms[1]);
  }

  // Robot Safety Light equivalent
  if( curRobotOperatingMode != ROBOTDISABLED){
    if(blinkState){
      setWarningLed(warningcol);
    } else {
      setWarningLed(CRGB::Black);
    }
  } else {
    setWarningLed(warningcol);
  } // Warning Strobes

  // Robot Operating Modes
  if(curRobotOperatingMode == ROBOTDISABLED){
    setModeLed(disabledcol);
  } else if( curRobotOperatingMode == ROBOTAUTO){
    setModeLed(autocol);
  } else if( curRobotOperatingMode == ROBOTTELEOP){
    setModeLed(teleopcol);
  } else if( curRobotOperatingMode == ROBOTTEST){
    setModeLed(testcol);
  }

  if(curRobotStatus == ROBOTBATTERYGOOD){
    setBatteryLed(battgoodcol);
  } else if(curRobotStatus == ROBOTBATTERYWARN){
    setBatteryLed(battwarncol);
  } else if(curRobotStatus == ROBOTBATTERYCRIT){
    setBatteryLed(battcritcol);
  } else if(curRobotStatus == ROBOTBATTERYSTOP){
    if(blinkState){
      setBatteryLed(battcritcol);
    } else {
      setBatteryLed(CRGB::Black);
    }
  }
        
  if(curmillis - ledtimer > 50){
    FastLED.setBrightness(50);
    FastLED.show();
    ledtimer = curmillis;
  }
}




