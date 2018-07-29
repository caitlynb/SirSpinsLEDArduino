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

// DEFINES
#define LEDDataPin 32
#define LEDClockPin 35

#define numstrips 4
#define striplen 26 

#define clkwise true
#define cntrclkwise false

// Rear = Rio
#define FrontTop 0
#define FrontBottom 3
#define RearTop 1
#define RearBottom 2

const uint8_t numleds = numstrips * striplen;
CRGB leds[numleds];

uint8_t stripidx[numstrips];
bool stripdir[numstrips] = {clkwise, cntrclkwise, clkwise, clkwise};

unsigned long time;
unsigned long strobetime;
bool warningblink = false;

CRGB warningcol = {200,60,0};

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
    warningblink = !warningblink;
    if(warningblink){
      setLED(FrontTop, 1, warningcol);
      setLED(FrontTop, striplen-2, warningcol);
      setLED(RearTop, 1, warningcol);
      setLED(RearTop, striplen-2, warningcol);
      setLED(FrontTop, 3, warningcol);
      setLED(FrontTop, striplen-4, warningcol);
      setLED(RearTop, 3, warningcol);
      setLED(RearTop, striplen-4, warningcol);
      setLED(FrontTop, 5, warningcol);
      setLED(FrontTop, striplen-6, warningcol);
      setLED(RearTop, 5, warningcol);
      setLED(RearTop, striplen-6, warningcol);
    } else {
      setLED(FrontTop, 1, CRGB::Black);
      setLED(FrontTop, striplen-2, CRGB::Black);
      setLED(RearTop, 1, CRGB::Black);
      setLED(RearTop, striplen-2, CRGB::Black);
      setLED(FrontTop, 3, CRGB::Black);
      setLED(FrontTop, striplen-4, CRGB::Black);
      setLED(RearTop, 3, CRGB::Black);
      setLED(RearTop, striplen-4, CRGB::Black);
      setLED(FrontTop, 5, CRGB::Black);
      setLED(FrontTop, striplen-6, CRGB::Black);
      setLED(RearTop, 5, CRGB::Black);
      setLED(RearTop, striplen-6, CRGB::Black);
    }
  }
  FastLED.setBrightness(50);
  FastLED.show();
}


