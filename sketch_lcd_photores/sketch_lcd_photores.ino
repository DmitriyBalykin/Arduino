//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

/*******************************************************

This program will test the LCD panel and the buttons
Mark Bramwell, July 2010

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
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
#define displayLight 1
#define displayWater 2

#define setNothing -1
#define setLow 0
#define setHigh 1

int photocellPin = 1;
int photocellReading;
int relayPin = 2;
int waterFlowPin = 3;
bool ledOn = false;
int currentBrightness = 0;
int lightLowThres = 700;
int lightHighThres = 800;

int currentMode = 0;
int currentDisplay = 0;
int currentSet = 0;
int total_pulses = 0;
volatile int pulse;

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
 photocellReading = analogRead(photocellPin);
 if(photocellReading > lightHighThres) { ledOn = false; }
 if(photocellReading < lightLowThres) { ledOn = true; }
  
 if(ledOn) { digitalWrite(relayPin, LOW);}
 else { digitalWrite(relayPin, HIGH);}

 
}

void lightDisplay()
{
  if(currentDisplay != displayLight) { return; }
  
  lcd.setCursor(0,0);
  if(currentSet == setLow){
    lcd.print("Low>");
    if(lcd_key == btnUP) { lightLowThres++; }
    if(lcd_key == btnDOWN) { lightLowThres--; }
  }
  else {
    lcd.print("Low ");
  }
  lcd.setCursor(4,0);
  lcd.print(lightLowThres);

  lcd.setCursor(8,0);
  if(currentSet == setHigh){
    lcd.print("High>");
    if(lcd_key == btnUP) { lightHighThres++; }
    if(lcd_key == btnDOWN) { lightHighThres--; }
  }
  else {
    lcd.print("High ");
  }
  lcd.setCursor(13,0);
  lcd.print(lightHighThres);

  lcd.setCursor(0,1);
  lcd.print("Light:");
  lcd.setCursor(7,1);
  lcd.print(photocellReading);
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
  lcd.print("Main display   ");
}

void displaySwitch()
{
  //if(lcd_key == btnSELECT) { return; }
 lcd_key = read_LCD_buttons();

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
      currentSet = setHigh;
     break;
     }
   case btnLEFT:
     {
      currentSet = setLow;
     break;
     }
   case btnUP:
     {
     break;
     }
   case btnDOWN:
     {
     break;
     }
   case btnSELECT:
     {
      currentDisplay++;
      if(currentDisplay > 2) { currentDisplay = 0; }
     break;
     }
     case btnNONE:
     {
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

 mainDisplay();
 lightDisplay(); //configuring light
 waterDisplay(); //configuring water
 ledSwitch(); //final led action
}


