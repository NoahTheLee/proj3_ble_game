#include "ButtonGame.h"
#include <stdlib.h>
#include "GameHelpers.h" // for drawCenteredTextBlock

// ====================== INTERNAL DEFINITIONS ======================
const char *buttonNames[4] = {"Circle", "Square", "Triangle", "Cross"};
const uint16_t buttonColors[4] = {RED, BLUE, GREEN, YELLOW};
const int buttonShapeTypes[4] = {0, 1, 2, 3}; // 0=circle, 1=square, 2=triangle, 3=cross

struct ButtonPuzzle
{
    int sequence[4];  // order of buttons to press
    const char *hint; // hint text for the host to display
};

// Simple shape drawer
void drawShape(int shapeType, int cx, int cy, int size, uint16_t color)
{
    switch (shapeType)
    {
    case 0: // Circle
        M5.Lcd.fillCircle(cx, cy, size / 2, color);
        break;
    case 1: // Square
        M5.Lcd.fillRect(cx - size / 2, cy - size / 2, size, size, color);
        break;
    case 2: // Triangle (pointing up)
        M5.Lcd.fillTriangle(cx, cy - size / 2,
                            cx - size / 2, cy + size / 2,
                            cx + size / 2, cy + size / 2, color);
        break;
    case 3: // Cross
        int thick = size / 5;
        M5.Lcd.fillRect(cx - size / 2, cy - thick / 2, size, thick, color); // horizontal
        M5.Lcd.fillRect(cx - thick / 2, cy - size / 2, thick, size, color); // vertical
        break;
    }
}

void drawAllButtons(int btnX[], int btnY[], int btnSize, bool pressed[])
{
    for (int i = 0; i < 4; i++)
    {
        uint16_t col = pressed[i] ? TFT_DARKGREY : buttonColors[i];
        int cx = btnX[i] + btnSize / 2;
        int cy = btnY[i] + btnSize / 2;

        drawShape(buttonShapeTypes[i], cx, cy, btnSize * 0.75, col);

        // Button border
        M5.Lcd.drawRect(btnX[i], btnY[i], btnSize, btnSize, WHITE);

        // Visual feedback: checkmark
        if (pressed[i])
        {
            int bx = btnX[i];
            int by = btnY[i];
            M5.Lcd.drawLine(bx + 18, by + 35, bx + 30, by + 48, GREEN);
            M5.Lcd.drawLine(bx + 30, by + 48, bx + 52, by + 22, GREEN);
        }
    }
}

// ====================== CLIENT GAME LOGIC ======================
bool doButtonGame(const int sequence[4])
{
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print("Press Sequence");

    int btnSize = 70;
    int gap = 25;
    int startX = (320 - (2 * btnSize + gap)) / 2;
    int startY = 50;

    int btnX[4] = {startX, startX + btnSize + gap, startX, startX + btnSize + gap};
    int btnY[4] = {startY, startY, startY + btnSize + gap, startY + btnSize + gap};

    bool pressed[4] = {false, false, false, false};
    int currentStep = 0;

    drawAllButtons(btnX, btnY, btnSize, pressed);

    while (true)
    {
        M5.update();

        if (M5.Touch.ispressed())
        {
            TouchPoint_t touch = M5.Touch.getPressPoint();
            int tx = touch.x;
            int ty = touch.y;

            for (int i = 0; i < 4; i++)
            {
                if (!pressed[i] &&
                    tx >= btnX[i] && tx <= btnX[i] + btnSize &&
                    ty >= btnY[i] && ty <= btnY[i] + btnSize)
                {
                    // Correct next button in sequence?
                    if (i == sequence[currentStep])
                    {
                        pressed[i] = true;
                        currentStep++;
                        drawAllButtons(btnX, btnY, btnSize, pressed);

                        if (currentStep == 4)
                            return true; // WIN
                    }
                    else
                    {
                        return false; // immediate LOSS on wrong order
                    }

                    delay(200); // debounce + prevent double-press
                    break;
                }
            }
        }
        delay(10);
    }
}

// ====================== LAUNCH (called by both devices) ======================
bool launchButtonGame(boolean isClient)
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    // Define puzzles with explicit hint text
    ButtonPuzzle puzzles[] = {
        {{0, 1, 2, 3}, "Press Circle -> Square -> Triangle -> Cross"},
        {{3, 2, 1, 0}, "Press Cross -> Triangle -> Square -> Circle"},
        {{1, 3, 0, 2}, "Press Square -> Cross -> Circle -> Triangle"},
        {{2, 0, 3, 1}, "Press Triangle -> Circle -> Cross -> Square"},
        {{0, 2, 3, 1}, "Press Circle -> Triangle -> Cross -> Square"},
        {{1, 0, 3, 2}, "Press Square -> Circle -> Cross -> Triangle"}};

    int numPuzzles = sizeof(puzzles) / sizeof(puzzles[0]);
    int selected = rand() % numPuzzles;

    ButtonPuzzle p = puzzles[selected];

    if (!isClient)
    {
        // Host shows hint text (instead of generating the string dynamically)
        drawCenteredTextBlock(p.hint);
        return false;
    }
    else
    {
        // Client plays interactive game
        return doButtonGame(p.sequence);
    }
}