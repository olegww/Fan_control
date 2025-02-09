#ifndef CONFIG_H // Conditional compilation starts. Check if we define the CONFIG_H macro.
#define CONFIG_H // If the macro is not defined yet, we define it. This creates protection against re-inclusion of the file.

#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverEncoder.h>
#include <ESP32Servo.h>
#include "display_logo.h"
#include "menu_logic.h"
#include "local_disp.h"
#include <esp_system.h>
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

extern Preferences preferences; // Объект для работы с NVS
//extern const unsigned char myLogo;
//float version_firmware = 1.0;

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
extern bool localMode;
extern bool menuActive;

// Настройки AP
extern const char *apSSID;
extern const char *apPassword;

// Mode
enum DeviceMode {
    UNDEFINED_MODE,
    NETWORK_MODE,
    LOCAL_MODE
};
extern DeviceMode currentMode;


extern int menuIndex;
extern const int menuItems; // Количество пунктов меню
// Определение списка состояний меню
enum MenuState {
    MAIN_MENU,
    SETTINGS,
    TIME_CONFIG,
    FAN_MODE,
    CUSTOM_MODE
};
extern MenuState currentMenu;
extern int subMenuIndex;



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
