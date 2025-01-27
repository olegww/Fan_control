#include "tasks_manager.h"
#include "time_manager.h"
#include "rpm.h"
#include "display.h"
#include "encoder_logic.h"
#include <time.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
// #define DEBUG_TIME
//  Дескрипторы задач
TaskHandle_t updateDisplayTaskHandle = NULL;
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
        vTaskDelay(pdMS_TO_TICKS(10000)); // Обновление раз в секунду
    }
    vTaskDelete(NULL); // Завершаем задачу
}

// Задача мониторинга RPM
void monitorRPMTask(void *pvParameters)
{
    for (;;)
    {
        monitorRPM();                  // Мониторинг RPM
        vTaskDelay(pdMS_TO_TICKS(100)); // Задержка 10 мс
    }
}

// Задача для обновления дисплея
void updateDisplayTask(void *pvParameters)
{
    while (true)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            updateDisplay(); // Обновление дисплея
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Обновление раз в секунду
    }
}

// Задача для обработки энкодера
void handleEncoderTask(void *pvParameters)
{
    while (true)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            handleEncoder(); // Обработка энкодера
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2)); // Частота 2 мс
    }
}

// Создание задач
void createTasks()
{
    // Инициализация мьютекса
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL)
    {
        Serial.println("Failed to create mutex");
        return;
    }
    // мониторинг RPM
    xTaskCreatePinnedToCore(
        monitorRPMTask,
        "MonitorRPMTask",
        2048,
        NULL,
        1,
        &monitorRPMTaskHandle,
        0);

    // обновление дисплея
    xTaskCreatePinnedToCore(
        updateDisplayTask,
        "UpdateDisplayTask",
        2048,
        NULL,
        1,
        &updateDisplayTaskHandle,
        0 // Закрепляем за ядром 0
    );

    // обработка энкодера
    xTaskCreatePinnedToCore(
        handleEncoderTask,
        "HandleEncoderTask",
        2048,
        NULL,
        1,
        &handleEncoderTaskHandle,
        0 // Закрепляем за ядром 1
    );
    // управление временем
    xTaskCreatePinnedToCore(
        timeManagerTask,      // Функция задачи
        "TimeManagerTask",    // Имя задачи
        8192,                 // Размер стека
        NULL,                 // Параметры задачи
        tskIDLE_PRIORITY + 1, // Низкий приоритет
        &updateDisplayTaskHandle,
        1 // Ядро для выполнения задачи
    );
}

// Удаление задач
void deleteTasks()
{
    if (updateDisplayTaskHandle != NULL)
    {
        vTaskDelete(updateDisplayTaskHandle);
        updateDisplayTaskHandle = NULL;
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
