#include "GameHelpers.h"
void drawCenteredTextBlock(const char *text)
{
    int padding = 20;
    int lineHeight = 32;

    int screenW = M5.Lcd.width();
    int screenH = M5.Lcd.height();

    int maxWidth = screenW - padding * 2;

    String str = String(text);
    String lines[20]; // adjust if needed
    int lineCount = 0;

    String currentLine = "";

    // --- Word wrapping ---
    int start = 0;
    while (start < str.length())
    {
        int spaceIndex = str.indexOf(' ', start);
        if (spaceIndex == -1)
            spaceIndex = str.length();

        String word = str.substring(start, spaceIndex);
        String testLine = currentLine.length() == 0 ? word : currentLine + " " + word;

        if (M5.Lcd.textWidth(testLine) > maxWidth)
        {
            if (lineCount < 20)
            {
                lines[lineCount++] = currentLine;
            }
            currentLine = word;
        }
        else
        {
            currentLine = testLine;
        }

        start = spaceIndex + 1;
    }

    if (currentLine.length() > 0 && lineCount < 20)
    {
        lines[lineCount++] = currentLine;
    }

    // --- Vertical centering ---
    int totalTextHeight = lineCount * lineHeight;
    int startY = (screenH - totalTextHeight) / 2;

    // --- Draw lines ---
    for (int i = 0; i < lineCount; i++)
    {
        int lineWidth = M5.Lcd.textWidth(lines[i]);
        int x = (screenW - lineWidth) / 2;
        int y = startY + i * lineHeight;

        M5.Lcd.setCursor(x, y);
        M5.Lcd.print(lines[i]);
    }
}

int colorTicker = 0;
// ====================== YOU WON! SCREEN (add to the bottom of main.cpp) ======================
void showWin()
{
    M5.Lcd.fillScreen(BLACK);

    // --- Confetti setup (simple, lightweight, no extra structs needed) ---
    const int NUM_CONFETTI = 40;
    int cx[NUM_CONFETTI], cy[NUM_CONFETTI], cvx[NUM_CONFETTI], cvy[NUM_CONFETTI];
    uint16_t ccol[NUM_CONFETTI];

    uint16_t festiveColors[4] = {RED, GREEN, BLUE, YELLOW};

    for (int i = 0; i < NUM_CONFETTI; i++)
    {
        cx[i] = rand() % 320;
        cy[i] = rand() % 120 - 80; // start above the screen
        cvx[i] = rand() % 5 - 2;   // gentle horizontal drift
        cvy[i] = rand() % 4 + 2;   // falling speed
        ccol[i] = festiveColors[rand() % 4];
    }

    // Text color cycling for the big "YOU WON!"
    uint16_t textColors[4] = {GREEN, YELLOW, RED, BLUE};
    int colorIdx = 0;

    // --- Animation loop (runs until touch) ---
    while (true)
    {
        M5.update();

        // Touch to dismiss
        if (M5.Touch.ispressed())
        {
            break;
        }

        M5.Lcd.fillScreen(BLACK);

        // Update confetti
        for (int i = 0; i < NUM_CONFETTI; i++)
        {
            cx[i] += cvx[i];
            cy[i] += cvy[i];

            // Gentle horizontal bounce
            if (cx[i] < 0 || cx[i] > 320)
                cvx[i] = -cvx[i];

            // Reset when it falls off the bottom
            if (cy[i] > 240)
            {
                cy[i] = -10;
                cx[i] = rand() % 320;
                cvx[i] = rand() % 5 - 2;
                cvy[i] = rand() % 4 + 2;
                ccol[i] = festiveColors[rand() % 4];
            }
        }

        // Draw big "YOU WON!" (centered, color-cycling)
        M5.Lcd.setTextSize(5);
        String msg = "YOU WON!";
        int textW = M5.Lcd.textWidth(msg);
        int textX = (320 - textW) / 2;
        M5.Lcd.setTextColor(textColors[colorIdx]);
        M5.Lcd.setCursor(textX, 65);
        M5.Lcd.print(msg);

        // Smaller subtitle
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE);
        String sub = "Touch to continue";
        int subW = M5.Lcd.textWidth(sub);
        M5.Lcd.setCursor((320 - subW) / 2, 180);
        M5.Lcd.print(sub);

        // Draw confetti on top
        for (int i = 0; i < NUM_CONFETTI; i++)
        {
            M5.Lcd.fillCircle(cx[i], cy[i], 3, ccol[i]);
        }

        if (colorTicker++ % 4 == 0)
        {
            colorIdx = (colorIdx + 1) % 4;
        }

        delay(50); // ~20 fps – smooth but very light on the ESP32
    }

    // Clean up when dismissed
    M5.Lcd.fillScreen(BLACK);
}

// ====================== YOU LOST! SCREEN (add to the bottom of main.cpp) ======================
void showLose()
{
    M5.Lcd.fillScreen(BLACK);

    // --- Simple falling "X" marks / broken pieces animation ---
    const int NUM_X = 25;
    int cx[NUM_X], cy[NUM_X], cvx[NUM_X], cvy[NUM_X];
    uint16_t xcol[NUM_X];

    for (int i = 0; i < NUM_X; i++)
    {
        cx[i] = rand() % 320;
        cy[i] = rand() % 100 - 100;                     // start above screen
        cvx[i] = rand() % 7 - 3;                        // more chaotic horizontal movement
        cvy[i] = rand() % 5 + 4;                        // faster fall
        xcol[i] = (rand() % 2 == 0) ? RED : TFT_MAROON; // red tones for "loss" feel
    }

    // Text color (pulsing between red and dark red)
    uint16_t textColors[2] = {RED, TFT_MAROON};
    int colorIdx = 0;
    int pulseCounter = 0;

    while (true)
    {
        M5.update();

        // Touch anywhere to dismiss
        if (M5.Touch.ispressed())
        {
            break;
        }

        M5.Lcd.fillScreen(BLACK);

        // Update and draw falling X marks
        for (int i = 0; i < NUM_X; i++)
        {
            cx[i] += cvx[i];
            cy[i] += cvy[i];

            // Bounce off sides
            if (cx[i] < 0 || cx[i] > 320)
                cvx[i] = -cvx[i];

            // Reset when off bottom
            if (cy[i] > 240)
            {
                cy[i] = -20;
                cx[i] = rand() % 320;
                cvx[i] = rand() % 7 - 3;
                cvy[i] = rand() % 5 + 4;
            }

            // Draw X
            int size = 8 + (i % 6); // varied sizes
            M5.Lcd.drawLine(cx[i] - size, cy[i] - size, cx[i] + size, cy[i] + size, xcol[i]);
            M5.Lcd.drawLine(cx[i] + size, cy[i] - size, cx[i] - size, cy[i] + size, xcol[i]);
        }

        // Big "YOU LOST!" text (pulsing red)
        M5.Lcd.setTextSize(5);
        String msg = "YOU LOST";
        int textW = M5.Lcd.textWidth(msg);
        int textX = (320 - textW) / 2;
        M5.Lcd.setTextColor(textColors[colorIdx]);
        M5.Lcd.setCursor(textX, 70);
        M5.Lcd.print(msg);

        // Subtitle
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE);
        String sub = "Better luck next time";
        int subW = M5.Lcd.textWidth(sub);
        M5.Lcd.setCursor((320 - subW) / 2, 165);
        M5.Lcd.print(sub);

        pulseCounter++;
        if (pulseCounter % 8 == 0) // slow pulse
        {
            colorIdx = (colorIdx + 1) % 2;
        }

        delay(60); // ~22 fps – smooth and lightweight
    }

    // Clean exit
    M5.Lcd.fillScreen(BLACK);
}