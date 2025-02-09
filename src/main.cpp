#include "config.h"
#include "display.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "rpm.h"
#include "server_setup.h"
#include "time_manager.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include <atomic>

extern std::atomic<int> activeConnections;

Preferences preferences;

DeviceMode currentMode = UNDEFINED_MODE; // Начальный режим (не определен)

bool localMode = true; // Локальный или сетевой режим

bool menuActive = false;


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

// Настройки AP
const char *apSSID = "Fan_control";
const char *apPassword = "12344321";
unsigned long wifiTimeout;

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
    Serial.print("menuActive enc_logic = ");
    Serial.println(menuActive);
    esc.attach(27);                    // Подключите сигнальный провод ESC к пину 27
    esc.writeMicroseconds(1450);       // Инициализация начального значения ширины импульса ESC
    delay(2000);                       // Задержка для инициализации ESC
    esc.writeMicroseconds(pulseWidth); // Установка начальной ширины импульса

    // Инициализация пина датчика Холла как входа
    pinMode(hallSensorPin, INPUT);

    //  Инициализация дисплея
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("SSD1306 allocation failed");
        while (true)
            ; // Остановить выполнение
    }
    display.clearDisplay();
    display.drawBitmap(0, 0, myLogo, 128, 64, SSD1306_WHITE); // Рисуем логотип
    display.display();
    delay(2000); // Задержка для показа лого
    //  Читаем сохраненный режим
    preferences.begin("fan_control", false);
    currentMode = static_cast<DeviceMode>(preferences.getInt("device_mode", NETWORK_MODE));
    preferences.end();
    Serial.print("currentMode ");
    Serial.println(currentMode);

    Serial.print("Saved Mode: ");
    Serial.println(currentMode == LOCAL_MODE ? "LOCAL_MODE" : "NETWORK_MODE");

    // Экран выбора режима
    int selectedMode = 0; // 0 - LOCAL_MODE, 1 - NETWORK_MODE
    unsigned long startTime = millis();

    while (millis() - startTime < 10000) // 10 секунд на выбор режима
    {
        enc1.tick();

        if (enc1.isRight())
        {
            selectedMode = 1; // NETWORK_MODE
        }

        if (enc1.isLeft())
        {
            selectedMode = 0; // LOCAL_MODE
        }

        // Выбор режима
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Select Mode:");
        display.println(selectedMode == 0 ? "> LOCAL" : "  LOCAL");
        display.println(selectedMode == 1 ? "> NETWORK" : "  NETWORK");
        display.display();

        if (enc1.isClick())
        {
            currentMode = (selectedMode == 0) ? LOCAL_MODE : NETWORK_MODE;
            break;
        }

        delay(10);
    }

    // Если ничего не выбрано, загружаем последний режим из памяти
    if (millis() - startTime >= 10000)
    {
        Serial.println("No selection, loading saved mode.");
    }
    else
    {
        // Сохраняем новый выбор пользователя
        preferences.begin("fan_control", false);
        preferences.putInt("device_mode", currentMode);
        preferences.end();
    }

    Serial.print("Selected Mode: ");
    Serial.println(currentMode == LOCAL_MODE ? "LOCAL_MODE" : "NETWORK_MODE");

    // Запускаем нужный режим
    if (currentMode == NETWORK_MODE)
    {
        Serial.println("Attempting WiFi connection...");
        wifiManager.setConnectTimeout(10);
        Serial.println("WiFi connection failed, switching to AP Mode.");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPassword);
        delay(100);
        if (esp_wifi_connect() != ESP_OK && currentMode == NETWORK_MODE)
        {
            // AP Mode на дисплее
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);
            display.println("AP Mode Active");
            display.println("AP Name: " + String(apSSID));
            display.print("IP: ");
            display.println(WiFi.softAPIP());
            display.print("Pass: ");
            display.println(apPassword);
            display.display();
        }

        if (wifiManager.autoConnect(apSSID, apPassword))
        {
            Serial.println("WiFi connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());

            if (!SPIFFS.begin(true))
            {
                Serial.println("Failed to mount SPIFFS");
                return;
            }

            Serial.println("SPIFFS mounted successfully");

            setupServer(server);
            server.begin();
            Serial.println("Server started!");
        }
        else
        {
            Serial.println("WiFi connection failed, switching to local mode.");
            currentMode = LOCAL_MODE;
        }
    }
    createTasks();
    initRPM();
}

void loop()
{
    if (currentMode == NETWORK_MODE)
    {
        static unsigned long lastMillis = 0;
        if (millis() - lastMillis > 5000)
        {
            lastMillis = millis();
            Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
            Serial.printf("Heap: %d bytes, Active connections: %d\n", esp_get_free_heap_size(), activeConnections.load());
            Serial.println("Preparing response for /api/data...");
        }
    }
    // vTaskDelay(portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1000));
}