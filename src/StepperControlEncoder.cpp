#include "StepperControlEncoder.h"

/* Check the encoder channels for movement according to this specification
                    ________            ________
Channel A          /        \          /        \
             _____/          \________/          \________
                         ________            ________
Channel B               /        \          /        \
             __________/          \________/          \____
                                   __
Channel I                         /  \
             ____________________/    \___________________

rotation ----------------------------------------------------->

*/

StepperControlEncoder::StepperControlEncoder()
{
    //set all variables to default values
    pinChannelA.reg = NULL;
    pinChannelAQ.reg = NULL;
    pinChannelB.reg = NULL;
    pinChannelBQ.reg = NULL;

    pinChannelA.mask = 0;
    pinChannelAQ.mask = 0;
    pinChannelB.mask = 0;
    pinChannelBQ.mask = 0;

    prvValChannelA = false;
    prvValChannelB = false;

    curValChannelA = false;
    curValChannelB = false;

    readChannelA = false;
    readChannelAQ = false;
    readChannelB = false;
    readChannelBQ = false;

    long postion = 0;
}

//set the pins for the encoder
void StepperControlEncoder::loadPins(
    byte *pinChannelA_reg, byte pinChannelA_mask,
    byte *pinChannelAQ_reg, byte pinChannelAQ_mask,
    byte *pinChannelB_reg, byte pinChannelB_mask,
    byte *pinChannelBQ_reg, byte pinChannelBQ_mask)
{
    pinChannelA.reg = pinChannelA_reg;
    pinChannelAQ.reg = pinChannelAQ_reg;
    pinChannelB.reg = pinChannelB_reg;
    pinChannelBQ.reg = pinChannelBQ_reg;

    pinChannelA.mask = pinChannelA_mask;
    pinChannelAQ.mask = pinChannelAQ_mask;
    pinChannelB.mask = pinChannelB_mask;
    pinChannelBQ.mask = pinChannelBQ_mask;
}

// Read encoder pulses and update encoder postion
void StepperControlEncoder::checkEncoder(void)
{
    shiftChannels();
    readPins();
    setChannels();
    processEncoder();
}

// set the encoder postion to a value in steps
void StepperControlEncoder::setPosition(long newPosition)
{
    position = newPosition;
}

// get the position of the encoder in steps
long StepperControlEncoder::getPosition(void)
{
    return position;
}

/*
------------------------- Private -------------------------
*/

// Save current values to previous values
void StepperControlEncoder::shiftChannels(void)
{
    prvValChannelA = curValChannelA;
    prvValChannelB = curValChannelB;
}

//read pin registers
void StepperControlEncoder::readPins(void)
{
    readChannelA = *pinChannelA.reg & pinChannelA.mask;
    readChannelAQ = *pinChannelAQ.reg & pinChannelAQ.mask;
    readChannelB = *pinChannelB.reg & pinChannelB.mask;
    readChannelBQ = *pinChannelBQ.reg & pinChannelBQ.mask;
}

//decide value of chanels, for defferential encoder
void StepperControlEncoder::setChannels(void)
{
    if ((readChannelA ^ readChannelAQ) && (readChannelB ^ readChannelBQ))
    {
        curValChannelA = readChannelA;
        curValChannelB = readChannelB;
    }
}

// Detect edges on the A channel when the B channel is high
void StepperControlEncoder::processEncoder(void)
{
    if (curValChannelB == true && prvValChannelA == false && curValChannelA == true)
    {
        position--;
    }

    if (curValChannelB == true && prvValChannelA == true && curValChannelA == false)
    {
        position++;
    }
}
