#include "local_disp.h"
#include "config.h"
#include "server_setup.h"
#include "encoder_logic.h"
#include "tasks_manager.h"
#include "esp_wifi.h"

const int menuItems = 4; // Количество пунктов меню
MenuState currentMenu = MAIN_MENU;
extern AsyncWebServer server;


void executeMenuAction(int index)
{
    switch (currentMenu)
    {
    case MAIN_MENU:
        switch (index)
        {
        case 0:
            Serial.println("Entering settings...");
            currentMenu = SETTINGS;
            break;
        case 1:
            Serial.println("Adjusting fan mode...");
            currentMenu = FAN_MODE;
            break;
        case 2:
            Serial.println("Rebooting device...");
            wifiManager.resetSettings();
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 20);
            display.println("Wait reboot...");
            delay(2000);
            ESP.restart();
            break;
        case 3:
            Serial.println("version info...");
            break;
        }
        break;
    }
}

void handleMenuNavigation(unsigned long &menuTimeout)
{
    static unsigned long lastMoveTime = 0; // Время последнего движения
    static int lastDirection = 0;          // Последнее направление (1 = вправо, -1 = влево)

    enc1.tick();
    
    int currentDirection = 0; // Направление движения

    if (enc1.isRight()) currentDirection = 1;
    if (enc1.isLeft()) currentDirection = -1;

    unsigned long now = millis(); // Текущее время

    if (currentDirection != 0)
    {
        if (currentDirection == lastDirection && now - lastMoveTime < 200)
        {
            return;
        }

        menuIndex = (menuIndex + currentDirection + menuItems) % menuItems;
        menuTimeout = now;
        lastMoveTime = now;
        lastDirection = currentDirection;

        Serial.print("Moved ");
        Serial.println(currentDirection > 0 ? "Right" : "Left");
    }

    if (enc1.isClick()) // Проверяем нажатие кнопки
    {
        Serial.print("Selected menu item: ");
        Serial.println(menuIndex);
        executeMenuAction(menuIndex);
        menuTimeout = millis();
    }

    updateLocalDisplay();
}