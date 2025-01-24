#include "display.h"
#include "config.h"

void drawSignalBar()
{
    int signal = WiFi.RSSI();            // Получаем уровень сигнала
    signal = constrain(signal, -100, 0); // Ограничиваем диапазон от -100 до 0

    int totalBars = 10;                                  // Количество "кубиков"
    int barWidth = 7;                                    // Ширина одного "кубика"
    int filledBars = map(signal, -100, 0, 0, totalBars); // Количество закрашенных "кубиков"

    int barHeight = 8;                        // Высота полоски (такой же, как размер текста 1)
    int barY = SCREEN_HEIGHT - barHeight - 2; // Позиция полоски снизу

    // Очищаем область полоски
    display.fillRect(0, barY, SCREEN_WIDTH, barHeight, SSD1306_BLACK);

    // Рисуем полоску
    for (int i = 0; i < totalBars; i++)
    {
        int x = i * barWidth; // Позиция "кубика" по x

        if (i < filledBars)
        {
            // Закрашенный "кубик"
            display.fillRect(x, barY, barWidth - 1, barHeight, SSD1306_WHITE);
        }
        else
        {
            // Пустой "кубик"
            display.drawRect(x, barY, barWidth - 1, barHeight, SSD1306_WHITE);
        }
    }
    // Отображаем текст "RSSI: [значение]" справа от полоски
    int textX = totalBars * barWidth + 4;   // Начало текста после всех кубиков с отступом
    int textY = barY + (barHeight / 2) - 4; // Текст выравнен по центру полоски

    display.setCursor(textX, textY);
    display.print("RSSI:");
    // display.println(WiFi.RSSI());
    display.println(signal); // Показываем значение RSSI
    // display.display();
}
void updateDisplay()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    // Отображаем статус подключения Wi-Fi
    display.setCursor(0, 0);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.println("WiFi: Connected");
        display.print("IP: ");
        display.println(WiFi.localIP());
    }
    else
    {
        display.println("WiFi: Disconnected");
    }
    // Отображаем температуру и влажность
    display.print("Pulse: ");
    display.print(pulseWidth);
    display.println(" W");
    display.print("RPM: ");
    display.print(rpm);
    display.println(" ");
    // Вычисляем аптайм
    unsigned long uptimeMillis = millis();
    unsigned long uptimeSeconds = uptimeMillis / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;

    // Отображаем аптайм
    display.print("Uptime: ");
    display.printf("%02lu:%02lu:%02lu\n", hours, minutes, seconds);
    // Отображаем уровень сигнала
    // display.print("RSSI: ");
    // display.println(WiFi.RSSI());
    drawSignalBar();
    display.display();
}