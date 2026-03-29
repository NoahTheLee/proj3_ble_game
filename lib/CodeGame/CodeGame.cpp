#include "CodeGame.h"
#include <stdlib.h>
#include "GameHelpers.h"

struct CodePuzzle
{
    const char *secretCode;
    const char *hostHint;
};

bool launchCodeEntryGame(boolean isClient, int difficulty)
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    CodePuzzle puzzles[] = {
        {"34156", "As their positions: Earth, Mars, Mercury, Jupiter, Saturn"},
        {"345468", "How many letters each: 1, 5, 7, 9, 11, 13"},
        {"0348304", "Corners: Circle, Triangle, Square, Octagon, Triangle, Circle, Square"},
        {"5903", "SQRT(25), 3^2, 0^(3294*2), 27/9"},
        {"82448", "Legs: Spider, Human, Cat, Dog, Spider"},
        {"15", "Which is not prime: 11, 13, 15, 7, 29, 23, 31"},
        // {"9876", "Enter the code: 9876"},
        // {"123", "Enter the code: 123"},
        // {"4829", "Enter the code: 4829"},
        // {"9876", "Enter the code: 9876"},
        // {"11223", "Enter the code: 11223"},
        // {"55555", "Enter the code: 55555"},
        // {"0", "Enter the code: 0"}
    };

    int numPuzzles = sizeof(puzzles) / sizeof(puzzles[0]);
    int selected = rand() % numPuzzles;
    CodePuzzle p = puzzles[selected];

    if (!isClient)
    {
        drawCenteredTextBlock(p.hostHint);
        return false;
    }
    else
    {
        return doCodeEntryGame(p.secretCode);
    }
}

void redrawEntered(String entered)
{
    M5.Lcd.fillRect(10, 40, 300, 35, BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setCursor(40, 48);
    M5.Lcd.print(entered);
    if (entered.length() < 8)
        M5.Lcd.print("_"); // visual cursor
};

// ====================== CLIENT TOUCHPAD ======================
bool doCodeEntryGame(const char *secretCode)
{
    String entered = "";
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(30, 10);
    M5.Lcd.print("Enter the Code");

    redrawEntered(entered);

    // Shrunk keypad to fix stretched layout on Core2 (240 px height)
    // New sizes centered nicely, total keypad height now fits perfectly under the header
    const int btnW = 46;
    const int btnH = 34;
    const int gap = 7;
    const int kx = 84; // horizontally centered
    const int ky = 82;

    // Draw keypad 1-9
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            int num = r * 3 + c + 1;
            int x = kx + c * (btnW + gap);
            int y = ky + r * (btnH + gap);

            M5.Lcd.fillRect(x, y, btnW, btnH, 0x1A7F);
            M5.Lcd.setTextColor(WHITE);
            M5.Lcd.setTextSize(3);
            M5.Lcd.setCursor(x + 14, y + 7); // centered digit for new button size
            M5.Lcd.print(num);
        }
    }

    int bottomY = ky + 3 * (btnH + gap);

    // === DELETE ===
    int delX = kx;
    M5.Lcd.fillRect(delX, bottomY, btnW, btnH, RED);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(delX + 14, bottomY + 9); // centered symbol
    M5.Lcd.print("BS");

    // === 0 ===
    int zeroX = kx + (btnW + gap);
    M5.Lcd.fillRect(zeroX, bottomY, btnW, btnH, 0x1A7F);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(zeroX + 15, bottomY + 7); // centered 0
    M5.Lcd.print("0");

    // === ENTER (now ⏎) ===
    int enterX = kx + 2 * (btnW + gap);
    M5.Lcd.fillRect(enterX, bottomY, btnW, btnH, GREEN);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(enterX + 18, bottomY + 9); // centered symbol
    M5.Lcd.print("OK");

    while (true)
    {
        M5.update();
        if (!M5.Touch.ispressed())
            continue;

        TouchPoint_t touch = M5.Touch.getPressPoint();
        int tx = touch.x;
        int ty = touch.y;

        bool handled = false;

        // === DIGITS 1-9 ===
        for (int r = 0; r < 3 && !handled; r++)
        {
            for (int c = 0; c < 3 && !handled; c++)
            {
                int num = r * 3 + c + 1;
                int x = kx + c * (btnW + gap);
                int y = ky + r * (btnH + gap);

                if (tx >= x && tx <= x + btnW && ty >= y && ty <= y + btnH)
                {
                    if (entered.length() < 8)
                    {
                        entered += String(num);
                        redrawEntered(entered);
                    }
                    handled = true;
                }
            }
        }

        // === 0 ===
        if (!handled && tx >= zeroX && tx <= zeroX + btnW && ty >= bottomY && ty <= bottomY + btnH)
        {
            if (entered.length() < 8)
            {
                entered += "0";
                redrawEntered(entered);
            }
            handled = true;
        }

        // === DELETE ===
        if (!handled && tx >= delX && tx <= delX + btnW && ty >= bottomY && ty <= bottomY + btnH)
        {
            if (entered.length() > 0)
            {
                entered = entered.substring(0, entered.length() - 1);
                redrawEntered(entered);
            }
            handled = true;
        }

        // === ENTER ===
        if (!handled && tx >= enterX && tx <= enterX + btnW && ty >= bottomY && ty <= bottomY + btnH)
        {
            return (entered == secretCode);
        }

        if (handled)
            delay(180); // debounce
        delay(10);
    }
}
