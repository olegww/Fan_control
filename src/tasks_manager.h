#ifndef TASKS_MANAGER_H
#define TASKS_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Дескрипторы задач
extern TaskHandle_t updateDisplayTaskHandle;
extern TaskHandle_t handleEncoderTaskHandle;
void monitorRPMTask(void *pvParameters);

// Объявление функций
void createTasks();
void deleteTasks();

#endif // TASKS_MANAGER_H
