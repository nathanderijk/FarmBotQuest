#include "StepperControlAxis.h"

StepperControlAxis::StepperControlAxis()
{
    speed = 0;
    direction = true;
}

void StepperControlAxis::loadSettings(unsigned int newSpeed, bool newDirection)
{
    speed = newSpeed;
    direction = newDirection;

}

void StepperControlAxis::enableServo()
{

}

void StepperControlAxis::startServo()
{

}

void StepperControlAxis::stopServo()
{

}

void StepperControlAxis::disableServo()
{

}
