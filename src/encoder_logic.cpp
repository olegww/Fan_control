#include "encoder_logic.h"
#include "config.h"

void initEncoder()
{
    // Инициализация энкодера
    enc1.setType(TYPE2);
}

void handleEncoder()
{
    enc1.tick();

    if (enc1.isRight())
    {
        if (isStepAdjusting)
        {
            stepSize = constrain(stepSize + 1, 1, 20);
        }
        else
        {
            pulseWidth = constrain(pulseWidth + stepSize, 1000, 2000);
        }
        esc.writeMicroseconds(pulseWidth);
    }

    if (enc1.isLeft())
    {
        if (isStepAdjusting)
        {
            stepSize = constrain(stepSize - 1, 1, 20);
        }
        else
        {
            pulseWidth = constrain(pulseWidth - stepSize, 1000, 2000);
        }
        esc.writeMicroseconds(pulseWidth);
    }

    if (enc1.isClick())
    {
        if (isStepAdjusting)
        {
            isStepAdjusting = false;
        }
        else
        {
            pulseWidth = 1450; // Сброс к стартовому значению
        }
        esc.writeMicroseconds(pulseWidth);
    }

    if (enc1.isHolded())
    {
        isStepAdjusting = true;
    }
}