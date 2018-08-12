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

// What do we send over I2C?
#define BYTEROBOTOPERATINGMODE 0
#define BYTEROBOTSTATUS 1
#define BYTETARGETANGLE 2

// Robot Operating Modes
#define ROBOTDISABLED B00000001
#define ROBOTAUTO     B00000010
#define ROBOTTELEOP   B00000100
#define ROBOTTEST     B00001000

// Robot Status
#define ROBOTBATTERYGOOD  B00000001
#define ROBOTBATTERYWARN  B00000010
#define ROBOTBATTERYCRIT  B00000100
#define ROBOTBATTERYSTOP  B00001000
#define ROBOTHASTARGET    B00010000

// communication stuff
#define COMMUNICATIONBUFFERLENGTH 10
