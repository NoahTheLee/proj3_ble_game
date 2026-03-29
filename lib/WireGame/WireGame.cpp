#include "WireGame.h"
#include <stdlib.h>
#include "BLEHandler.h"
#include "GameHelpers.h"

struct WirePuzzle // Simple struct to extract puzzle setup
{
    uint16_t colors[10];
    int count;
    int correct;
    const char *hint;
};

bool launchWireGame(boolean isClient, int difficulty)
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    WirePuzzle puzzles[] = {
        // TODO: Update hints to be puzzles or something
        // {{RED, BLUE, GREEN}, 3, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789"},
        // {{RED, BLUE, GREEN, WHITE, ORANGE, PURPLE}, 6, 1, "Cut the BLUE wire"},
        // {{RED, BLUE, GREEN}, 3, 1, "Cut the BLUE wire"},
        // {{YELLOW, RED, RED, WHITE}, 4, 3, "Cut the LAST wire"},
        // {{GREEN, BLUE, GREEN, RED}, 4, 0, "Cut the FIRST wire"},
        // {{WHITE, YELLOW, BLUE}, 3, 2, "Cut the BLUE wire"},

        {{YELLOW, RED, RED, WHITE, BLUE}, 5, 3, "Yellow is youngest, Reds are twins, White is oldest, and Blue is a stranger. Who dies of old age first?"},
        {{RED, BLUE, GREEN, YELLOW, BLUE, GREEN}, 6, 4, "Cut the first wire that appears for a second time."},
        {{WHITE, GREEN, WHITE, ORANGE, BLUE, GREEN}, 6, 4, "Cut the second wire that only appears once."},
        {{RED, GREEN, BLUE, PURPLE}, 4, 1, "Cut the color that is directly opposite the first wire on the color wheel."},
        {{RED, YELLOW, ORANGE, BLUE, YELLOW}, 5, 3, "Cut the wire whose letters match its position"},
        {{PURPLE, ORANGE, WHITE, RED, RED, BLUE}, 6, 5, "Count the letters in each color. 4*5-6-3+1 = ?"}};

    WirePuzzle p = puzzles[rand() % sizeof(puzzles) / sizeof(puzzles[0])];
    // WirePuzzle p = puzzles[0];

    if (!isClient)
    { // skip if server
        drawCenteredTextBlock(p.hint);
        return false;
    }
    else
    {
        return doWireGame(p.colors, p.count, p.correct);
    }
}

bool doWireGame(uint16_t colors[], int colorCount, int correctWire)
{
    int wireX[10];
    int wireY[10];
    int wireW = 40;
    int wireH = 150;
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print("Cut the Correct Wire");

    int spacing = 320 / (colorCount + 1);

    for (int i = 0; i < colorCount; i++)
    {
        wireX[i] = spacing * (i + 1) - wireW / 2;
        wireY[i] = 60;

        M5.Lcd.fillRect(wireX[i], wireY[i], wireW, wireH, colors[i]);
    }

    while (true)
    {
        M5.update();

        if (M5.Touch.ispressed())
        {
            TouchPoint_t touch = M5.Touch.getPressPoint();
            int tx = touch.x;
            int ty = touch.y;

            for (int i = 0; i < colorCount; i++)
            {
                if (tx >= wireX[i] && tx <= wireX[i] + wireW &&
                    ty >= wireY[i] && ty <= wireY[i] + wireH)
                {
                    return i == correctWire;
                }
            }

            delay(150);
        }
    }
}
