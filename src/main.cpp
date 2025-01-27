#include "config.h"
#include "display.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "rpm.h"
#include "server_setup.h"
#include "time_manager.h"
#include <WiFi.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>

#include <atomic>
extern std::atomic<int> activeConnections;

// Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Create server&DNS for work WiFiManager
AsyncWebServer server(80);
DNSServer dns;
// Encoder
Encoder enc1(CLK, DT, SW);
// Servo pulse
Servo esc;
// WiFiManager
AsyncWiFiManager wifiManager(&server, &dns);

// Глобальные переменные
int pulseWidth = 1450;        // Ширина импульса (начальное значение)
volatile int rpm = 0;         // Начальное значение оборотов двигателя
int stepSize = 10;            // Шаг изменения
bool isStepAdjusting = false; // Флаг изменения шага
const int hallSensorPin = 35; // Пины для датчика Холла

// Данные для тестирования
String sensorData = "42";
String rotationMode = "Left"; // Лево или Право

void setup()
{
    Serial.begin(115200);
    // Инициализация ESC
    esc.attach(27);                    // Подключите сигнальный провод ESC к пину 27
    esc.writeMicroseconds(1450);       // Инициализация начального значения ширины импульса ESC
    delay(2000);                       // Задержка для инициализации ESC
    esc.writeMicroseconds(pulseWidth); // Установка начальной ширины импульса
    // Инициализация пина датчика Холла как входа
    pinMode(hallSensorPin, INPUT);
    // attachInterrupt(digitalPinToInterrupt(hallSensorPin), hallSensorISR, CHANGE);
    //  Инициализация дисплея
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("SSD1306 allocation failed");
        while (true)
            ; // Остановить выполнение
    }
    display.display();
    delay(2000); // Задержка для показа стартового экрана
    display.clearDisplay();
    // Устанавливаем таймаут подключения к Wi-Fi
    wifiManager.setConnectTimeout(10); // Таймаут в секундах
    // Подключение к Wi-Fi с использованием WiFiManager
    if (!wifiManager.autoConnect("fan_control"))
    {
        Serial.println("Failed to connect to WiFi. Restarting...");
        delay(3000);
        ESP.restart();
    }
    // Если подключение успешно
    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // Монтируем файловую систему SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");
    // Проверяем доступные файлы
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        Serial.print("File: ");
        Serial.println(file.name());
        file = root.openNextFile();
        yield(); // Сбрасываем WDT во время чтения файлов
    }
    setupServer(server); // Инициализация сервера
    updateDisplay();     // Обновляем дисплей
    initRPM();           // Инициализация датчика Холла
    server.begin();      // Запуск сервера
    Serial.println("Server started!");
    // initTime(true); // Пробуем синхронизацию с NTP
    // Serial.println("Time initialization completed.");
    createTasks(); // Инициализация менеджера задач
                   // Определяем причину последней перезагрузки
    esp_reset_reason_t resetReason = esp_reset_reason();
    switch (resetReason)
    {
    case ESP_RST_POWERON:
        Serial.println("Power-on reset (initial power-up)");
        break;
    case ESP_RST_EXT:
        Serial.println("External reset via RTC watchdog or other pin");
        break;
    case ESP_RST_SW:
        Serial.println("Software reset via esp_restart()");
        break;
    case ESP_RST_PANIC:
        Serial.println("Software panic (unhandled exception)");
        break;
    case ESP_RST_INT_WDT:
        Serial.println("Interrupt watchdog reset");
        break;
    case ESP_RST_TASK_WDT:
        Serial.println("Task watchdog reset");
        break;
    case ESP_RST_WDT:
        Serial.println("Other watchdog reset");
        break;
    case ESP_RST_DEEPSLEEP:
        Serial.println("Wakeup from deep sleep");
        break;
    case ESP_RST_BROWNOUT:
        Serial.println("Brownout reset (voltage too low)");
        break;
    case ESP_RST_SDIO:
        Serial.println("Reset over SDIO");
        break;
    default:
        Serial.println("Unknown reset reason");
        break;
    }
}
void loop()
{
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 5000) {
        lastMillis = millis();
        Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
        Serial.printf("Heap: %d bytes, Active connections: %d\n", esp_get_free_heap_size(), activeConnections.load());
        Serial.println("Preparing response for /api/data...");
    }
    // vTaskDelay(portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1000));
}