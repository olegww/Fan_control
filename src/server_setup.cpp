#include "server_setup.h"
#include "config.h" // Для переменных rpm, rotationMode, pulseWidth, и esc
#include <SPIFFS.h>

extern String rotationMode;

void setupServer(AsyncWebServer &server) {
    // Раздача статических файлов
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // API для получения данных
    server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        String response = "{\"value\":\"" + String(rpm) + "\", \"rotation\":\"" + rotationMode + "\", \"pulseWidth\":\"" + String(pulseWidth) + "\"}";
        request->send(200, "application/json", response);
    });

    // API для получения команд
    server.on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value", true)) {
            String value = request->getParam("value", true)->value();
            Serial.println("Command received: " + value);
            String response = "{\"status\":\"ok\"}";
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
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing value\"}");
        }
    });

    // Запуск сервера
    Serial.println("Server initialized!");
}
