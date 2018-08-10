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

// Communications with the Rio
#include "SirTipsDefines.h"

const uint8_t I2CAddress = 8;

const uint8_t numleds = numstrips * striplen;
CRGB leds[numleds];

uint8_t stripidx[numstrips];
bool stripdir[numstrips] = {clkwise, cntrclkwise, clkwise, clkwise};

unsigned long time;
unsigned long strobetime;
bool blinkState = false;

uint8_t curTargetAngle = 0;
bool hasTarget = 0;


CRGB warningcol = {200,60,0};
CRGB battgoodcol = {0,200,0};
CRGB battwarncol = warningcol;
CRGB battcritcol = {200,0,0};

CRGB disabledcol = CRGB::Green;
CRGB autocol = CRGB::Blue;
CRGB teleopcol = CRGB::Yellow;
CRGB testcol = CRGB::White;

uint8_t curRobotOperatingMode = ROBOTDISABLED;
uint8_t curRobotStatus = ROBOTBATTERYSTOP;

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<APA102, LEDDataPin, LEDClockPin, BGR>(leds, numleds);
  FastLED.show(); // immediately turn off LEDs.
  // setup stripidx for indexing later
  for (int i = 0; i < numstrips; i++){
    stripidx[i] = i * striplen;
  }

  time = millis();
  strobetime = millis();

}

void setLED(uint8_t stripid, uint8_t ledid, CRGB color){
  if(stripdir[stripid]){
    // true = clockwise
    leds[stripidx[stripid]+ledid] = color;
  } else {
    // false = counterclockwise
    leds[stripidx[stripid]+map(ledid, 0, striplen, striplen, 0)-1] = color;
  }
  Wire.begin(I2CAddress);
  Wire.onReceive(receiveI2CEvent);
}

void receiveI2CEvent(int numbytes){
  int bytenum = 0;
  while(1 < Wire.available()){
    uint8_t in = Wire.read();
    switch(bytenum){
      case BYTEROBOTOPERATINGMODE:
        curRobotOperatingMode = in;
        break;
      case BYTEROBOTSTATUS:
        curRobotStatus = in;
        break;
      case BYTETARGETANGLE:
        curTargetAngle = in;
        break;
      default:
        break;
    }
    bytenum++;
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
  // put your main code here, to run repeatedly:
  /*for(int i = 0; i < numleds; i++){
    leds[i] = CRGB::Green;
    FastLED.show();
    delay(30);
    leds[i] = CRGB::Black;
  }*/
  
  time = millis();

  if(time - strobetime > 500){
      strobetime = time;
      blinkState = !blinkState;
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
    
  FastLED.setBrightness(50);
  FastLED.show();
}




