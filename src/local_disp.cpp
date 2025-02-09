#include "local_disp.h"
#include "config.h"
#include <time.h>
#include "time_manager.h"

int menuIndex = 0;
int subMenuIndex = 0;
void updateLocalDisplay()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
   
    if (menuActive) // Если меню активно, отображаем его
    {
        display.setCursor(0, 0);
        display.println("Main Menu");
        display.println(menuIndex == 0 ? "> Settings" : "  Settings");
        display.println(menuIndex == 1 ? "> Fan Mode" : "  Fan Mode");
        display.println(menuIndex == 2 ? "> Reboot" : "  Reboot");
        display.println(menuIndex == 3 ? "> Version" : "  Version");
    }
    else // В остальное время отображается основной экран
    {
        display.setCursor(0, 0);
        display.print("Mode: ");
        display.println(currentMode);

        struct tm timeInfo = getCurrentTime();
        display.setCursor(0, 10);
        display.printf("Time: %02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

        display.setCursor(0, 20);
        display.print("RPM: ");
        display.println(rpm);

        display.setCursor(0, 30);
        display.print("Pulse: ");
        display.println(pulseWidth);
    }

    display.display();
}