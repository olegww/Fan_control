#include "menu_logic.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "config.h"


void initEncoder()
{
    // Инициализация энкодера
    enc1.setType(TYPE2);
    enc1.setTickMode(AUTO); // Добавить, если нет
    enc1.setFastTimeout(300);

    Serial.println("Encoder initialized");
    Serial.print("menuActive enc_logic = ");
    Serial.println(menuActive);
}

static unsigned long holdStartTime = 0;

static unsigned long menuTimeout = 0;

void handleEncoder()
{
    enc1.tick();

    if (currentMode == LOCAL_MODE)
    {
        if (enc1.isHold()) // Кнопка удерживается
        {
            if (holdStartTime == 0) 
            {
                holdStartTime = millis();
                Serial.print("holdStartTime memory: ");
                Serial.println(holdStartTime);
            }
            unsigned long holdDuration = millis() - holdStartTime;
            if (holdDuration > 2000 && !menuActive) // Если удержание больше 2 секунд
            {
                menuActive = true;
                currentMenu = MAIN_MENU;
                menuTimeout = millis();
                Serial.println("Menu activated!");
            }
        }
        else // Если кнопку отпустили
        {
            holdStartTime = 0; // Сбрасываем таймер
        }

        if (menuActive)
        {
            handleMenuNavigation(menuTimeout);

            if (millis() - menuTimeout > 7000) // Таймаут бездействия
            {
                menuActive = false;
                Serial.println("Menu closed due to inactivity");
            }
        }
        else
        {
            updateLocalDisplay();
        }

        return;
    }
    // Сетевой режим остается без изменений
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
        if (WiFi.getMode() == WIFI_AP)
        {
            Serial.println("Encoder clicked in AP mode, switching to local mode...");
            server.end();
            delay(500);
            deleteTasks();
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            delay(2000);

            currentMode = LOCAL_MODE;
            preferences.begin("fan_control", false);
            preferences.putInt("device_mode", LOCAL_MODE);
            preferences.end();
            Serial.println("Switched to LOCAL MODE");

            createTasks();
            return;
        }
        if (isStepAdjusting)
        {
            isStepAdjusting = false;
        }
        else
        {
            pulseWidth = 1450;
        }
        esc.writeMicroseconds(pulseWidth);
    }

    if (enc1.isHolded())
    {
        isStepAdjusting = true;
    }
}