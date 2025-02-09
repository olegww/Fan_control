#include "tasks_manager.h"
#include "server_setup.h"
#include "time_manager.h"
#include "config.h"
#include "rpm.h"
#include "display.h"
#include "encoder_logic.h"
#include "local_disp.h"
#include <time.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
// #define DEBUG_TIME

extern bool localMode;
//  Дескрипторы задач
TaskHandle_t updateDisplayTaskHandle = NULL;
TaskHandle_t updateLocalDisplayTaskHandle = NULL;
TaskHandle_t handleEncoderTaskHandle = NULL;
TaskHandle_t monitorRPMTaskHandle = NULL;
TaskHandle_t timeManagerTaskHandle = NULL;

// Мьютекс для синхронизации данных
SemaphoreHandle_t mutex;

void timeManagerTask(void *pvParameters)
{
    static unsigned long lastSyncTime = 0; // Последняя синхронизация
    while (true)
    {
        // Обновляем локальное системное время
        updateSystemTime();

#ifdef DEBUG_TIME
        // Вывод текущего времени каждые 10 секунд
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 10000)
        {
            lastDebugTime = millis();
            time_t now = time(nullptr); // Получаем текущее время
            struct tm timeInfo;
            localtime_r(&now, &timeInfo); // Локальное время

            char buffer[20];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

            Serial.print("Updated system time: ");
            Serial.println(buffer);
        }
#endif
        esp_task_wdt_reset(); // Сброс watchdog
        // Задержка между циклами
        vTaskDelay(pdMS_TO_TICKS(1000)); // Обновление раз в секунду
    }
    vTaskDelete(NULL); // Завершаем задачу
}

void monitorRPMTask(void *pvParameters)
{
    for (;;)
    {
        monitorRPM();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void updateDisplayTask(void *pvParameters)
{
    while (true)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            updateDisplay();
            xSemaphoreGive(mutex);
        }
        esp_task_wdt_reset(); 
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void updateLocalDisplayTask(void *pvParameters)
{
    while (true)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            updateLocalDisplay();
            xSemaphoreGive(mutex);
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(menuActive ? 200 : 1000));
        //vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void handleEncoderTask(void *pvParameters)
{
    while (true)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            handleEncoder();
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// Создание задач
void createTasks()
{
    deleteTasks();
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL)
    {
        Serial.println("Failed to create mutex");
        return;
    }

    xTaskCreatePinnedToCore(monitorRPMTask, "MonitorRPMTask", 2048, NULL, 1, &monitorRPMTaskHandle, 0);
    xTaskCreatePinnedToCore(handleEncoderTask, "HandleEncoderTask", 2048, NULL, 1, &handleEncoderTaskHandle, 0);
    Serial.println("monitorRPMTask HandleEncoderTask start");

    if (currentMode == LOCAL_MODE)
    {
        xTaskCreatePinnedToCore(updateLocalDisplayTask, "UpdateLocalDisplayTask", 2048, NULL, 1, &updateLocalDisplayTaskHandle, 0);
        Serial.println("updateLocalDisplayTask started (LOCAL_MODE)");
    }
    else
    {
        Serial.println("Starting display task for NETWORK_MODE or AP Mode");

        xTaskCreatePinnedToCore(updateDisplayTask, "UpdateDisplayTask", 2048, NULL, 1, &updateDisplayTaskHandle, 0);
        Serial.println("updateDisplayTask started");

        if (currentMode == NETWORK_MODE)
        {
            setupServer(server);
            server.begin();
            Serial.println("Server started in NETWORK_MODE");

            xTaskCreatePinnedToCore(timeManagerTask, "TimeManagerTask", 8192, NULL, 1, &timeManagerTaskHandle, 1);
            Serial.println("timeManagerTask started");
        }
    }
}

// Удаление задач
void deleteTasks()
{
    if (updateDisplayTaskHandle != NULL)
    {
        vTaskDelete(updateDisplayTaskHandle);
        updateDisplayTaskHandle = NULL;
    }
    if (timeManagerTaskHandle != NULL)
    {
        vTaskDelete(timeManagerTaskHandle);
        timeManagerTaskHandle = NULL;
    }
    if (updateLocalDisplayTaskHandle != NULL)
    {
        vTaskDelete(updateLocalDisplayTaskHandle);
        updateLocalDisplayTaskHandle = NULL;
    }
    if (handleEncoderTaskHandle != NULL)
    {
        vTaskDelete(handleEncoderTaskHandle);
        handleEncoderTaskHandle = NULL;
    }
    if (monitorRPMTaskHandle != NULL)
    {
        vTaskDelete(monitorRPMTaskHandle);
        monitorRPMTaskHandle = NULL;
    }
    if (mutex != NULL)
    {
        vSemaphoreDelete(mutex);
        mutex = NULL;
    }
}
