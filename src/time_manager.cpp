#include "config.h"
#include "time_manager.h"
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>

// Глобальные переменные
static time_t systemTime = 0; // Храним системное время
static unsigned long lastMillis = 0; // Время последнего обновления (millis)
static bool timeSynced = false;

// NTP настройки
const char* ntpServer = "ntp0.ntp-servers.net"; // ntp0.ntp-servers.net pool.ntp.org
long gmtOffset_sec = 3* 3600;  // Часовой пояс (Москва +3)
extern const int daylightOffset_sec;    // Летнее время (нет)

// Инициализация времени
void initTime(bool useNTP) {
    if (useNTP) {
        configTime(gmtOffset_sec, daylightOffset_sec, "ntp0.ntp-servers.net", "ntp2.ntp-servers.net", "ntp3.ntp-servers.net"); // Установка NTP серверов и часового пояса
        Serial.println("NTP servers configured. Attempting to synchronize...");
    } else {
        Serial.println("NTP synchronization skipped.");
    }

    // Проверка успешного получения времени
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println("Time synchronized:");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

void updateSystemTime() {
    static time_t lastSyncTime = 0;
    time_t now = time(nullptr);
    if (now - lastSyncTime > 3600) { // Синхронизация раз в час
        lastSyncTime = now;
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("Time synchronized.");
    }
}
/*
void updateSystemTime() {
    time_t now = time(nullptr);
    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
}
*/

// Получение текущего времени
struct tm getCurrentTime() {
    updateSystemTime();
    time_t rawTime = systemTime + ((millis() - lastMillis) / 1000);
    struct tm timeInfo;
    localtime_r(&rawTime, &timeInfo);
    return timeInfo;
}

// Проверка статуса синхронизации
bool isTimeSynced() {
    return timeSynced;
}
