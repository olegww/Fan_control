#include "local_disp.h"
#include "config.h"
#include <time.h>
#include "time_manager.h"

int menuIndex = 0;
int subMenuIndex = 0;

void updateLocalDisplay()
{
    //Serial.println("updateLocalDisplay() start");
    display.clearDisplay();
    display.setCursor(0, 20);
        switch (currentMenu) {
    case MAIN_MENU:
        display.println("Main Menu");
        display.println(menuIndex == 0 ? "> Settings" : "  Settings");
        display.println(menuIndex == 1 ? "> Fan Mode" : "  Fan Mode");
        display.println(menuIndex == 2 ? "> Reboot" : "  Reboot");
        display.println(menuIndex == 3 ? "> Version" : "  Version");
        break;
    case SETTINGS:
        display.println("Settings Menu");
        display.println(subMenuIndex == 0 ? "> Set Time" : "  Set Time");
        display.println(subMenuIndex == 1 ? "> Back" : "  Back");
        break;
    }
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    //display.setCursor(0, 0);
    //display.println("Local Mode");

    display.setCursor(0, 0);
    display.print("RPM: ");
    display.println(rpm);

    // Получаем текущее время
    struct tm timeInfo = getCurrentTime();
    display.setCursor(0, 10);
    display.printf("Time: %02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min);

    // Отображение меню или данных
    display.display();
}