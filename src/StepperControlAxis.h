#ifndef STEPPERCONTROLAXIS_H_
#define STEPPERCONTROLAXIS_H_

#include "Arduino.h"
#include "CurrentState.h"
#include "ParameterList.h"
#include "pins.h"
#include "StepperControlEncoder.h"
#include "Config.h"
#include <stdio.h>
#include <stdlib.h>

class StepperControlAxis
{
public:
  StepperControlAxis();

  void loadSettings(unsigned int speed, bool direction);
  void enableServo(void);
  void startServo(void);
  void stopServo(void);
  void disableServo(void);

private:
  unsigned int speed;
  bool direction;
};

#endif /* STEPPERCONTROLAXIS_H_ */
