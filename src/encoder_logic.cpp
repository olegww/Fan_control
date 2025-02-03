#include "menu_logic.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "config.h"

void initEncoder()
{
    // Инициализация энкодера
    enc1.setType(TYPE2);
}

void handleEncoder()
{
    enc1.tick();

    // В локальном режиме энкодер управляет меню
    if (currentMode == LOCAL_MODE)
    {
        handleMenuNavigation();
        return;
    }

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
        // Проверяем, если устройство в режиме AP, переключаемся в локальный режим
        if (WiFi.getMode() == WIFI_AP)
        {
            Serial.println("Encoder clicked in AP mode, switching to local mode...");
            // wifiManager.resetSettings(); // Удаляет сохраненные SSID и пароль из памяти
            //  Остановить сервер перед отключением Wi-Fi
            server.end();
            delay(500);

            // Остановить все задачи перед удалением Wi-Fi
            deleteTasks();

            // Отключить Wi-Fi
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            delay(2000);

            // Переключить устройство в локальный режим
            currentMode = LOCAL_MODE;
            preferences.begin("fan_control", false);
            preferences.putInt("device_mode", LOCAL_MODE);
            preferences.end();
            Serial.println("Switched to LOCAL MODE");

            // Запустить задачи заново
            createTasks();
            return;
        }
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