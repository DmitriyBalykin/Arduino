//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

/*******************************************************

This program will test the LCD panel and the buttons
Mark Bramwell, July 2010

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
//int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define displayMode 0
#define menuMode 1
#define settingMode 2

#define setRealTime    0
#define setTimeBounds  1
#define setLight       2
#define setWater       3

#define displayMain 0
#define displayTime 1
#define displayTimer 2
#define displayLowLight 3
#define displayHighLight 4
#define displayWater 5
#define displayStat 6

byte photocellPin = 1;

byte relayPin = 2;
byte waterFlowPin = 3;

bool ledOn = false;
short currentBrightness = 0;
short lightLevel;
short lightLowThresOff = 470;
short lightLowThresOn = 500;
short lightHighThresOn = 100;
short lightHighThresOff = 1050;

byte currentDisplay = 0;
byte valueSet = 0;
short valueDelta = 0;
bool buttonPressed = false;

int total_pulses = 0;
volatile int pulse;

byte second = 0;
byte minute = 0;
byte hour = 0;
unsigned long secondsShift = 0;
bool interruptValueSet = false;

bool useTimer = false;
short timerMinuteOn = 8*60;
short timerMinuteOff = 21*60;

unsigned int ledOnSeconds = 0;
unsigned int ledOnIntervalSeconds = 0;

// read the buttons
short read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;

 return btnNONE;  // when all others fail, return this...
}

void ledSwitch() {
  lightLevel = analogRead(photocellPin);
  bool ledOnOld = ledOn;
  if(useTimer) {
    short timeMinutes = hour*60 + minute;
    if(timeMinutes >= timerMinuteOn && timeMinutes < timerMinuteOff) { ledOn = true; }
    if(timeMinutes < timerMinuteOn || timeMinutes >= timerMinuteOff) { ledOn = false; }
    if(lightLevel > lightHighThresOff) { ledOn = false; }
  }
  else {
    if(lightLevel >= lightLowThresOn && lightLevel <= lightHighThresOn) { ledOn = true; }
    else if(lightLevel < lightLowThresOff || lightLevel > lightHighThresOff) { ledOn = false; }
  }
  if(ledOn) {
    digitalWrite(relayPin, LOW);
    //Serial.println("LED On");
    if(!ledOnOld) {
      ledOnIntervalSeconds = millis()/1000;
    }
  }
  else {
    digitalWrite(relayPin, HIGH);
    //Serial.println("LED Off");
    if(!ledOnOld) {
      ledOnSeconds += millis()/1000 - ledOnIntervalSeconds;
    }
  }
}

void lightDisplay() {
  if(currentDisplay != displayHighLight && currentDisplay != displayLowLight) { return; }
  interruptValueSet = false;
  
  //Serial.print("Show light display: ");
  lcd.setCursor(0,0);
  if(currentDisplay == displayLowLight) {
    lcd.print("Low");
    //Serial.println("Low");
  }
  else {
    lcd.print("High");
    //Serial.println("High");
  }
  lcd.print(". Level:");lcd.print(lightLevel);lcd.print("   ");
    
  lcd.setCursor(0,1);
  lcd.print("Off");
  if(valueSet <= 0){
    valueSet = 0;
    lcd.print(">");
    if(currentDisplay == displayLowLight) {
      lightLowThresOff += valueDelta;
    }
    else {
      lightHighThresOff += valueDelta;
    }
  }

  if(currentDisplay == displayLowLight) {
    lcd.print(lightLowThresOff);
  }
  else {
    lcd.print(lightHighThresOff);
  }

  lcd.print(" On");
  if(valueSet >= 1){
    valueSet = 1;
    lcd.print(">");
    if(currentDisplay == displayLowLight) {
      lightLowThresOn += valueDelta;
    }
    else {
      lightHighThresOn += valueDelta;
    }
  }
  if(currentDisplay == displayLowLight) {
    lcd.print(lightLowThresOn);
  }
  else {
    lcd.print(lightHighThresOn);
  }
  lcd.print("  ");
  //EEPROM.write(lightLowThresOff, ); //save only if value changed
  //EEPROM.write(lightHighThresOff, ); //save only if value changed
  //EEPROM.write(lightLowThresOn, ); //save only if value changed
  //EEPROM.write(lightHighThresOn, ); //save only if value changed
}

void waterDisplay() {
  if(currentDisplay != displayWater) { return; }

  lcd.setCursor(0,0);
  lcd.print("Water flow      ");
  lcd.setCursor(0,1);
  lcd.print("Total: ");
  
  pulse = 0;
  delay(1000);
  total_pulses += pulse;

  int milliliters = total_pulses*7.5;
  int partMilli = milliliters%1000;
  lcd.print(milliliters/1000);
  lcd.print(".");
  if(partMilli < 10){lcd.print("00");}
  else if(partMilli < 100){lcd.print("0");}
  lcd.print(partMilli);
  lcd.print(" L     ");
}

void mainDisplay() {
  if(currentDisplay != displayMain) { return; }
  
  lcd.setCursor(0,0);
  lcd.print("Main display    ");
  lcd.setCursor(0,1);
  lcd.print("Privet!         ");
}

void timeDisplay() {
  if(currentDisplay != displayTime) { return; }
  interruptValueSet = true;
  
  if(valueSet < 0){ valueSet = 0; }
  if(valueSet > 3){ valueSet = 3; }
  
  lcd.setCursor(0,0);
  lcd.print("Time:");
  if(valueDelta == -1) { secondsShift += 86400; }
  if(valueSet == 0) {
    lcd.print(">");
    secondsShift += valueDelta*3600;
  }
  else { lcd.print(" "); }
    
  if(hour < 10) lcd.print("0");
  lcd.print(hour);
  if(valueSet == 1) {
    secondsShift+= valueDelta*60;
    lcd.print(">");
  }
  else { lcd.print(":"); }
  
  if(minute < 10) lcd.print("0");
  lcd.print(minute);
  if(valueSet == 2) {
    secondsShift+= valueDelta;
    lcd.print(">"); 
  }
  else { lcd.print(":"); }
  
  if(second < 10) lcd.print("0");
  lcd.print(second);
  lcd.print(" ");
  
  lcd.setCursor(0,1);
  lcd.print("Use timer?");
  if(valueSet == 3) {
    if(valueDelta > 0) { useTimer = true; }
    else if(valueDelta < 0) { useTimer = false; }
    lcd.print(">");
  }
  else { lcd.print(" "); }
  
  secondsShift %= 86400;
  //EEPROM.write(secondsShift, ); //save only if value changed
  if(useTimer){ lcd.print("Yes"); } else { lcd.print("No "); }
}

void timerDisplay() {
  if(currentDisplay != displayTimer) { return; }
  if(valueSet < 0){ valueSet = 0; }
  if(valueSet > 3){ valueSet = 3; }
  
  lcd.setCursor(0,0);
  lcd.print("On time:");
  if(valueSet == 0) {
    lcd.print(">");
    timerMinuteOn += valueDelta*60;
  }
  else {
    lcd.print(" ");
  }
  byte timerHourOn = (timerMinuteOn/60)%24;
  if(timerHourOn < 10) lcd.print("0");
  lcd.print(timerHourOn);
  if(valueSet == 1) {
    lcd.print(">");
    timerMinuteOn += valueDelta;
  }
  else {
    lcd.print(":");
  }
  if(timerMinuteOn%60 < 10) lcd.print("0");
  lcd.print(timerMinuteOn%60);
  lcd.print("     ");
  
  lcd.setCursor(0,1);
  lcd.print("Off time:");
  if(valueSet == 2) {
    lcd.print(">");
    timerMinuteOff += valueDelta*60;
  }
  else {
    lcd.print(" ");
  }
  byte timerHourOff = (timerMinuteOff/60)%24;
  if(timerHourOff < 10) lcd.print("0");
  lcd.print(timerHourOff);
  if(valueSet == 3) {
    lcd.print(">");
    timerMinuteOff += valueDelta;
  }
  else {
    lcd.print(":");
  }
  if(timerMinuteOff%60 < 10) lcd.print("0");
  lcd.print(timerMinuteOff%60);
  lcd.print("     ");

  //EEPROM.write(timerMinuteOn, ); //save only if value changed
  //EEPROM.write(timerMinuteOff, ); //save only if value changed
}

void statDisplay() {
  if(currentDisplay != displayStat) { return; }
  interruptValueSet = true;
  
  lcd.setCursor(0,0);
  byte ledOnSecondsToTime = ledOnSeconds %60;
  byte ledOnMinutes = (ledOnSeconds / 60) % 60;
  byte ledOnHours = (ledOnSeconds / 3600) % 24;
  lcd.print("Led on: "); lcd.print(ledOnHoursRound);
  lcd.print(":");if(ledOnMinutes < 10 ){ lcd.print("0"); } lcd.print(ledOnMinutes);
  lcd.print(":");if(ledOnSecondsToTime < 10 ){ lcd.print("0"); } lcd.print(ledOnSecondsToTime);
  lcd.setCursor(0,1);
  lcd.print("Press UP to reset.");
  if(valueDelta > 0){
    ledOnSeconds = 0;
  }
}

void timeSet() {
  
  unsigned long seconds = millis()/1000 + secondsShift;
  second = seconds % 60;
  minute = (seconds / 60) % 60;
  hour = (seconds / 3600) % 24;
}

void displaySwitch() {
  int lcd_key = read_LCD_buttons();
  if(interruptValueSet){ valueDelta = 0; interruptValueSet = false; }
  
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
      if(buttonPressed) {break;}
      valueSet++;
      buttonPressed = true;
      break;
     }
   case btnLEFT:
     {
      if(buttonPressed) {break;}
      valueSet--;
      buttonPressed = true;
      break;
     }
   case btnUP:
     {
      if(buttonPressed) { break; }
      buttonPressed = true;
      valueDelta = 1;
      break;
     }
   case btnDOWN:
     {
      if(buttonPressed) { break; }
      buttonPressed = true;
      valueDelta = -1;
      break;
     }
   case btnSELECT:
     {
      if(buttonPressed) { break; }
      buttonPressed = true;
      //Serial.print("Change display to: ");
      currentDisplay++;
      valueSet = 0;
      if(currentDisplay > 6) { currentDisplay = 0; }
      //Serial.println(currentDisplay);
      break;
     }
     case btnNONE:
     {
      buttonPressed = false;
      valueDelta = 0;
      break;
     }
 }
}

void count_pulse() {
  pulse++;
}

void setup() {
 lcd.begin(16, 2);              // start the library

 pinMode(relayPin, OUTPUT);
 pinMode(waterFlowPin, INPUT);

 attachInterrupt(digitalPinToInterrupt(waterFlowPin), count_pulse, RISING);
 //Serial.begin(9600);
 }
 
void loop() {
 /*Serial.print("Analog reading = ");
 Serial.print(photocellReading);*/
 displaySwitch(); //detects pressed key
 timeSet();
 
 mainDisplay();
 lightDisplay(); //configuring light
 waterDisplay(); //configuring water
 timeDisplay(); //configuring time
 timerDisplay(); //configuring timer
 statDisplay(); //configuring timer
 ledSwitch(); //final led action
}
