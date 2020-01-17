#ifndef STEPPERCONTROL_H_
#define STEPPERCONTROL_H_

#include "Arduino.h"
#include "CurrentState.h"
#include "ParameterList.h"
#include "StepperControlAxis.h"
#include "StepperControlEncoder.h"
#include "pins.h"
#include "Config.h"
#include <stdio.h>
#include <stdlib.h>
#include "Command.h"

class StepperControl
{
public:
  StepperControl();

  static StepperControl *getInstance();

  int calibrateAxis(int axis);
  void handleMovementInterrupt();
  void loadSettings();
  int moveToCoords(double xDestScaled, double yDestScaled, double zDestScaled,
                   unsigned int xMaxSpd, unsigned int yMaxSpd, unsigned int zMaxSpd,
                   bool homeX, bool homeY, bool homeZ);
  void reportEncoders();
  void setPositionX(long posX);
  void setPositionY(long posY);
  void setPositionZ(long posZ);
  void storePosition();

private:
  StepperControlAxis axis[3];       //0=x, 2=y, 2=z
  StepperControlEncoder encoder[3];

  //prototypes
  void checkEncoders(void);
  void createEncoders(void);
  void createAxis(void);

  void updatePosition(void);
  void updateParameters(void);
  void resetMovementVariables(void);

  void calculateDestination(double xDestScaled, double yDestScaled, double zDestScaled);
  void calculateSpeed(unsigned int xMaxSpd, unsigned int yMaxSpd, unsigned int zMaxSpd);
  void calculateSpeedDirection(void);
  void setDestinationHome(bool homeX, bool homeY, bool homeZ);
  void calculateActiveAxis(void);

  void loadSettingToStepper(void);
  void enableStepperDriver(void);
  void disableStepperDriver(void);
  void startStepper(void);

  void checkDestinationReached(void);
  void checkStall(void);
  void checkEStop(void);

  //variables
  long position[3] = {0, 0, 0};
  long destination[3] = {0, 0 ,0};
  unsigned int speed[3] = {0, 0, 0};
  bool directionIsUp[3] = {true, true, true};
  bool axisIsActive[3] = {false, false, false}; //axis is (going to be) used when moving
  bool axisIsMoving[3] = {false, false, false}; //axis is moving

  //parameters
  long stepsPerMm[3] = {1, 1, 1};
  long homePosition[3] = {0, 0, 0};
  long maxPosition[3] = {0, 0, 0};
  unsigned int maxSpeed[3] = {0, 0, 0};
  unsigned int homeSpeed[3] = {0, 0, 0};
};

#endif /* STEPPERCONTROL_H_ */