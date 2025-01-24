#include "rpm.h"
#include "config.h"
#include <Arduino.h>



// Константы
//const int hallSensorPin = 35; // Пин датчика Холла
const unsigned long debounceInterval = 6; // Минимальное время между состояниями (мс)
const unsigned long stopTimeout = 2000;   // Время без изменений до обнуления RPM (мс)

// Глобальные переменные
//volatile int rpm = 0;                      // Текущее значение RPM
volatile unsigned long pulseDurations[10]; // Буфер времени между изменениями
volatile int pulseIndex = 0;               // Индекс буфера
volatile unsigned long lastPulseTime = 0;  // Время последнего изменения состояния

// Прерывание на изменение состояния
void IRAM_ATTR hallSensorInterrupt() {
    unsigned long now = micros(); // Текущее время в микросекундах
    if ((now - lastPulseTime) > debounceInterval * 1000) { // Проверка на дребезг
        unsigned long duration = now - lastPulseTime; // Время между изменениями
        lastPulseTime = now; // Обновляем время последнего изменения

        // Сохраняем длительность в буфер
        pulseDurations[pulseIndex] = duration;
        pulseIndex = (pulseIndex + 1) % 10; // Кольцевой буфер
    }
}

// Инициализация датчика RPM
void initRPM() {
    pinMode(hallSensorPin, INPUT_PULLUP); // Настраиваем пин как вход
    attachInterrupt(digitalPinToInterrupt(hallSensorPin), hallSensorInterrupt, CHANGE); // Прерывание на изменение состояния
}

// Мониторинг RPM
void monitorRPM() {
    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();

    // Если нет импульсов в течение stopTimeout — RPM обнуляется
    if (currentTime - (lastPulseTime / 1000) > stopTimeout) {
        rpm = 0;
        return;
    }

    // Считаем среднее время между импульсами
    unsigned long sumDurations = 0;
    int validPulses = 0;

    for (int i = 0; i < 10; i++) {
        if (pulseDurations[i] > 0) {
            sumDurations += pulseDurations[i];
            validPulses++;
        }
    }

    if (validPulses > 0) {
        unsigned long averageDuration = sumDurations / validPulses; // Средняя длительность
        rpm = 60000000 / (averageDuration * 2); // Вычисление RPM
    }

    // Отладочный вывод RPM каждые 100 мс
    if (currentTime - lastUpdateTime >= 100) {
        Serial.print("RPM: ");
        Serial.println(rpm);
        lastUpdateTime = currentTime;
    }
}
