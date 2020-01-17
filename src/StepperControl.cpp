#include "StepperControl.h"
#include "Debug.h"
#include "Config.h"

/*
 *  Position[] is in steps. Steps are dependend on the encoder.
 *  Pos in arguments of functions are in mm.
 * 
*/

static StepperControl *instance;

StepperControl *StepperControl::getInstance()
{
    if (!instance)
    {
        instance = new StepperControl();
    };
    return instance;
};

StepperControl::StepperControl()
{
    createEncoders();
    createAxis();
    loadSettings();
}

int StepperControl::calibrateAxis(int axis)
{
}

// code executed by interrupt
void StepperControl::handleMovementInterrupt()
{
    checkEncoders();
}

void StepperControl::loadSettings()
{
    updateParameters();
}

int StepperControl::moveToCoords(double xDestScaled, double yDestScaled, double zDestScaled,
                                 unsigned int xMaxSpd, unsigned int yMaxSpd, unsigned int zMaxSpd,
                                 bool homeX, bool homeY, bool homeZ)
{
    //refresh variables and parameters before calculations
    updateParameters();
    updatePosition();
    resetMovementVariables();

    //calculate destination, speed and direction
    //and store results in class arrays
    if(homeX || homeY || homeZ)
    {
        setDestinationHome(homeX, homeY, homeZ);
        calculateSpeed(homeSpeed[0], homeSpeed[1], homeSpeed[2]);
    }
    else
    {
        calculateDestination(xDestScaled, yDestScaled, zDestScaled);
        calculateSpeed(xMaxSpd, yMaxSpd, zMaxSpd);
    }

    calculateSpeedDirection();
    calculateActiveAxis();

    //load en start motors
    loadSettingToStepper();
    enableStepperDriver();
    startStepper();

    //control movement
    while(axisIsMoving[0] || axisIsMoving[1] || axisIsMoving[2])
    {
        updatePosition();
        checkDestinationReached();
        checkStall();
        checkEStop();
    }

    //End of movement
    //Disable servo's and Report results
    disableStepperDriver();
    updatePosition();

}

// Send the position over uart
void StepperControl::reportEncoders()
{
    char line[100];
    
    updatePosition();

    long posScaled[3] = {(float)position[0] / (float)stepsPerMm[0], (float)position[1] / (float)stepsPerMm[1], (float)position[2] / (float)stepsPerMm[2]};

    //send scaled position in mm
    sprintf(line, " X%ld Y%ld Z%ld", posScaled[0], posScaled[1], posScaled[2]);
    Serial.print(COMM_REPORT_ENCODER_SCALED);
    Serial.print(line);
    CurrentState::getInstance()->printQAndNewLine();

    //send raw position in steps
    sprintf(line, " X%ld Y%ld Z%ld", position[0], position[1], position[2]);
    Serial.print(COMM_REPORT_ENCODER_RAW);
    Serial.print(line);
    CurrentState::getInstance()->printQAndNewLine();
}

// Set the X position to a value in mm, without moving
void StepperControl::setPositionX(long posX)
{
    updateParameters();
    position[0] = posX * stepsPerMm[0];
    encoder[0].setPosition(position[0]);
}

// Set the Y position to a value in mm, without moving
void StepperControl::setPositionY(long posY)
{
    updateParameters();
    position[1] = posY * stepsPerMm[1];
    encoder[1].setPosition(position[1]);
}

// Set the Z position to a value in mm, without moving
void StepperControl::setPositionZ(long posZ)
{
    updateParameters();
    position[2] = posZ * stepsPerMm[2];
    encoder[2].setPosition(position[2]);
}

// Store the position in CurrentState
void StepperControl::storePosition()
{
    updatePosition();

    CurrentState::getInstance()->setX(position[0]);
    CurrentState::getInstance()->setY(position[1]);
    CurrentState::getInstance()->setZ(position[2]);
}

/*
------------------------- Private -------------------------
*/

// update and process the encoder values
void StepperControl::checkEncoders(void)
{
    for (byte i = 0; i < 3; i++)
    {
        encoder[i].checkEncoder();
    }
}

// Create the encoder controller
void StepperControl::createEncoders(void)
{
    for (byte i = 0; i < 3; i++)
    {
        encoder[i] = StepperControlEncoder();
    }

    encoder[0].loadPins(
        ENC_X_A_PORT, ENC_X_A_BYTE,
        ENC_X_A_Q_PORT, ENC_X_A_Q_BYTE,
        ENC_X_B_PORT, ENC_X_B_BYTE,
        ENC_X_B_Q_PORT, ENC_X_B_Q_BYTE);
    encoder[1].loadPins(
        ENC_Y_A_PORT, ENC_Y_A_BYTE,
        ENC_Y_A_Q_PORT, ENC_Y_A_Q_BYTE,
        ENC_Y_B_PORT, ENC_Y_B_BYTE,
        ENC_Y_B_Q_PORT, ENC_Y_B_Q_BYTE);
    encoder[2].loadPins(
        ENC_Z_A_PORT, ENC_Z_A_BYTE,
        ENC_Z_A_Q_PORT, ENC_Z_A_Q_BYTE,
        ENC_Z_B_PORT, ENC_Z_B_BYTE,
        ENC_Z_B_Q_PORT, ENC_Z_B_Q_BYTE);
}

// Create the axis controller
void StepperControl::createAxis(void)
{
    for (byte i = 0; i < 3; i++)
    {
        axis[i] = StepperControlAxis();
    }
}

// get the position from the encoders
void StepperControl::updatePosition()
{
    for (byte i = 0; i < 3; i++)
    {
        position[i] = encoder[i].getPosition();
    }
}

// update variables with ParameterList
void StepperControl::updateParameters()
{
    stepsPerMm[0] = ParameterList::getInstance()->getValue(MOVEMENT_STEP_PER_MM_X);
    stepsPerMm[1] = ParameterList::getInstance()->getValue(MOVEMENT_STEP_PER_MM_Y);
    stepsPerMm[2] = ParameterList::getInstance()->getValue(MOVEMENT_STEP_PER_MM_Z);
}

//reset class variables used when moving
void StepperControl::resetMovementVariables()
{
    for(byte i = 0; i < 3; i++)
    {
        destination[i] = 0;
        speed[i] = 0;
        directionIsUp[i] = true;
        axisIsActive[i] = false;
        axisIsMoving[i] = false;
    }
}

//Check if desitation within range and store in array
void StepperControl::calculateDestination(double xDestScaled, double yDestScaled, double zDestScaled)
{
    long newDest[3];

    newDest[0] = xDestScaled * stepsPerMm[0];
    newDest[1] = yDestScaled * stepsPerMm[1];
    newDest[2] = zDestScaled * stepsPerMm[2];

    for (byte i = 0; i < 3; i++)
    {
        if((newDest[i] >= maxPosition[i] && newDest[i] <= homePosition[i]) || (newDest[i] >= homePosition[i] && newDest[i] <= maxPosition[i]))
        {
            destination[i] = newDest[i];
        }
        else
        {
            destination[i] = position[i];
        }
    }
}

//Check if speed within range and store in array
void StepperControl::calculateSpeed(unsigned int xMaxSpd, unsigned int yMaxSpd, unsigned int zMaxSpd)
{
    unsigned int newSpeed[3] = {xMaxSpd, yMaxSpd, zMaxSpd};

    for (byte i = 0; i < 3; i++)
    {
        if(newSpeed[i] >= 0 && newSpeed[i] <= maxSpeed[i])
        {
            speed[i] = newSpeed[i];
        }
        else
        {
            speed[i] = maxSpeed[i];
        }
    }
}

//calculate whether movent is towards position or not
void StepperControl::calculateSpeedDirection()
{
    for (byte i = 0; i < 3; i++)
    {
        if(destination[i] >= position[i])
        {
            directionIsUp[i] = true;
        }
        else
        {
            directionIsUp[i] = false;
        } 
    }
}

//Set destination home (0) for certain axis
void StepperControl::setDestinationHome(bool homeX, bool homeY, bool homeZ)
{
    bool desitationIsHome[3] = {homeX, homeY, homeZ};

    for (byte i = 0; i < 3; i++)
    {
        if(desitationIsHome[i])
        {
            destination[i] = 0;
        }
        else
        {
            destination[i] = position[i];
        } 
    }
}

//if destination and postion are equal, no movement is needed
void StepperControl::calculateActiveAxis()
{
    for (byte i = 0; i < 3; i++)
    {
        if(destination[i] != position[i])
        {
            axisIsActive[i] = true;
        }
        else
        {
            axisIsActive[i] = false;
        } 
    }
}

//load speed and direction settings in servo class
void StepperControl::loadSettingToStepper()
{
    for (byte i = 0; i < 3; i++)
    {
        if(axisIsActive[i])
        {
            axis[i].loadSettings(speed[i], directionIsUp[i]);
        }
    }
}

//Enable the stepper drivers
void StepperControl::enableStepperDriver()
{
    for (byte i = 0; i < 3; i++)
    {
        if(axisIsActive[i])
        {
            axis[i].enableServo();
        }
    }
}

//Disable the stepper driver
void StepperControl::disableStepperDriver()
{
    for (byte i = 0; i < 3; i++)
    {
        if(axisIsActive[i])
        {
            axis[i].disableServo();
        }
    }
}


//begin movement
void StepperControl::startStepper()
{
    for (byte i = 0; i < 3; i++)
    {
        if(axisIsActive[i])
        {
            axis[i].startServo();
            axisIsMoving[i] = true;
        }
    }    
}

//stop servo is postion is equal to (or over) destination
void StepperControl::checkDestinationReached()
{
    for (byte i = 0; i < 3; i++)
    {
        if(axisIsActive[i] && ((directionIsUp[i] && position[i] >= destination[i]) || (!directionIsUp[i] && position[i] <= destination[i])))
        {
            axis[i].stopServo();
            axisIsMoving[i] = false;
        }
    }  
}

void StepperControl::checkStall()
{

}

void StepperControl::checkEStop()
{

}