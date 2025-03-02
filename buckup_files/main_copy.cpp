#include "config.h"
#include "display.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "rpm.h"
#include "server_setup.h"
#include "time_manager.h"
#include <WiFi.h>
// #include <WiFiAP.h>
// #include <WiFiClient.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include <atomic>

extern std::atomic<int> activeConnections;

bool localMode = true; // Локальный или сетевой режим

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
    delay(2000);   // Задержка для показа лого
    createTasks(); // Инициализация менеджера задач
    // wifiManager.resetSettings(); // Удаляет сохраненные SSID и пароль из памяти
    // wifiManager.setConnectTimeout(10); // Таймаут в секундах

    // **Добавляем выбор режима перед включением Wi-Fi**
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Press encoder to");
    display.println("start in LOCAL mode");
    display.display();

    unsigned long startTime = millis();
    bool userSelectedLocalMode = false;
    while (millis() - startTime < 30000)
    {
        enc1.tick();
        if (enc1.isClick())
        {
            userSelectedLocalMode = true;
            break;
        }
        delay(10);
    }

    if (userSelectedLocalMode)
    {
        Serial.println("User selected LOCAL MODE");
        localMode = true;
    }
    
    // **Настройка Wi-Fi**
    if (!localMode)
    {
        Serial.println("Attempting WiFi connection...");
        wifiManager.setConnectTimeout(10);
        if (!wifiManager.autoConnect(apSSID, apPassword))
        {
            Serial.println("WiFi connection failed, switching to local mode.");
            localMode = true;
        }
    }

    if (localMode)
    {
        Serial.println("Starting in LOCAL MODE.");
        deleteTasks();
        createTasks();
    }
    else
    {
        Serial.println("WiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }

    // **Запуск сервера и дополнительных функций**
    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    setupServer(server);
    server.begin();
    updateDisplay();
    initRPM();
    Serial.println("Server started!");
}


    //================================================================================
    
    
/*    
    // Перед запуском AP полностью отключаем STA режим Wi-Fi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    WiFi.mode(WIFI_AP);
    delay(100);
    bool apStarted = WiFi.softAP(apSSID, apPassword);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    if (apStarted)
    {
        int y = 0; // Начальная координата по вертикали
        display.setCursor(0, y);
        display.print("AP Name: ");
        display.println(apSSID);
        y += 10;
        display.setCursor(0, y);
        display.print("IP:      ");
        display.println(WiFi.softAPIP());
        y += 10;
        display.setCursor(0, y);
        display.print("pass:    ");
        display.println(apPassword);
        y += 10;
        display.setCursor(0, y);
        display.print("firmware: ");
        display.println("v1.0");
        y += 10;
        display.setCursor(0, y);
        display.print("Please connect and   ");
        display.print(" configure Wi-Fi...");
        y += 10;
        display.display();
    }
    else
    {
        Serial.println("Failed to start Access Point");
    }
    display.display();

    if (!wifiManager.autoConnect(apSSID, apPassword))
        // Таймер ожидания подключения (5 минут)
        wifiTimeout = millis() + 300000; // 5 минут = 300000 мс
    while (millis() < wifiTimeout)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected!");
            break;
        }
        delay(500);
    }

    // Перед стартом спрашиваем пользователя, хочет ли он выбрать локальный режим
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Press encoder to");
    display.println("start in LOCAL mode");
    display.display();

    // Даём 5 секунд на выбор
    unsigned long startTime = millis();
    bool userSelectedLocalMode = false;

    while (millis() - startTime < 5000)
    {
        enc1.tick();
        if (enc1.isClick())
        {
            userSelectedLocalMode = true;
            break;
        }
        delay(10);
    }

    // Если нажали на энкодер, включаем локальный режим
    if (userSelectedLocalMode)
    {
        Serial.println("User selected LOCAL MODE");
        localMode = true;
        deleteTasks();
        createTasks();
    }
    else
    {
        Serial.println("Starting in NETWORK MODE...");
    }

    // Ожидание Wi-Fi перед переключением в локальный режим
    wifiTimeout = millis() + 300000; // 5 минут = 300000 мс
    while (millis() < wifiTimeout)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected!");
            localMode = false;
            break;
        }
        delay(500);
    }

    // Если Wi-Fi не подключился, переключаемся в локальный режим
    if (WiFi.status() != WL_CONNECTED && !localMode)
    {
        Serial.println("No WiFi connection, switching to local mode.");
        localMode = true;
        deleteTasks();
        createTasks();
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
    server.begin();      // Запуск сервера
    updateDisplay();     // Обновляем дисплей
    initRPM();           // Инициализация датчика Холла
    server.begin();      // Запуск сервера
    Serial.println("Server started!");
    // initTime(true); // Пробуем синхронизацию с NTP

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
*/

void loop()
{
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 5000)
    {
        lastMillis = millis();
        Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
        Serial.printf("Heap: %d bytes, Active connections: %d\n", esp_get_free_heap_size(), activeConnections.load());
        Serial.println("Preparing response for /api/data...");
    }
    // vTaskDelay(portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1000));
}