#ifndef STEPPERCONTROLENCODER_H_
#define STEPPERCONTROLENCODER_H_

#include "Arduino.h"
#include "pins.h"
#include "Config.h"
#include <stdio.h>
#include <stdlib.h>

struct pinReg
{
    byte *reg;
    byte mask;
};

class StepperControlEncoder
{
public:
    StepperControlEncoder();
    
    void loadPins(
    byte *pinChannelA_reg, byte pinChannelA_mask,
    byte *pinChannelAQ_reg, byte pinChannelAQ_mask,
    byte *pinChannelB_reg, byte pinChannelB_mask,
    byte *pinChannelBQ_reg, byte pinChannelBQ_mask);

    void checkEncoder(void);
    void setPosition(long newPosition);
    long getPosition(void);

private:
    void shiftChannels(void);
    void readPins(void);
    void setChannels(void);
    void processEncoder(void);

    //pin
    struct pinReg pinChannelA;
    struct pinReg pinChannelAQ;
    struct pinReg pinChannelB;
    struct pinReg pinChannelBQ;

    //channels
    bool prvValChannelA;
    bool prvValChannelB;

    bool curValChannelA;
    bool curValChannelB;

    bool readChannelA;
    bool readChannelAQ;
    bool readChannelB;
    bool readChannelBQ;

    // encoder
    long position;
};

#endif /* STEPPERCONTROLENCODER_H_ */
