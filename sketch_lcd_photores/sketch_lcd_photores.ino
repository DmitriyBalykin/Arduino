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
#define displayLowLight 2
#define displayHighLight 3
#define displayWater 4

int photocellPin = 1;

int relayPin = 2;
int waterFlowPin = 3;

bool ledOn = false;
int currentBrightness = 0;
int lightLevel;
int lightLowThresOff = 300;
int lightLowThresOn = 350;
int lightHighThresOn = 700;
int lightHighThresOff = 800;

int currentDisplay = 0;
int valueSet = 0;
int valueDelta = 0;
bool displayChanged = false;

int total_pulses = 0;
volatile int pulse;

int second = 0;
int minute = 0;
int hour = 0;
bool useTimer = false;

// read the buttons
int read_LCD_buttons()
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

void ledSwitch()
{
  /*lcd.setCursor(0,1);*/
 lightLevel = analogRead(photocellPin);
  if((!useTimer || lightLevel >= lightLowThresOn) && lightLevel <= lightHighThresOn) { ledOn = true; }
  else if((useTimer && lightLevel < lightLowThresOff)|| lightLevel > lightHighThresOff) { ledOn = false; }  
  
 if(ledOn) { digitalWrite(relayPin, LOW);}
 else { digitalWrite(relayPin, HIGH);}
}

void lightDisplay()
{
  if(currentDisplay != displayHighLight || currentDisplay != displayLowLight) { return; }

  lcd.setCursor(0,0);
  if(currentDisplay == displayLowLight)
  {
    lcd.print("High");
  }
  else
  {
    lcd.print("Low");
  }
  lcd.print(". Level:");lcd.print(lightLevel);lcd.print("   ");
    
  lcd.setCursor(0,1);
  lcd.print("Off");
  if(valueSet == 1){
    lcd.print(">");
    if(currentDisplay == displayLowLight)
    {
      lightLowThresOff += valueDelta;
    }
    else
    {
      lightHighThresOff += valueDelta;
    }
  }

  if(currentDisplay == displayLowLight)
  {
    lcd.print(lightLowThresOff);
  }
  else
  {
    lcd.print(lightHighThresOff);
  }

  lcd.print(" On");
  if(valueSet == 0){
    lcd.print(">");
    if(currentDisplay == displayLowLight)
    {
      lightLowThresOn += valueDelta;
    }
    else
    {
      lightHighThresOn += valueDelta;
    }
  }
  if(currentDisplay == displayLowLight)
  {
    lcd.print(lightLowThresOn);
  }
  else
  {
    lcd.print(lightHighThresOn);
  }
  lcd.print("   ");
}

void waterDisplay()
{
  if(currentDisplay != displayWater) { return; }

  pulse = 0;

  interrupts();
  delay(1000);
  noInterrupts();
  total_pulses += pulse;
  lcd.setCursor(0,0);
  lcd.print("Water flow      ");
  lcd.setCursor(0,1);
  lcd.print("Total: ");
  lcd.print(total_pulses/450);
  lcd.print("   L");
}

void mainDisplay()
{
  if(currentDisplay != displayMain) { return; }
  
  lcd.setCursor(0,0);
  lcd.print("Main display    ");
  lcd.setCursor(0,1);
  lcd.print("Privet!         ");
}

void timeDisplay()
{
  if(currentDisplay != displayTime) { return; }
  
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute);
  lcd.print(":");
  lcd.print(second);
  lcd.print("   ");
  
  lcd.setCursor(0,1);
  lcd.print("Use timer? ");
  if(useTimer){ lcd.print("Yes"); } else { lcd.print("No"); }
}

void timeSet()
{
  if(millis()%1000 != 0) return;
  second = 60*millis()%(1000*60);
  minute = 60*60*millis()%(1000*60*60);
  hour = 24*60*60*millis()%(1000*60*60*24);
}

void displaySwitch()
{
 int lcd_key = read_LCD_buttons();

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
      valueSet++;
      break;
     }
   case btnLEFT:
     {
      valueSet--;
      break;
     }
   case btnUP:
     {
      valueDelta = 1;
      break;
     }
   case btnDOWN:
     {
      valueDelta = -1;
      break;
     }
   case btnSELECT:
     {
      if(displayChanged) { break; }
      displayChanged = true;
      currentDisplay++;
      if(currentDisplay > 4) { currentDisplay = 0; }
     break;
     }
     case btnNONE:
     {
      displayChanged = false;
      valueDelta = 0;
      break;
     }
 }
}

void count_pulse()
{
  pulse++;
}

void setup()
{
 lcd.begin(16, 2);              // start the library

 pinMode(relayPin, OUTPUT);
 pinMode(waterFlowPin, INPUT);

 attachInterrupt(digitalPinToInterrupt(waterFlowPin), count_pulse, RISING);
 /*Serial.begin(9600);*/
 }
 
void loop()
{
 /*Serial.print("Analog reading = ");
 Serial.print(photocellReading);*/
 displaySwitch(); //detects pressed key
 timeSet();

 mainDisplay();
 lightDisplay(); //configuring light
 waterDisplay(); //configuring water
 timeDisplay(); //configuring time
 ledSwitch(); //final led action
}


