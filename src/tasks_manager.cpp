#include "tasks_manager.h"
#include "rpm.h"
#include "display.h"
#include "encoder_logic.h"
#include <esp_task_wdt.h>

// Дескрипторы задач
TaskHandle_t updateDisplayTaskHandle = NULL;
TaskHandle_t handleEncoderTaskHandle = NULL;
TaskHandle_t monitorRPMTaskHandle = NULL;

// Мьютекс для синхронизации данных
SemaphoreHandle_t mutex;


// Задача мониторинга RPM
void monitorRPMTask(void *pvParameters) {
    for (;;) {
        monitorRPM(); // Мониторинг RPM
        vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 10 мс
    }
}

/*
// Задача для мониторинга RPM
void monitorRPMTask(void *pvParameters) {
    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            monitorRPM(); // Вызываем функцию мониторинга RPM
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 10 мс для высокой частоты опроса
    }
}
*/

// Задача для обновления дисплея
void updateDisplayTask(void *pvParameters) {
    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            updateDisplay(); // Обновление дисплея
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Обновление раз в секунду
    }
}

// Задача для обработки энкодера
void handleEncoderTask(void *pvParameters) {
    while (true) {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
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
        // Создаем задачу для мониторинга RPM
    xTaskCreatePinnedToCore(
        monitorRPMTask,
        "MonitorRPMTask",
        2048,
        NULL,
        1,
        &monitorRPMTaskHandle,
        1
    );

    // Создаем задачу для обновления дисплея
    xTaskCreatePinnedToCore(
        updateDisplayTask,
        "UpdateDisplayTask",
        2048,
        NULL,
        1,
        &updateDisplayTaskHandle,
        0 // Закрепляем за ядром 0
    );

    // Создаем задачу для обработки энкодера
    xTaskCreatePinnedToCore(
        handleEncoderTask,
        "HandleEncoderTask",
        2048,
        NULL,
        1,
        &handleEncoderTaskHandle,
        1 // Закрепляем за ядром 1
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
