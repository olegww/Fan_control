#include "server_setup.h"
#include "time_manager.h"
#include "config.h"
#include <time.h>
#include <SPIFFS.h>

#include <atomic>
#include <mutex>

std::atomic<int> activeConnections{0}; // Счетчик активных подключений
std::mutex connectionsMutex;           // Мьютекс для потокобезопасности

extern String rotationMode;
void setupServer(AsyncWebServer &server)
{
    // Раздача статических файлов
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // API для получения данных
    server.on("/api/data", HTTP_GET, [&](AsyncWebServerRequest *request)
              {
    request->onDisconnect([]() {
        Serial.println("Connection closed");
        Serial.printf("Active connections: %d\n", activeConnections.load());
    });
            // Увеличиваем счетчик при новом подключении
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        activeConnections++;
    }

    // Обработка запроса
    if (activeConnections > 8) {
         vTaskDelay(pdMS_TO_TICKS(50)); // Задержка
         Serial.println("Preparing response for /api/data...");
        request->send(503, "application/json", "{\"error\":\"Server busy\"}");
        // Уменьшаем счетчик при отказе
        Serial.printf("Active connections: %d\n", activeConnections.load());
        {
            std::lock_guard<std::mutex> lock(connectionsMutex);
            activeConnections--;
        }
        return;
    }

    server.on("/api/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Preparing response for /api/data...");
    request->send(200, "text/plain", "pong");
    });

    // Уменьшаем счетчик при отключении клиента
    request->onDisconnect([]() {
        Serial.println("Connection closed");
        std::lock_guard<std::mutex> lock(connectionsMutex);
        activeConnections--;
        Serial.printf("Active connections: %d\n", activeConnections.load());
    });
        if (pulseWidth > 1450) {
            rotationMode = "Left";
        } else if (pulseWidth < 1450) {
            rotationMode = "Right";
        } else {
            rotationMode = "Stopped"; // При pulseWidth = 1450
        }
        char response[128];
        snprintf(response, sizeof(response), "{\"value\":\"%d\", \"rotation\":\"%s\", \"pulseWidth\":\"%d\"}", rpm, rotationMode.c_str(), pulseWidth);
        Serial.println("Preparing response for /api/data...");
        request->send(200, "application/json", response); });

#ifdef DEBUG_TIME
    // API для получения текущего времени
    server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year > (1970 - 1900)) {
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        String json = "{\"time\":\"" + String(timeStr) + "\"}";
        request->send(200, "application/json", json);
    } else {
        request->send(503, "application/json", "{\"error\":\"Time not synchronized\"}");
    } });
#endif
    server.on("/api/sync-time", HTTP_POST, [](AsyncWebServerRequest *request)
              {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.printf("Active connections: %d\n", activeConnections.load());
    Serial.println("Preparing response for /api/data...");
    request->send(200, "application/json", "{\"status\":\"success\"}");
    Serial.println("Time synchronized via button"); });

    // API для изменения и получения настроек
    server.on("/api/settings", HTTP_ANY, [](AsyncWebServerRequest *request)
              {
    if (request->method() == HTTP_GET) {
        // Возвращаем текущую тайм-зону
        String response = "{\"timezone\":" + String(gmtOffset_sec / 3600) + "}";
        Serial.println("Preparing response for /api/data...");
        request->send(200, "application/json", response);
        Serial.printf("Current time zone sent: GMT %d hours\n", gmtOffset_sec / 3600);
    } else if (request->method() == HTTP_POST) {
        // Изменяем тайм-зону
        if (request->hasParam("timezone", true)) {
            String timezone = request->getParam("timezone", true)->value();
            gmtOffset_sec = timezone.toInt() * 3600;
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
            request->send(200, "application/json", "{\"status\":\"success\"}");
            Serial.printf("Time zone updated: GMT %d hours\n", timezone.toInt());
        } else {
            Serial.println("Preparing response for /api/data...");
            request->send(400, "application/json", "{\"error\":\"Missing timezone parameter\"}");
        }
    } else {
        Serial.println("Preparing response for /api/data...");
        request->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    } });
    /*
        server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
        time_t now = time(nullptr);
        struct tm timeInfo;
        localtime_r(&now, &timeInfo);

        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

        String jsonResponse = "{\"time\":\"" + String(buffer) + "\"}";
        request->send(200, "application/json", jsonResponse); });
    */
    server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    request->onDisconnect([]() {
    Serial.println("Connection closed");
    });
    time_t now = time(nullptr);
    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeInfo);
    Serial.println("Preparing response for /api/data...");
    request->send(200, "application/json", "{\"time\":\"" + String(buffer) + "\"}"); });
    // API для получения команд
    server.on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        if (request->hasParam("value", true)) {
            String value = request->getParam("value", true)->value();
            Serial.println("Command received: " + value);
            String response = "{\"status\":\"ok\"}";
            Serial.println("Preparing response for /api/data...");
            request->send(200, "application/json", response);

            // Обрабатываем команды
            if (value == "MODE") {
                rotationMode = (rotationMode == "Left") ? "Right" : "Left";
                Serial.println("Rotation mode changed to: " + rotationMode);
            } else if (value == "ON") {
                Serial.println("Turning ON...");
            } else if (value == "OFF") {
                Serial.println("Turning OFF...");
            } else if (value == "AUTO") {
                Serial.println("Auto mode enabled.");
            } else if (value == "STOP") {
                pulseWidth = 1450; // Сброс в начальное значение
                esc.writeMicroseconds(pulseWidth);
                Serial.println("Fan stopped.");
            } else if (value == "SPEED_UP") {
                pulseWidth += 20; // Увеличиваем на 10
                esc.writeMicroseconds(pulseWidth);
                Serial.println("Increasing speed...");
            } else if (value == "SPEED_DOWN") {
                pulseWidth -= 20; // Уменьшаем на 10
                esc.writeMicroseconds(pulseWidth);
                Serial.println("Decreasing speed...");
            }
            if (pulseWidth < 1000) pulseWidth = 1000; // Минимум
            if (pulseWidth > 2000) pulseWidth = 2000; // Максимум
            // Обновляем rotationMode после изменения pulseWidth
            if (pulseWidth > 1450) {
                rotationMode = "Left";
            } else if (pulseWidth < 1450) {
                rotationMode = "Right";
            } else {
                rotationMode = "Stopped"; // При pulseWidth = 1450
            }
        } else {
            Serial.println("Preparing response for /api/data...");
            request->send(400, "application/json", "{\"error\":\"Missing value\"}");
        } });

    // Запуск сервера
    Serial.println("Server initialized!");
}
