#include <Arduino.h>
#define MC1 32
#define MC2 31
#define MC3 30
#define MC4 29
#define MC5 25
#define MC6 24
#define MC7 10
#define MC8 6


class StepperMotor {
  public:
    StepperMotor();
    void beginRotation(int);
    void setDirection(int);
    void Rotate(void);
    void Enable(void);
    void Disable(void);
    void Reset(void);
    void setMotorSpeed(int);

    uint32_t motorPeriod;
    bool motorEnabled;
    int stepsRemaining; // this should be an int to avoid underflow errors.
    bool motorRotating;
    int motorPosition;
    int motorDirection;
  private:
    const int directionPin = MC1;
    const int stepPin = MC2;
    const int sleepPin = MC3;
    const int resetPin = MC4;
    const int ms3Pin = MC5;
    const int ms2Pin = MC6;
    const int ms1Pin = MC7;
    const int enablePin = MC8;
};

StepperMotor::StepperMotor(void) {
  
  pinMode(this->directionPin, OUTPUT); 
  pinMode(this->stepPin, OUTPUT);
  pinMode(this->sleepPin, OUTPUT);
  pinMode(this->resetPin, OUTPUT);
  pinMode(this->ms3Pin, OUTPUT);
  pinMode(this->ms2Pin, OUTPUT);
  pinMode(this->ms1Pin, OUTPUT);
  pinMode(this->enablePin, OUTPUT);
  this->Reset();
 
}

void StepperMotor::beginRotation(int numberSteps) {
  this->motorRotating = true;
  
  if(numberSteps < 0) {
    this->setDirection(1);
    numberSteps *= -1;
  }
  else {
    this->setDirection(0);
  }
  this->stepsRemaining = numberSteps;
  
}

void StepperMotor::Rotate(void) {
  this->stepsRemaining -= 1;
  if(this->motorDirection == 0) {
    this->motorPosition += 1;
  }
  else {
    this->motorPosition -= 1;
  }
  digitalWrite(this->stepPin, LOW);
  digitalWrite(this->stepPin, HIGH);
}

void StepperMotor::setDirection(int newDirection) {
  this->motorDirection = newDirection;
  if(newDirection == 0) digitalWrite(this->directionPin, LOW);
  else if (newDirection == 1) digitalWrite(this->directionPin, HIGH);
  else this->motorDirection = 0;
}

void StepperMotor::Disable() {
  digitalWrite(this->enablePin, HIGH);
  this->motorEnabled = 0;
}

void StepperMotor::Enable() {
  digitalWrite(this->enablePin, LOW);
  this->motorEnabled = 1;
}

void StepperMotor::Reset() {
  this->motorPosition = 0;
  this->motorDirection = 0;
  this->motorPeriod = 2;
  this->motorEnabled = 1;
  this->stepsRemaining = 0;
  this->motorRotating = false;
  this->Disable();
  
  digitalWrite(this->ms3Pin, HIGH); // use the finest possible (1/16 step) microstepping
  digitalWrite(this->ms2Pin, HIGH);
  digitalWrite(this->ms1Pin, HIGH);

  digitalWrite(this->directionPin, this->motorDirection);
  digitalWrite(this->stepPin, HIGH);

  digitalWrite(this->sleepPin, HIGH); // sleep is active low
  digitalWrite(this->resetPin, HIGH); // reset is active low
  digitalWrite(this->resetPin, LOW); // reset the controller
  digitalWrite(this->resetPin, HIGH); // go back to an on state
}
