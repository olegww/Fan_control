#ifndef CONFIG_H // Начало условной компиляции. Проверяем, определён ли макрос CONFIG_H.
#define CONFIG_H // Если макрос ещё не определён, то определяем его. Это создаёт защиту от повторного включения файла.
/*
  Примечание:
  Этот механизм называется "include guard" (защита от многократного включения).
  Он предотвращает ошибку множественного определения функций, переменных или других сущностей,
  если файл заголовка случайно будет включён в нескольких местах проекта.
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Adafruit_GFX.h>        // Библиотека графики
#include <Adafruit_SSD1306.h>    // Библиотека для работы с дисплеем SSD1306
#include <GyverEncoder.h>
#include <ESP32Servo.h>
#include "display_logo.h"
#include <esp_system.h> // Для работы с функцией esp_reset_reason()
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

//extern const unsigned char myLogo;

#define SCREEN_WIDTH 128 // Ширина дисплея
#define SCREEN_HEIGHT 64 // Высота дисплея
#define OLED_RESET -1    // Сброс не используется
#define OLED_ADDRESS 0x3C // Адрес I2C

//#define DEBUG_TIME // Включить режим отладки

// Пины для энкодера
#define CLK 33
#define DT 32
#define SW 4

// Глобальные переменные
extern Adafruit_SSD1306 display;
extern AsyncWebServer server;
extern DNSServer dns;
extern AsyncWiFiManager wifiManager;
extern Encoder enc1;
extern Servo esc;
//extern volatile int rotationCount;
extern int pulseWidth;


// NTP настройки
extern const char* ntpServer; // ntp0.ntp-servers.net pool.ntp.org
extern long gmtOffset_sec;  // Часовой пояс (Москва +3)
const int daylightOffset_sec = 0;    // Летнее время (нет)


// Переменные для расчета RPM
extern volatile unsigned long lastPulseTime;
//extern volatile unsigned long pulseInterval;
extern volatile int rpm;

extern int stepSize; // Шаг изменения

extern bool isStepAdjusting; // Флаг изменения шага

//extern volatile bool lastHallState;

extern const int hallSensorPin; // Пины для датчика Холла



#endif // CONFIG_H
