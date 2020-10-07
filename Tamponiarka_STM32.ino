#include <AccelStepper.h>

// The X Stepper pins
#define STEPPER_X_DIR_PIN PB12
#define STEPPER_X_STEP_PIN PB13
#define STEPPER_X_ENABLE_PIN PA8

#define END_STOP_PIN_X PB10

// The Y Stepper pins
#define STEPPER_Y_DIR_PIN PB14
#define STEPPER_Y_STEP_PIN PB15
#define STEPPER_Y_ENABLE_PIN PA9

#define END_STOP_PIN_Y PB11

#define LED PC13

#define MANUAL_MODE_BUTTON PB1
#define FOOT_SWITCH PB0

#define JOG_X_PIN PA0
#define JOG_Y_PIN PA1
#define JOG_BUTTON PA2

#define SPEED_POT_PIN PA3

// Define some steppers and the pins the will use
AccelStepper stepper_x(AccelStepper::DRIVER, STEPPER_X_STEP_PIN, STEPPER_X_DIR_PIN);
AccelStepper stepper_y(AccelStepper::DRIVER, STEPPER_Y_STEP_PIN, STEPPER_Y_DIR_PIN);

int fullSteps1 = 370;
int fullSteps2 = 1200;
int microSteps = 4;
long distance1 = fullSteps1*microSteps;
long distance2 = fullSteps2*microSteps;

volatile bool modeButtonPressed = false;

bool debounceModeButton = false;

bool manualMode = false;

bool footSwitchMode = true;

bool ledState = false;

bool yHomed = false;

bool lastJogButtonState = true;

bool jogButtonState;

int xValue = 0;
int yValue = 0;

unsigned long currentTime = 0;
unsigned long lastDebounceTime = 0;
byte debounceDelay = 150;

void handleSwitchModeInterrupt()
{
  modeButtonPressed = true;
}

void checkIfModeButtonPressed()
{
//  if(digitalRead(MANUAL_MODE_BUTTON) == HIGH) modeButtonPressed = false;
//  if(modeButtonPressed)
//  {
//    lastDebounceTime = millis();
//    modeButtonPressed = false;
//    debounceModeButton = true;
//  }
//  if((debounceModeButton && (millis() - lastDebounceTime) > debounceDelay))
//  {
//    switchMode();
//    debounceModeButton = false;
//    Serial.print("Manual mode: ");
//    Serial.println(manualMode ? "ON" : "OFF");
//  }
}

void switchMode()
{
  manualMode = !manualMode;
  ledState = !ledState;
  digitalWrite(LED, ledState);
}

boolean pedalConnected()
{
  return digitalRead(MANUAL_MODE_BUTTON);
}

boolean footSwitchPressed()
{
//  if(!footSwitchMode) return true;
//  if(footSwitchMode && digitalRead(FOOT_SWITCH) == LOW) return true;
//  return false;

//  return (footSwitchMode && (!digitalRead(FOOT_SWITCH)));
  if (!pedalConnected()) return true;
  return !digitalRead(FOOT_SWITCH);
//  if(digitalRead(FOOT_SWITCH) == LOW) return true;
//  return false;
}

boolean xEndStopTriggered()
{
  if(digitalRead(END_STOP_PIN_X) == LOW) return true;
  return false;
}

boolean yEndStopTriggered()
{
  if(digitalRead(END_STOP_PIN_Y) == LOW) return true;
  return false;
}

void setup()
{  
  Serial.begin(9600);
  delay(5000);

  Serial.println("Tamponiarka startuje...");
  Serial.print("Podaj ilosc krokow X: ");
  while (Serial.available() == 0);
  fullSteps1 = Serial.parseInt();
  Serial.println(fullSteps1);

    stepper_x.setMinPulseWidth(38);
    stepper_x.setMaxSpeed(2000.0*microSteps);
    stepper_x.setAcceleration(6000.0*microSteps);

    stepper_y.setMinPulseWidth(54);
    stepper_y.setMaxSpeed(3000.0*microSteps);
    stepper_y.setAcceleration(12000.0*microSteps);

    pinMode(END_STOP_PIN_X, INPUT_PULLUP);
    pinMode(END_STOP_PIN_Y, INPUT_PULLUP);
    pinMode(FOOT_SWITCH, INPUT_PULLUP);
    pinMode(MANUAL_MODE_BUTTON, INPUT_PULLUP);
    pinMode(JOG_BUTTON, INPUT_PULLUP);
    
    pinMode(JOG_X_PIN, INPUT);
    pinMode(JOG_Y_PIN, INPUT);
    pinMode(SPEED_POT_PIN, INPUT);
    
    pinMode(LED, OUTPUT);
    pinMode(STEPPER_X_ENABLE_PIN, OUTPUT);
    pinMode(STEPPER_Y_ENABLE_PIN, OUTPUT);

    digitalWrite(STEPPER_X_ENABLE_PIN, LOW);
    digitalWrite(STEPPER_Y_ENABLE_PIN, LOW);
    digitalWrite(LED, ledState);

    //attachInterrupt(MANUAL_MODE_BUTTON, handleSwitchModeInterrupt, FALLING);

    currentTime = millis();

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

unsigned int speedPotValue()
{
  unsigned int analogValue = 0;
  unsigned int sumAnalogValue = 0;
  unsigned int maxAnalogValue = 0;
  unsigned int minAnalogValue = 4095;
  for(int i=0;i<10;i++){
    analogValue = analogRead(SPEED_POT_PIN);
    sumAnalogValue += analogValue;
    if (analogValue > maxAnalogValue) analogValue=maxAnalogValue;
    if (analogValue < minAnalogValue) analogValue=minAnalogValue;
  }
  sumAnalogValue = sumAnalogValue - maxAnalogValue - minAnalogValue;
  analogValue = sumAnalogValue / 8;
  analogValue = constrain(analogValue, 400, 3700);
  analogValue = map(analogValue, 400, 3700, 5, 100);
  return analogValue;
}

void homeY()
{
  Serial.println("Y homing started");
  stepper_y.setMaxSpeed(350*microSteps);
  stepper_y.move(8000*microSteps);
  while (!yEndStopTriggered() && stepper_y.isRunning())
  {
    checkIfModeButtonPressed();
    stepper_y.run();
  }
  stepper_y.setCurrentPosition(0);
  
  stepper_y.setMaxSpeed(30*speedPotValue()*microSteps);
  Serial.println("Y homing finished");
}

void homeX()
{
  Serial.println("X homing started");
  stepper_x.setMaxSpeed(100*microSteps);
  stepper_x.move(4000*microSteps);
  while (!xEndStopTriggered() && stepper_x.isRunning())
  {
    checkIfModeButtonPressed();
    stepper_x.run();
  }
  stepper_x.setCurrentPosition(0);
  stepper_x.setMaxSpeed(20*speedPotValue()*microSteps);
  Serial.println("X homing finished");
}

//void setYDistance()
//{
//  if (!yHomed)
//  {
//    homeY();
//    yHomed = true;
//  } else
//  {
//    distance2 = stepper_y.currentPosition();
//  }
//}

void loop()
{
if (manualMode){

  checkIfModeButtonPressed();
  stepper_x.setSpeed(xValue);
  stepper_y.setSpeed(yValue);
  
  if (xValue >= 0) stepper_x.moveTo(10000*microSteps);
  else stepper_x.moveTo(-10000*microSteps);
  if (yValue >= 0) stepper_y.moveTo(10000*microSteps);
  else stepper_y.moveTo(-10000*microSteps);
  
  if (digitalRead(END_STOP_PIN_X)) stepper_x.runSpeed();
  else if (xValue < 0) stepper_x.runSpeed();
  
  if (digitalRead(END_STOP_PIN_Y)) stepper_y.runSpeed();
  else if (yValue < 0) stepper_y.runSpeed();
  
  if ((millis() - currentTime) > 10)
  {
    currentTime = millis();
    
    xValue = joystickValue(JOG_X_PIN);
    yValue = joystickValue(JOG_Y_PIN);    
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
  
  while (digitalRead(END_STOP_PIN_Y) == HIGH)
  {
    homeY();
  }

  stepper_y.moveTo(-distance2);
  stepper_y.setMaxSpeed(30*speedPotValue()*microSteps);
  Serial.println("Moving up");

  do{
    checkIfModeButtonPressed();
    stepper_y.run();
  }while ((stepper_y.currentPosition() > -distance2) && !manualMode);

  while (digitalRead(END_STOP_PIN_X) == HIGH)
  {
    homeX();
  }

  stepper_x.moveTo(-distance1);
  stepper_x.setMaxSpeed(20*speedPotValue()*microSteps);
  Serial.println("Moving over template");
  
  do{
    checkIfModeButtonPressed();
    stepper_x.run();
  }while ((stepper_x.currentPosition() > -distance1) && !manualMode);

  Serial.println("Waiting for foot switch");

  while(!footSwitchPressed()){}

  Serial.println("Foot switch pressed");

  stepper_y.moveTo(0);
  stepper_y.setMaxSpeed(30*speedPotValue()*microSteps);
  Serial.println("Moving down");

  do{
    if(yEndStopTriggered())
    {
      Serial.println("Y endstop triggered");
      stepper_y.setCurrentPosition(0);
    }
    checkIfModeButtonPressed();
    stepper_y.run();
  }while ((stepper_y.currentPosition() < 0) && !manualMode);

  stepper_y.moveTo(-distance2);
  stepper_y.setMaxSpeed(30*speedPotValue()*microSteps);
  Serial.println("Moving up");

  do{
    checkIfModeButtonPressed();
    stepper_y.run();
  }while ((stepper_y.currentPosition() > -distance2) && !manualMode);

  stepper_x.moveTo(0);
  stepper_x.setMaxSpeed(20*speedPotValue()*microSteps);
  Serial.println("Moving over detail");

  do{
    if(xEndStopTriggered())
    {
      Serial.println("X endstop triggered");
      stepper_x.setCurrentPosition(0);
    }    
    checkIfModeButtonPressed();
    stepper_x.run();
  }while ((stepper_x.currentPosition() < 0) && !manualMode);

  Serial.println("Waiting for foot switch");

  while(!footSwitchPressed()){}

  Serial.println("Foot switch pressed");

  stepper_y.moveTo(0);
  stepper_y.setMaxSpeed(30*speedPotValue()*microSteps);
  Serial.println("Moving down");

  do{
    if(yEndStopTriggered())
    {
      Serial.println("Y endstop triggered");
      stepper_y.setCurrentPosition(0);
    }    
    checkIfModeButtonPressed();
    stepper_y.run();
  }while ((stepper_y.currentPosition() < 0) && !manualMode);

}   
}
