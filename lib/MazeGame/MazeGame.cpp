#include "MazeGame.h"
#include <stdlib.h>
#include <M5Core2.h>

// ====================== INTERNAL STRUCT & CONSTANTS ======================
struct MazePuzzle
{
    bool walls[5][5];
    int startX, startY;
    int goalX, goalY;
};

// Layout constants – tuned for 320×240 screen, perfect centering, no edge clipping
const int cellSize = 28;
const int gridX = 90; // (320 - 140) / 2
const int gridY = 55; // leaves room for title + directional buttons

const int btnW = 46;
const int btnH = 36;

// ====================== DRAW HELPERS ======================
void drawGrid()
{
    for (int i = 0; i <= 5; i++)
    {
        M5.Lcd.drawLine(gridX, gridY + i * cellSize,
                        gridX + 5 * cellSize, gridY + i * cellSize, WHITE);
        M5.Lcd.drawLine(gridX + i * cellSize, gridY,
                        gridX + i * cellSize, gridY + 5 * cellSize, WHITE);
    }
}

void drawHostMaze(const MazePuzzle &p)
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print("Guide the client");

    drawGrid();

    // Barriers – perfectly centered inside each cell (inset = 3 px)
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 5; col++)
        {
            if (p.walls[row][col])
            {
                int left = gridX + col * cellSize + 3;
                int top = gridY + row * cellSize + 3;
                M5.Lcd.fillRect(left, top, cellSize - 6, cellSize - 6, TFT_DARKGREY);
            }
        }
    }
}

void drawClientMaze(int currX, int currY, int goalX, int goalY)
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print("Reach the\n Goal");

    drawGrid();

    // Goal marker – green frame + centered "G"
    int goalLeft = gridX + goalX * cellSize;
    int goalTop = gridY + goalY * cellSize;
    M5.Lcd.drawRect(goalLeft + 3, goalTop + 3, cellSize - 6, cellSize - 6, GREEN);

    // M5.Lcd.setTextSize(3);
    // M5.Lcd.setTextColor(GREEN);
    // M5.Lcd.setCursor(goalLeft + cellSize / 2 - 9, goalTop + cellSize / 2 - 12);
    // M5.Lcd.print("G");

    // Player dot – centered
    int pcx = gridX + currX * cellSize + cellSize / 2;
    int pcy = gridY + currY * cellSize + cellSize / 2;
    M5.Lcd.fillCircle(pcx, pcy, 11, RED);

    // Directional buttons – now safely inside screen edges
    int upX = gridX + (5 * cellSize - btnW) / 2;
    int upY = gridY - btnH - 8;
    int downY = gridY + 5 * cellSize + 6;
    int leftX = gridX - btnW - 8;
    int leftY = gridY + (5 * cellSize - btnH) / 2;
    int rightX = gridX + 5 * cellSize + 8;

    // UP
    M5.Lcd.fillRect(upX, upY, btnW, btnH, BLUE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(upX + 15, upY + 7);
    M5.Lcd.print("^");

    // DOWN
    M5.Lcd.fillRect(upX, downY, btnW, btnH, BLUE);
    M5.Lcd.setCursor(upX + 17, downY + 7);
    M5.Lcd.print("v");

    // LEFT
    M5.Lcd.fillRect(leftX, leftY, btnW, btnH, BLUE);
    M5.Lcd.setCursor(leftX + 13, leftY + 9);
    M5.Lcd.print("<");

    // RIGHT
    M5.Lcd.fillRect(rightX, leftY, btnW, btnH, BLUE);
    M5.Lcd.setCursor(rightX + 13, leftY + 9);
    M5.Lcd.print(">");
}

// ====================== CLIENT GAME LOGIC ======================
bool doClientMaze(MazePuzzle p)
{
    int currX = p.startX;
    int currY = p.startY;

    drawClientMaze(currX, currY, p.goalX, p.goalY);

    while (true)
    {
        M5.update();

        if (M5.Touch.ispressed())
        {
            TouchPoint_t touch = M5.Touch.getPressPoint();
            int tx = touch.x;
            int ty = touch.y;

            int newX = currX;
            int newY = currY;
            bool attemptedMove = false;

            // Button hit boxes (exact match to drawClientMaze)
            int upX = gridX + (5 * cellSize - btnW) / 2;
            int upY = gridY - btnH - 8;
            int downY = gridY + 5 * cellSize + 6;
            int leftX = gridX - btnW - 8;
            int leftY = gridY + (5 * cellSize - btnH) / 2;
            int rightX = gridX + 5 * cellSize + 8;

            if (tx >= upX && tx <= upX + btnW && ty >= upY && ty <= upY + btnH)
            {
                newY = currY - 1;
                attemptedMove = true;
            }
            else if (tx >= upX && tx <= upX + btnW && ty >= downY && ty <= downY + btnH)
            {
                newY = currY + 1;
                attemptedMove = true;
            }
            else if (tx >= leftX && tx <= leftX + btnW && ty >= leftY && ty <= leftY + btnH)
            {
                newX = currX - 1;
                attemptedMove = true;
            }
            else if (tx >= rightX && tx <= rightX + btnW && ty >= leftY && ty <= leftY + btnH)
            {
                newX = currX + 1;
                attemptedMove = true;
            }

            if (attemptedMove)
            {
                if (newX < 0 || newX > 4 || newY < 0 || newY > 4)
                {
                    delay(100);
                    continue;
                }

                if (p.walls[newY][newX])
                {
                    return false; // hit barrier = LOSS
                }

                currX = newX;
                currY = newY;
                drawClientMaze(currX, currY, p.goalX, p.goalY);

                if (currX == p.goalX && currY == p.goalY)
                {
                    return true; // WIN
                }

                delay(180); // debounce
            }
        }
        delay(10);
    }
}

// ====================== LAUNCH ======================
bool launchMazeGame(boolean isClient)
{
    M5.Lcd.fillScreen(BLACK);

    MazePuzzle puzzles[] = {
        // Puzzle 0
        {{{false, true, false, false, false},
          {false, true, false, false, false},
          {false, true, false, true, false},
          {false, false, false, true, false},
          {false, false, false, false, false}},
         0,
         0,
         4,
         4},

        // Puzzle 1
        {{{false, false, true, false, false},
          {false, true, false, true, false},
          {false, false, false, false, true},
          {true, false, true, false, false},
          {false, false, false, false, false}},
         0,
         1,
         4,
         4},

        // Puzzle 2
        {{{false, true, false, false, false},
          {false, false, false, false, false},
          {false, true, false, true, false},
          {false, true, false, false, true},
          {false, false, false, false, false}},
         0,
         4,
         4,
         0},

        // Puzzle 3
        {{{false, false, false, false, false},
          {true, true, false, true, true},
          {false, false, false, false, false},
          {false, true, true, false, false},
          {false, false, false, false, false}},
         2,
         0,
         2,
         4},

        // Puzzle 4
        {{{false, true, true, true, true},
          {false, false, false, false, true},
          {true, true, true, false, true},
          {true, false, false, false, false},
          {true, true, true, true, false}},
         0,
         0,
         4,
         4},

        // Puzzle 5
        {{{false, false, false, false, false},
          {false, false, false, false, false},
          {false, false, false, false, false},
          {false, false, false, false, false},
          {false, false, false, false, false}},
         0,
         0,
         4,
         4},

        // Puzzle 6
        {{{false, false, false, false, false},
          {false, true, true, true, false},
          {false, true, false, true, false},
          {false, true, true, true, false},
          {false, false, false, false, false}},
         0,
         2,
         4,
         2},

        // Puzzle 7 - CORRIDOR
        {{{true, true, false, true, true},
          {true, true, false, true, true},
          {false, false, false, false, false},
          {true, true, false, true, true},
          {true, true, false, true, true}},
         2,
         0,
         2,
         4},

        // Puzzle 8 - ZIGZAG
        {{{false, false, true, false, false},
          {true, false, true, false, true},
          {false, false, false, false, false},
          {true, false, true, false, true},
          {false, false, true, false, false}},
         0,
         2,
         4,
         2},

    };

    int numPuzzles = sizeof(puzzles) / sizeof(puzzles[0]);
    int selected = rand() % numPuzzles;
    MazePuzzle p = puzzles[selected];

    if (!isClient)
    {
        drawHostMaze(p);
        return false;
    }
    else
    {
        return doClientMaze(p);
    }
}