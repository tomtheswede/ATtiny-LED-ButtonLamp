/*  
 *   For a Tactile button and an LED driver at the ledPin.
 *   Code by Thomas Friberg (https://github.com/tomtheswede)
 *   Updated 17/07/2016
 */

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) //OR
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) //AND

//Interrupt variables
volatile bool pressed=false;
volatile bool buttonState=false;
volatile unsigned long pressTime=0;
volatile const unsigned int reTriggerDelay=190; //minimum time in millis between button presses

volatile const unsigned int buttonPin=1; //(1 on ATtiny85)

//INPUT CONSTANTS
const unsigned int ledPin=0; // (0 on ATtiny85)
const unsigned int defaultFadeSpeed=5;
const unsigned int longPressLength=600; //Time in milliseconds for a long press
const unsigned int longerPressLength=3000; //Time in milliseconds for a longer press (all off)
const unsigned int PWMTable[101] = {0,1,1,1,1,1,2,2,2,2,3,3,3,4,4,5,5,6,6,7,7,8,9,9,10,11,12,13,14,15,16,17,18,19,20,21,23,24,25,27,28,30,32,33,35,37,39,41,43,45,47,49,51,53,56,58,61,63,66,69,72,74,77,80,84,87,90,93,97,100,104,108,111,115,119,123,127,131,136,140,145,149,154,159,163,168,174,179,184,189,195,200,206,212,218,224,230,236,242,249,255}; //0 to 100 values for brightnes


//GLOBAL VARIABLES
String data = "";
unsigned int fadeSpeed=defaultFadeSpeed;
bool longPressPrimer=false;
bool longerPressPrimer=false;
unsigned int ledPinState = 0; //Default boot state of LEDs and last setPoint of the pin between 0 and 100
unsigned int ledSetPoint = 0;
unsigned int brightness = 100; //last 'on' setpoint for 0-100 scale brightness
unsigned long lastFadeTime = 0;
long calibrationTime=0;
bool calibratePrimer=false;
int timerCount=0;
bool timerPrimer=false;


//-----------------------------------------------------------------------------

void setup() {
  setupLines();
}

//-----------------------------------------------------------------------------

void loop() {

  ReportButtonPress();
  FadeLEDs();
  delay(0);
}

//-----------------------------------------------------------------------------

void setupLines() {
  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin, HIGH); //Turn off LED while connecting
  delay(20);
  digitalWrite(ledPin, LOW); //Turn off LED while connecting
  ledSetPoint=0; // input a setpoint for fading as we enter the loop
  //Serial.begin(9600);
  
  //Set pin to start interrupts
  sbi(GIMSK,PCIE); //Turn on interrupt
  sbi(PCMSK,PCINT1); //set pin affected by interupt
}

//-----------------------------------------------------------------------------

ISR(PCINT0_vect) {
  // This is called when the interrupt occurs, but I don't need to do anything in it
  buttonState=!digitalRead(buttonPin);
  //Serial.println("Test trigger");
  if (buttonState && (millis()-pressTime>reTriggerDelay)) { //if pressed in
    pressed=true;
    pressTime=millis();
    //Serial.println("--Test trigger2");
  }
  else if (!buttonState && (millis()-pressTime>reTriggerDelay)) { //if released
    pressTime=millis();
    //Serial.println("--Test trigger4");
  }
}

//-----------------------------------------------------------------------------

void ReportButtonPress() {

  buttonState=!digitalRead(buttonPin);
  
  if (pressed && (ledPinState>0) && (ledPinState!=ledSetPoint)) { //Hold
    brightness=ledPinState;
    ledSetPoint=brightness;
    //Serial.println("Hold Triggered. ");
    longPressPrimer=true;
    longerPressPrimer=true;
    pressed=false;
  }
  if (pressed && (ledPinState==0)) { //Switch on
    ledSetPoint=brightness; // input a setpoint for fading
    //Serial.println("Toggle On Triggered. ");
    longPressPrimer=true;
    longerPressPrimer=true;
    pressed=false;
  }
  if (pressed && (ledPinState>0)) { //Off
    ledSetPoint=0; // input a setpoint for fading
    //Serial.println("Toggle Off Triggered. ");
    longPressPrimer=true;
    longerPressPrimer=true;
    pressed=false;
  }
  else if (buttonState && longPressPrimer && (millis()-pressTime>longPressLength)) { // Full power (or all off)
    longPressPrimer=false;
    brightness=100;
    ledSetPoint=brightness;
    //Serial.println("Full Power Triggered. ");
  }
  //else if (buttonState && longerPressPrimer && (millis()-pressTime>longerPressLength)) { // Longer hold
  //  longerPressPrimer=false;
  //  ledSetPoint=0; // input a setpoint for fading
  //  Serial.println("All Off Triggered. ");
  //}
  else if (!buttonState && (millis()-pressTime>longerPressLength)) {
    longPressPrimer=false;
    longerPressPrimer=false;
  }
}

//-----------------------------------------------------------------------------

void FadeLEDs() {
  if ((millis()-lastFadeTime>fadeSpeed) && (ledPinState<ledSetPoint)) {
    ledPinState = ledPinState + 1;
    analogWrite(ledPin, PWMTable[ledPinState]);
    //Serial.println("LED state is now set to " + String(ledPinState));
    lastFadeTime=millis();
  }
  else if ((millis()-lastFadeTime>fadeSpeed) && (ledPinState > ledSetPoint)) {
    ledPinState = ledPinState - 1;
    analogWrite(ledPin, PWMTable[ledPinState]);
    //Serial.println("LED state is now set to " + String(ledPinState));
    lastFadeTime=millis();
  }
}


