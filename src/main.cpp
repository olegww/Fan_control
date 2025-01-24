#include "config.h"
#include "display.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "rpm.h"
#include "server_setup.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>

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
int pulseWidth = 1450;              // Ширина импульса (начальное значение)
volatile int rpm = 0;               // Начальное значение оборотов двигателя
int stepSize = 10;                  // Шаг изменения
bool isStepAdjusting = false;       // Флаг изменения шага
const int hallSensorPin = 35;       // Пины для датчика Холла

// Данные для тестирования
//volatile int rotationCount = 0;
//volatile unsigned long lastPulseTime = 0;  // Время последнего импульса
//volatile unsigned long pulseInterval = 0; // Интервал между импульсами
//volatile bool lastHallState = HIGH; // Флаг состояния датчика Холла
String sensorData = "42";
String rotationMode = "Left"; // Лево или Право

void setup()
{
    Serial.begin(115200);
    // Инициализация ESC
    esc.attach(14);                    // Подключите сигнальный провод ESC к пину 14
    esc.writeMicroseconds(1450);       // Инициализация начального значения ширины импульса ESC
    delay(2000);                       // Задержка для инициализации ESC
    esc.writeMicroseconds(pulseWidth); // Установка начальной ширины импульса
    // Инициализация пина датчика Холла как входа
    pinMode(hallSensorPin, INPUT);
    //attachInterrupt(digitalPinToInterrupt(hallSensorPin), hallSensorISR, CHANGE);
    // Инициализация дисплея
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
    setupServer(server);
    // Запуск сервера
    updateDisplay();
    initRPM(); // Инициализация датчика Холла
    server.begin();
    Serial.println("Server started!");
    createTasks();
}
void loop()
{
    vTaskDelay(portMAX_DELAY);
    //vTaskDelay(100 / portTICK_PERIOD_MS);
}