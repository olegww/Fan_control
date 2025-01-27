#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <time.h>

// Инициализация времени
void initTime(bool syncWithNTP = true);

// Получение текущего времени в формате struct tm
struct tm getCurrentTime();

// Проверка, есть ли синхронизация времени с интернетом
bool isTimeSynced();

// Функция для обновления системного времени на основе таймеров
void updateSystemTime();

#endif
