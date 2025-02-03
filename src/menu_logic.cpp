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

void handleMenuNavigation()
{
    enc1.tick();

    if (enc1.isRight())
    {
        menuIndex = (menuIndex + 1) % menuItems;
    }

    if (enc1.isLeft())
    {
        menuIndex = (menuIndex - 1 + menuItems) % menuItems;
    }

    if (enc1.isClick())
    {
        Serial.print("Selected menu item: ");
        Serial.println(menuIndex);
        executeMenuAction(menuIndex);
    }

    updateLocalDisplay();
}