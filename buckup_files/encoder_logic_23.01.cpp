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
    // Переменные для подсчета изменений состояния
    static unsigned long lastTime = 0;
    static int changeCount = 0;
    static unsigned long lastHallChangeTime = 0; // Для отслеживания активности датчика Холла
    // Интервал обновления в миллисекундах
    const unsigned long updateInterval = 1000; // 1 секунда
    const unsigned long stopTimeout = 2000;    // Если нет изменений датчика Холла в течение 2 секунд, считаем двигатель остановленным
    // Чтение состояния датчика Холла
    int hallSensorState = digitalRead(hallSensorPin);
    if (millis() - lastHallChangeTime > stopTimeout)
    {
        rpm = 0; // Сброс RPM при отсутствии активности
    }
    const unsigned long debounceDelay = 5; // Минимальный интервал между изменениями
    static unsigned long lastDebounceTime = 0;
    if (millis() - lastDebounceTime > debounceDelay)
    {
        if (hallSensorState != lastHallState)
        {
            changeCount++;
            lastHallChangeTime = millis(); // Обновляем время последнего изменения
            lastHallState = hallSensorState;
        }
        lastDebounceTime = millis();
    }

    // Проверка, прошел ли интервал
    if (millis() - lastTime >= updateInterval)
    {
        // Проверяем, есть ли изменения в датчике за последнее время
        if (millis() - lastHallChangeTime > stopTimeout)
        {
            rpm = 0; // Если двигатель не активен, сбрасываем RPM
        }
        else
        {
            // Вычисляем обороты в минуту (RPM)
            rpm = (changeCount * 60000) / (2 * updateInterval); // 60000 мс в минуте, 2 магнита
        }
        // Сбрасываем счетчики
        changeCount = 0;
        lastTime = millis();
    }
}