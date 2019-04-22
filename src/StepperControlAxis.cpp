#include "StepperControlAxis.h"
//#include <TMC2130Stepper.h>


#if defined(FARMDUINO_EXP_V20)
  static TMC2130Stepper TMC2130X = TMC2130Stepper(X_ENABLE_PIN, X_DIR_PIN, X_STEP_PIN, X_CHIP_SELECT);
  static TMC2130Stepper TMC2130Y = TMC2130Stepper(Y_ENABLE_PIN, Y_DIR_PIN, Y_STEP_PIN, Y_CHIP_SELECT);
  static TMC2130Stepper TMC2130Z = TMC2130Stepper(Z_ENABLE_PIN, Z_DIR_PIN, Z_STEP_PIN, Z_CHIP_SELECT);
  static TMC2130Stepper TMC2130E = TMC2130Stepper(E_ENABLE_PIN, E_DIR_PIN, E_STEP_PIN, E_CHIP_SELECT);
#endif


StepperControlAxis::StepperControlAxis()
{
  lastCalcLog = 0;

  pinStep = 0;
  pinDirection = 0;
  pinEnable = 0;

  pin2Step = 0;
  pin2Direction = 0;
  pin2Enable = 0;

  pinMin = 0;
  pinMax = 0;

  axisActive = false;

  coordSourcePoint = 0;
  coordCurrentPoint = 0;
  coordDestinationPoint = 0;
  coordHomeAxis = 0;

  movementUp = false;
  movementToHome = false;
  movementAccelerating = false;
  movementDecelerating = false;
  movementCruising = false;
  movementCrawling = false;
  movementMotorActive = false;
  movementMoving = false;

  stepIsOn = false;

  setMotorStepWrite = &StepperControlAxis::setMotorStepWriteDefault;
  setMotorStepWrite2 = &StepperControlAxis::setMotorStepWriteDefault2;
  resetMotorStepWrite = &StepperControlAxis::resetMotorStepWriteDefault;
  resetMotorStepWrite2 = &StepperControlAxis::resetMotorStepWriteDefault2;

/**/
/*
#if defined(FARMDUINO_EXP_V20)
  //// TMC2130 Functions

  setMotorStepWrite = &StepperControlAxis::setMotorStepWriteTMC2130;
  setMotorStepWrite2 = &StepperControlAxis::setMotorStepWriteTMC2130_2;
  resetMotorStepWrite = &StepperControlAxis::resetMotorStepWriteTMC2130;
  resetMotorStepWrite2 = &StepperControlAxis::resetMotorStepWriteTMC2130_2;
#endif
*/
}

unsigned int StepperControlAxis::getLostSteps()
{
  unsigned int lostSteps;
  lostSteps = TMC2130A->LOST_STEPS();

  if (lostSteps != 0)
  {
    Serial.print("R99");
    Serial.print(" mis stp = ");
    Serial.print(lostSteps);
    Serial.print("\r\n");
  }

  return lostSteps;
  //return TMC2130A->LOST_STEPS();
}

void StepperControlAxis::test()
{
  Serial.print("R99");
  Serial.print(" mis stp = ");
  Serial.print(TMC2130A->LOST_STEPS());
  Serial.print("\r\n");


  //setMotorStep();
  //setMotorStepWriteTMC2130();
  //Serial.print("R99");
  //Serial.print(" cur loc = ");
  //Serial.print(coordCurrentPoint);
  //Serial.print(" enc loc = ");
  //Serial.print(coordEncoderPoint);
  //Serial.print(" cons steps missed = ");
  //Serial.print(label);
  //Serial.print(consMissedSteps);
  //Serial.print("\r\n");
}

#if defined(FARMDUINO_EXP_V20)
void StepperControlAxis::initTMC2130(int motorCurrent, int  stallSensitivity)
{
  /*
  TMC2130X.begin(); // Initiate pins and registeries
  TMC2130X.SilentStepStick2130(600); // Set stepper current to 600mA
  TMC2130X.stealthChop(1); // Enable extremely quiet stepping
  TMC2130X.shaft_dir(0);
  */

  /**/
  if (channelLabel == 'X')
  {
    TMC2130A = &TMC2130X;
    TMC2130B = &TMC2130E;
  }
  if (channelLabel == 'Y')
  {
    TMC2130A = &TMC2130Y;
  }
  if (channelLabel == 'Z')
  {
    TMC2130A = &TMC2130Z;
  }

  TMC2130A->begin();                      // Initiate pins and registeries

  TMC2130A->rms_current(motorCurrent);    // Set the required current in mA  
  TMC2130A->microsteps(0);                // Minimum of micro steps needed
  TMC2130A->chm(true);                    // Set the chopper mode to classic const. off time
  TMC2130A->diag1_stall(1);               // Activate stall diagnostics
  TMC2130A->sgt(stallSensitivity);        // Set stall detection sensitivity. most -64 to +64 least
  TMC2130A->shaft_dir(0);                 // Set direction

  //TMC2130A->SilentStepStick2130(600); // Set stepper current to 600mA
  //TMC2130A->stealthChop(1); // Enable extremely quiet stepping
  //TMC2130A->microsteps(0);

  if (channelLabel == 'X')
  {
    TMC2130B->begin();                    // Initiate pins and registeries

    TMC2130B->rms_current(motorCurrent);   // Set the required current in mA  
    TMC2130B->microsteps(0);               // Minimum of micro steps needed
    TMC2130B->chm(true);                   // Set the chopper mode to classic const. off time
    TMC2130B->diag1_stall(1);              // Activate stall diagnostics
    TMC2130B->sgt(stallSensitivity);       // Set stall detection sensitivity. most -64 to +64 least
    TMC2130B->shaft_dir(0);                // Set direction

                       
    //TMC2130B->SilentStepStick2130(600); // Set stepper current to 600mA
    //TMC2130B->stealthChop(1); // Enable extremely quiet stepping
    //TMC2130B->shaft_dir(0);
  }

  setMotorStepWrite = &StepperControlAxis::setMotorStepWriteTMC2130;
  setMotorStepWrite2 = &StepperControlAxis::setMotorStepWriteTMC2130_2;
  resetMotorStepWrite = &StepperControlAxis::resetMotorStepWriteTMC2130;
  resetMotorStepWrite2 = &StepperControlAxis::resetMotorStepWriteTMC2130_2;

}

bool StepperControlAxis::stallDetected() {
  return TMC2130A->stallguard();
}

#endif

unsigned int StepperControlAxis::calculateSpeed(long sourcePosition, long currentPosition, long destinationPosition, long minSpeed, long maxSpeed, long stepsAccDec)
{

  int newSpeed = 0;

  long curPos = abs(currentPosition);

  long staPos;
  long endPos;

  movementAccelerating = false;
  movementDecelerating = false;
  movementCruising = false;
  movementCrawling = false;
  movementMoving = false;


  /*
  if (abs(sourcePosition) < abs(destinationPosition))
  {
    staPos = abs(sourcePosition);
    endPos = abs(destinationPosition);
  }
  else
  {
    staPos = abs(destinationPosition);
    endPos = abs(sourcePosition);
  }
  */

  // Set the possible negative coordinates to all positive numbers
  // so the calculation code still works after the changes
  staPos = 0;
  endPos = abs(destinationPosition - sourcePosition);
    
  if (sourcePosition < destinationPosition)
  {
    curPos = currentPosition - sourcePosition;
  }
  else
  {
    curPos = currentPosition - destinationPosition;
  }


  unsigned long halfway = ((endPos - staPos) / 2) + staPos;
  //unsigned long halfway = ((destinationPosition - sourcePosition) / 2) + sourcePosition;

  // Set the homing speed if the position would be out of bounds
  if (
        (curPos < staPos || curPos > endPos)
        // || 
        // Also limit the speed to a crawl when the move would pass the home position
        // (sourcePosition > 0 && destinationPosition < 0) || (sourcePosition < 0 && destinationPosition > 0)
        // (!motorHomeIsUp && currentPosition <= 0) || (motorHomeIsUp && currentPosition >= 0) ||)
     )
  {
    newSpeed = motorSpeedHome;
    //newSpeed = minSpeed;
    movementCrawling = true;
    movementMoving = true;
  }
  else
  {
    if (curPos >= staPos && curPos <= halfway)
    {
      // accelerating
      if (curPos > (staPos + stepsAccDec))
      {
        // now beyond the accelleration point to go full speed
        newSpeed = maxSpeed + 1;
        movementCruising = true;
        movementMoving = true;
      }
      else
      {
        // speeding up, increase speed linear within the first period
        newSpeed = (1.0 * (curPos - staPos) / stepsAccDec * (maxSpeed - minSpeed)) + minSpeed;
        movementAccelerating = true;
        movementMoving = true;
      }
    }
    else
    {
      // decelerating
      if (curPos < (endPos - stepsAccDec))
      {
        // still before the deceleration point so keep going at full speed
        newSpeed = maxSpeed + 2;
        movementCruising = true;
        movementMoving = true;
      }
      else
      {
        // speeding up, increase speed linear within the first period
        newSpeed = (1.0 * (endPos - curPos) / stepsAccDec * (maxSpeed - minSpeed)) + minSpeed;
        movementDecelerating = true;
        movementMoving = true;
      }
    }
  }



  if (debugPrint && (millis() - lastCalcLog > 1000))
  {

    lastCalcLog = millis();

    Serial.print("R99");

    Serial.print(" sta ");
    Serial.print(staPos);
    Serial.print(" cur ");
    Serial.print(curPos);
    Serial.print(" end ");
    Serial.print(endPos);
    Serial.print(" half ");
    Serial.print(halfway);
    Serial.print(" len ");
    Serial.print(stepsAccDec);
    Serial.print(" min ");
    Serial.print(minSpeed);
    Serial.print(" max ");
    Serial.print(maxSpeed);
    Serial.print(" spd ");

    Serial.print(" ");
    Serial.print(newSpeed);

    Serial.print("\r\n");
  }

  // Return the calculated speed, in steps per second
  return newSpeed;
}

void StepperControlAxis::checkAxisDirection()
{

  if (coordHomeAxis)
  {
    // When home is active, the direction is fixed
    movementUp = motorHomeIsUp;
    movementToHome = true;
  }
  else
  {
    // For normal movement, move in direction of destination
    movementUp = (coordCurrentPoint < coordDestinationPoint);
    movementToHome = (abs(coordCurrentPoint) >= abs(coordDestinationPoint));
  }

  if (coordCurrentPoint == 0)
  {
    // Go slow when theoretical end point reached but there is no end stop siganl
    axisSpeed = motorSpeedMin;
  }
}

void StepperControlAxis::setDirectionAxis()
{

  if (((!coordHomeAxis && coordCurrentPoint < coordDestinationPoint) || (coordHomeAxis && motorHomeIsUp)))
  {
    setDirectionUp();
  }
  else
  {
    setDirectionDown();
  }
}

void StepperControlAxis::checkMovement()
{

  checkAxisDirection();

  // Handle movement if destination is not already reached or surpassed
  if (
      (
          (coordDestinationPoint > coordSourcePoint && coordCurrentPoint < coordDestinationPoint) ||
          (coordDestinationPoint < coordSourcePoint && coordCurrentPoint > coordDestinationPoint) ||
          coordHomeAxis) &&
      axisActive)
  {

    // home or destination not reached, keep moving

    // If end stop reached or the encoder doesn't move anymore, stop moving motor, otherwise set the timing for the next step
    if ((coordHomeAxis && !endStopAxisReached(false)) || (!coordHomeAxis && !endStopAxisReached(!movementToHome)))
    {

      // Get the axis speed, in steps per second
      axisSpeed = calculateSpeed(coordSourcePoint, coordCurrentPoint, coordDestinationPoint,
                                 motorSpeedMin, motorSpeedMax, motorStepsAcc);

//      // Set the moments when the step is set to true and false
//      if (axisSpeed > 0)
//      {

        // Take the requested speed (steps / second) and divide by the interrupt speed (interrupts per seconde)
        // This gives the number of interrupts (called ticks here) before the pulse needs to be set for the next step
//        stepOnTick = moveTicks + (1000.0 * 1000.0 / motorInterruptSpeed / axisSpeed / 2);
//        stepOffTick = moveTicks + (1000.0 * 1000.0 / motorInterruptSpeed / axisSpeed);
//      }
    }
    else
    {
      axisActive = false;
    }
  }
  else
  {
    // Destination or home reached. Deactivate the axis.
    axisActive = false;
  }

  // If end stop for home is active, set the position to zero
  if (endStopAxisReached(false))
  {
    coordCurrentPoint = 0;
  }
}

void StepperControlAxis::incrementTick()
{
  if (axisActive)
  {
    moveTicks++;
    //moveTicks+=3;
  }
}

void StepperControlAxis::checkTiming()
{

  if (stepIsOn)
  {
    if (moveTicks >= stepOffTick)
    {

      // Negative flank for the steps
      resetMotorStep();
      setTicks();
    }
  }
  else
  {
    if (axisActive)
    {
      if (moveTicks >= stepOnTick)
      {

        // Positive flank for the steps
        setStepAxis();
      }
    }
  }
}

void StepperControlAxis::setTicks()
{
  // Take the requested speed (steps / second) and divide by the interrupt speed (interrupts per seconde)
  // This gives the number of interrupts (called ticks here) before the pulse needs to be set for the next step
  stepOnTick = moveTicks + (1000.0 * 1000.0 / motorInterruptSpeed / axisSpeed / 2);
  stepOffTick = moveTicks + (1000.0 * 1000.0 / motorInterruptSpeed / axisSpeed);
}

void StepperControlAxis::setStepAxis()
{

  stepIsOn = true;

  if (movementUp)
  {
    coordCurrentPoint++;
  }
  else
  {
    coordCurrentPoint--;
  }

  // set a step on the motors
  setMotorStep();
}

bool StepperControlAxis::endStopAxisReached(bool movement_forward)
{

  bool min_endstop = false;
  bool max_endstop = false;
  bool invert = false;

  if (motorEndStopInv)
  {
    invert = true;
  }

  // for the axis to check, retrieve the end stop status

  if (!invert)
  {
    min_endstop = endStopMin();
    max_endstop = endStopMax();
  }
  else
  {
    min_endstop = endStopMax();
    max_endstop = endStopMin();
  }

  // if moving forward, only check the end stop max
  // for moving backwards, check only the end stop min

  if ((!movement_forward && min_endstop) || (movement_forward && max_endstop))
  {
    return 1;
  }

  return 0;
}

void StepperControlAxis::StepperControlAxis::loadPinNumbers(int step, int dir, int enable, int min, int max, int step2, int dir2, int enable2)
{
  pinStep = step;
  pinDirection = dir;
  pinEnable = enable;

  pin2Step = step2;
  pin2Direction = dir2;
  pin2Enable = enable2;

  pinMin = min;
  pinMax = max;
}

void StepperControlAxis::loadMotorSettings(
    long speedMax, long speedMin, long speedHome, long stepsAcc, long timeOut, bool homeIsUp, bool motorInv,
    bool endStInv, bool endStInv2, long interruptSpeed, bool motor2Enbl, bool motor2Inv, bool endStEnbl,
    bool stopAtHome, long maxSize, bool stopAtMax)
{

  motorSpeedMax = speedMax;
  motorSpeedMin = speedMin;
  motorSpeedHome = speedHome;
  motorStepsAcc = stepsAcc;
  motorTimeOut = timeOut;
  motorHomeIsUp = homeIsUp;
  motorMotorInv = motorInv;
  motorEndStopInv = endStInv;
  motorEndStopInv2 = endStInv2;
  motorEndStopEnbl = endStEnbl;
  motorInterruptSpeed = interruptSpeed;
  motorMotor2Enl = motor2Enbl;
  motorMotor2Inv = motor2Inv;
  motorStopAtHome = stopAtHome;
  motorMaxSize = maxSize;
  motorStopAtMax = stopAtMax;

  if (pinStep == 54)
  {
    setMotorStepWrite = &StepperControlAxis::setMotorStepWrite54;
    resetMotorStepWrite = &StepperControlAxis::resetMotorStepWrite54;
  }
  
  if (pinStep == 60)
  {
    setMotorStepWrite = &StepperControlAxis::setMotorStepWrite60;
    resetMotorStepWrite = &StepperControlAxis::resetMotorStepWrite60;
  }
  

  if (pinStep == 46)
  {
    setMotorStepWrite = &StepperControlAxis::setMotorStepWrite46;
    resetMotorStepWrite = &StepperControlAxis::resetMotorStepWrite46;
  }

  if (pin2Step == 26)
  {
    setMotorStepWrite2 = &StepperControlAxis::setMotorStepWrite26;
    resetMotorStepWrite2 = &StepperControlAxis::resetMotorStepWrite26;
  }

}

bool StepperControlAxis::loadCoordinates(long sourcePoint, long destinationPoint, bool home)
{

  coordSourcePoint = sourcePoint;
  coordCurrentPoint = sourcePoint;
  coordDestinationPoint = destinationPoint;
  coordHomeAxis = home;

  bool changed = false;

  // Limit normal movement to the home position

  if (motorStopAtHome)
  {
    if (!motorHomeIsUp && coordDestinationPoint < 0)
    {
      coordDestinationPoint = 0;
      changed = true;
    }

    if (motorHomeIsUp && coordDestinationPoint > 0)
    {
      coordDestinationPoint = 0;
      changed = true;
    }
  }

  // limit the maximum size the bot can move, when there is a size present
  if (motorMaxSize > 0 && motorStopAtMax)
  {
    if (abs(coordDestinationPoint) > abs(motorMaxSize))
    {
      if (coordDestinationPoint < 0)
      {
        coordDestinationPoint = -abs(motorMaxSize);
        changed = true;
      }
      else
      {
        coordDestinationPoint = abs(motorMaxSize);
        changed = true;
      }
    }
  }

  // Initialize movement variables
  moveTicks = 0;
  axisActive = true;

  return changed;
}

void StepperControlAxis::enableMotor()
{
  digitalWrite(pinEnable, LOW);
  if (motorMotor2Enl)
  {
    digitalWrite(pin2Enable, LOW);
  }
  movementMotorActive = true;
}

void StepperControlAxis::disableMotor()
{
  digitalWrite(pinEnable, HIGH);
  if (motorMotor2Enl)
  {
    digitalWrite(pin2Enable, HIGH);
  }
  movementMotorActive = false;
}

void StepperControlAxis::setDirectionUp()
{

#if !defined(FARMDUINO_EXP_V20)
  if (motorMotorInv)
  {
    digitalWrite(pinDirection, LOW);
  }
  else
  {
    digitalWrite(pinDirection, HIGH);
  }

  if (motorMotor2Enl && motorMotor2Inv)
  {
    digitalWrite(pin2Direction, LOW);
  }
  else
  {
    digitalWrite(pin2Direction, HIGH);
  }
#endif

/**/

#if defined(FARMDUINO_EXP_V20)

  // The TMC2130 uses a command to change direction, not a pin
  if (motorMotorInv)
  {
    TMC2130A->shaft_dir(0);
  }
  else
  {
    TMC2130A->shaft_dir(1);
  }

  if (motorMotor2Enl && motorMotor2Inv)
  {
    TMC2130B->shaft_dir(0);
  }
  else
  {
    TMC2130B->shaft_dir(1);
  }

#endif

}

void StepperControlAxis::setDirectionDown()
{
  #if !defined(FARMDUINO_EXP_V20)

  if (motorMotorInv)
  {
    digitalWrite(pinDirection, HIGH);
  }
  else
  {
    digitalWrite(pinDirection, LOW);
  }

  if (motorMotor2Enl && motorMotor2Inv)
  {
    digitalWrite(pin2Direction, HIGH);
  }
  else
  {
    digitalWrite(pin2Direction, LOW);
  }

  #endif

  /**/

  #if defined(FARMDUINO_EXP_V20)

  // The TMC2130 uses a command to change direction, not a pin
  if (motorMotorInv)
  {
    TMC2130A->shaft_dir(1);
  }
  else
  {
    TMC2130A->shaft_dir(0);
  }

  if (motorMotor2Enl && motorMotor2Inv)
  {
    TMC2130B->shaft_dir(1);
  }
  else
  {
    TMC2130B->shaft_dir(0);
  }

  #endif

}

void StepperControlAxis::setMovementUp()
{
  movementUp = true;
}

void StepperControlAxis::setMovementDown()
{
  movementUp = false;
}

void StepperControlAxis::setDirectionHome()
{
  if (motorHomeIsUp)
  {
    setDirectionUp();
    setMovementUp();
  }
  else
  {
    setDirectionDown();
    setMovementDown();
  }
}

void StepperControlAxis::setDirectionAway()
{
  if (motorHomeIsUp)
  {
    setDirectionDown();
    setMovementDown();
  }
  else
  {
    setDirectionUp();
    setMovementUp();
  }
}

unsigned long StepperControlAxis::getLength(long l1, long l2)
{
  if (l1 > l2)
  {
    return l1 - l2;
  }
  else
  {
    return l2 - l1;
  }
}

bool StepperControlAxis::endStopsReached()
{
  return ((digitalRead(pinMin) == motorEndStopInv2) || (digitalRead(pinMax) == motorEndStopInv2)) && motorEndStopEnbl;
}

bool StepperControlAxis::endStopMin()
{
  //return ((digitalRead(pinMin) == motorEndStopInv) || (digitalRead(pinMax) == motorEndStopInv));
  return ((digitalRead(pinMin) == motorEndStopInv2) && motorEndStopEnbl);
}

bool StepperControlAxis::endStopMax()
{
  //return ((digitalRead(pinMin) == motorEndStopInv) || (digitalRead(pinMax) == motorEndStopInv));
  return ((digitalRead(pinMax) == motorEndStopInv2) && motorEndStopEnbl);
}

bool StepperControlAxis::isAxisActive()
{
  return axisActive;
}

void StepperControlAxis::deactivateAxis()
{
  axisActive = false;
}

void StepperControlAxis::setMotorStep()
{
  stepIsOn = true;

  //digitalWrite(pinStep, HIGH);
  (this->*setMotorStepWrite)();

  if (pin2Enable)
  {
    (this->*setMotorStepWrite2)();
    //digitalWrite(pin2Step, HIGH);
  }
}

void StepperControlAxis::resetMotorStep()
{
  stepIsOn = false;
  movementStepDone = true;

  digitalWrite(pinStep, LOW);
  //(this->*resetMotorStepWrite)();

  if (pin2Enable)
  {
    digitalWrite(pin2Step, LOW);
    //(this->*resetMotorStepWrite2)();
  }
}

bool StepperControlAxis::pointReached(long currentPoint, long destinationPoint)
{
  return (destinationPoint == currentPoint);
}

long StepperControlAxis::currentPosition()
{
  return coordCurrentPoint;
}

void StepperControlAxis::setCurrentPosition(long newPos)
{
  coordCurrentPoint = newPos;
}

long StepperControlAxis::destinationPosition()
{
  return coordDestinationPoint;
}

void StepperControlAxis::setMaxSpeed(long speed)
{
  motorSpeedMax = speed;
}

void StepperControlAxis::activateDebugPrint()
{
  debugPrint = true;
}

bool StepperControlAxis::isStepDone()
{
  return movementStepDone;
}

void StepperControlAxis::resetStepDone()
{
  movementStepDone = false;
}

bool StepperControlAxis::movingToHome()
{
  return movementToHome;
}

bool StepperControlAxis::movingUp()
{
  return movementUp;
}

bool StepperControlAxis::isAccelerating()
{
  return movementAccelerating;
}

bool StepperControlAxis::isDecelerating()
{
  return movementDecelerating;
}

bool StepperControlAxis::isCruising()
{
  return movementCruising;
}

bool StepperControlAxis::isCrawling()
{
  return movementCrawling;
}

bool StepperControlAxis::isMotorActive()
{
  return movementMotorActive;
}

/// Functions for pin writing using alternative method

// Pin write default functions
void StepperControlAxis::setMotorStepWriteDefault()
{
  digitalWrite(pinStep, HIGH);
}

void StepperControlAxis::setMotorStepWriteDefault2()
{
  digitalWrite(pin2Step, HIGH);
}

void StepperControlAxis::resetMotorStepWriteDefault()
{
  digitalWrite(pinStep, LOW);
}

void StepperControlAxis::resetMotorStepWriteDefault2()
{
  digitalWrite(pin2Step, LOW);
}

// X step
void StepperControlAxis::setMotorStepWrite54()
{
  //PF0
  PORTF |= B00000001;
}

void StepperControlAxis::resetMotorStepWrite54()
{
  //PF0
  PORTF &= B11111110;
}


// X step 2
void StepperControlAxis::setMotorStepWrite26()
{
  //PA4
  PORTA |= B00010000;
}

void StepperControlAxis::resetMotorStepWrite26()
{
  PORTA &= B11101111;
}

// Y step
void StepperControlAxis::setMotorStepWrite60()
{
  //PF6
  PORTF |= B01000000;
}

void StepperControlAxis::resetMotorStepWrite60()
{
  //PF6
  PORTF &= B10111111;
}

// Z step
void StepperControlAxis::setMotorStepWrite46()
{
  //PL3
  PORTL |= B00001000;
}

void StepperControlAxis::resetMotorStepWrite46()
{
  //PL3
  PORTL &= B11110111;
}

#if defined(FARMDUINO_EXP_V20)
//// TMC2130 Functions

void StepperControlAxis::setMotorStepWriteTMC2130()
{
  // TMC2130 works on each edge of the step pulse, 
  // so instead of setting the step bit, 
  // toggle the bit here

  if (tmcStep)
  {
    digitalWrite(pinStep, HIGH);
    tmcStep = false;
    }
  else
  {
    digitalWrite(pinStep, LOW);
    tmcStep = true;
  }
}

void StepperControlAxis::setMotorStepWriteTMC2130_2()
{
  if (tmcStep2)
  {
    digitalWrite(pin2Step, HIGH);
    tmcStep2 = false;
  }
  else
  {
    digitalWrite(pin2Step, LOW);
    tmcStep2 = true;
  }
}

void StepperControlAxis::resetMotorStepWriteTMC2130()
{
  // No action needed
}

void StepperControlAxis::resetMotorStepWriteTMC2130_2()
{
  // No action needed
}
#endif
