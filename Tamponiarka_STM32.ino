#include <AccelStepper.h>
#include <LiquidCrystal.h>

// The X Stepper pins
#define STEPPER1_DIR_PIN PB12
#define STEPPER1_STEP_PIN PB13

#define END_STOP_PIN_X PB10

// The Y Stepper pins
#define STEPPER2_DIR_PIN PB14
#define STEPPER2_STEP_PIN PB15

#define END_STOP_PIN_Y PB11

#define LED PC13

#define MANUAL_MODE_BUTTON PB1

#define JOG_X_PIN PA0
#define JOG_Y_PIN PA1
#define JOG_BUTTON PA2

// Define some steppers and the pins the will use
AccelStepper stepper1(AccelStepper::DRIVER, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);

LiquidCrystal lcd(PA8, PA9, PB6, PB7, PB8, PB9);

int fullSteps1 = 370;
int fullSteps2 = 1200;
int microSteps = 4;
long distance1 = fullSteps1*microSteps;
long distance2 = fullSteps2*microSteps;

volatile bool manualMode = true;

bool ledState = false;

bool yHomed = false;

bool lastJogButtonState = true;

bool jogButtonState;

int xValue = 0;
int yValue = 0;

byte lcdRefreshTimer = 0;

unsigned long currentTime = 0;
unsigned long lastDebounceTime = 0;
byte debounceDelay = 50;

void switchMode()
{
  manualMode = !manualMode;
  ledState = !ledState;
  digitalWrite(LED, ledState);
}

void setup()
{  

    stepper1.setMinPulseWidth(38);
    stepper1.setMaxSpeed(2000.0*microSteps);
    stepper1.setAcceleration(6000.0*microSteps);

    stepper2.setMinPulseWidth(54);
    stepper2.setMaxSpeed(3000.0*microSteps);
    stepper2.setAcceleration(12000.0*microSteps);

    lcd.begin(16, 2);

    lcd.print("Tamponiarka V0.9");

    delay(3000);

    lcd.clear();

    pinMode(END_STOP_PIN_X, INPUT_PULLUP);
    pinMode(END_STOP_PIN_Y, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    pinMode(MANUAL_MODE_BUTTON, INPUT_PULLUP);
    pinMode(JOG_BUTTON, INPUT_PULLUP);
    pinMode(JOG_X_PIN, INPUT);
    pinMode(JOG_Y_PIN, INPUT);

    attachInterrupt(MANUAL_MODE_BUTTON, switchMode, FALLING);

    currentTime = millis();

}

void homeY()
{
  lcd.clear();
  lcd.print("Homing Y");
  stepper2.setMaxSpeed(350*microSteps);
  stepper2.move(8000*microSteps);
  while (digitalRead(END_STOP_PIN_Y) == HIGH && stepper2.isRunning())
  {
    stepper2.run();
  }
  stepper2.setCurrentPosition(0);
  stepper2.setMaxSpeed(3000*microSteps);
}

void homeX()
{
  stepper1.setMaxSpeed(100*microSteps);
  stepper1.move(4000*microSteps);
  while (digitalRead(END_STOP_PIN_X) == HIGH && stepper1.isRunning())
  {
    stepper1.run();
  }
  stepper2.setCurrentPosition(0);
}

int joystickValue(int pin)
{
  int analogValue = analogRead(pin);
  analogValue = map(analogValue, 0, 4095, -100, 100);
  if (abs(analogValue) < 15)
  {
    analogValue=0;
  }
  return analogValue*microSteps;
}

void setYDistance()
{
  if (!yHomed)
  {
    homeY();
    yHomed = true;
  } else
  {
    distance2 = stepper2.currentPosition();
  }
}

void loop()
{
if (manualMode){
  
  stepper1.setSpeed(xValue);
  stepper2.setSpeed(yValue);
  
  if (xValue >= 0) stepper1.moveTo(10000*microSteps);
  else stepper1.moveTo(-10000*microSteps);
  if (yValue >= 0) stepper2.moveTo(10000*microSteps);
  else stepper2.moveTo(-10000*microSteps);
  
  if (digitalRead(END_STOP_PIN_X)) stepper1.runSpeed();
  else if (xValue < 0) stepper1.runSpeed();
  
  if (digitalRead(END_STOP_PIN_Y)) stepper2.runSpeed();
  else if (yValue < 0) stepper2.runSpeed();
  
  if ((millis() - currentTime) > 10)
  {
    currentTime = millis();
    
    xValue = joystickValue(JOG_X_PIN);
    yValue = joystickValue(JOG_Y_PIN);
    
    lcdRefreshTimer++;

    if (lcdRefreshTimer > 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("X: ");
      lcd.print(stepper1.currentPosition());
      lcd.setCursor(10, 0);
      lcd.print(stepper1.speed());
      //lcd.print(xValue);
      lcd.setCursor(0, 1);
      lcd.print("Y: ");
      lcd.print(stepper2.currentPosition());
      lcd.setCursor(10, 1);
      lcd.print(stepper2.speed());
      //lcd.print(yValue);
      lcdRefreshTimer = 0;
    }
  }

//  bool jogButtonReading = digitalRead(JOG_BUTTON);
//
//  if (jogButtonReading != lastJogButtonState) lastDebounceTime = millis();
//
//  if ((millis() - lastDebounceTime) > debounceDelay)
//  {
//    if (jogButtonReading != jogButtonState)
//    {
//      jogButtonState = jogButtonReading;
//      if (!jogButtonState) setYDistance();
//    }
//  }
//
//  lastJogButtonState = jogButtonReading;
  
} else
{  
//  while (digitalRead(END_STOP_PIN_X) == HIGH)
//  {
//    homeX();
//  }
//

  while (digitalRead(END_STOP_PIN_Y) == HIGH)
  {
    homeY();
  }

  lcd.clear();
  lcd.print("Drukowanie");

  stepper2.moveTo(-distance2);
    
  do{
    stepper2.run();
  }while ((stepper2.currentPosition() > -distance2) || !manualMode);

  stepper1.moveTo(-distance1);
  
  do{
    stepper1.run();
  }while ((stepper1.currentPosition() > -distance1) || !manualMode);

  stepper2.moveTo(0);  

  do{
    stepper2.run();
  }while ((stepper2.currentPosition() < 0) || !manualMode);

  stepper2.moveTo(-distance2);

  do{
    stepper2.run();
  }while ((stepper2.currentPosition() > -distance2) || !manualMode);

  stepper1.moveTo(0);

  do{
    stepper1.run();
  }while ((stepper1.currentPosition() < 0) || !manualMode);

  stepper2.moveTo(0);

  do{
    stepper2.run();
  }while ((stepper2.currentPosition() < 0) || !manualMode);

}   
}
