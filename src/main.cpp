#include <Arduino.h>
#include <M5Core2.h>
#include "BLEHandler.h" // BLE Setup and communication handler
#include "WireGame.h"
#include "ButtonGame.h"
#include "CodeGame.h"
#include "MazeGame.h"
#include "GameHelpers.h"

// Func decs
// - Touch handling
TouchPoint_t touch;
bool isClient;
// - Data handling
String data;
String newData;
// - Syncing the randSeed
unsigned long randSeed = 0;
boolean randEdge = false;
boolean randSet = false;
// - Game life handler
int numLives = 5;
int numWins = 0;

bool isTouchIn(int x, int y, int w, int h, int tx, int ty);
bool pickIsClient();
void checkData();
void syncRand();
void runGameSync(bool (*launchGame)(bool, int), String gameName, int difficulty);

// ====================== SETUP ======================
void setup()
{
  M5.begin();
  Serial.begin(115200);

  isClient = pickIsClient(); // false = Host/Server, true = Join/Client

  Serial.println(isClient ? "I am CLIENT" : "I am SERVER");
  if (!isClient)
  {
    randSeed = millis();
    srand(randSeed);
    Serial.print("Rand seeded with");
    Serial.println(randSeed);
  }
  BLE_Begin(isClient); // ← one line setup, handles everything
  syncRand();
}

// ====================== LOOP ======================
void loop()
{
  M5.update();
  doHeartbeat(isClient);
  checkData();

  // ====================== SEND EXAMPLE (both devices do this the same way) ======================

  while (true)
  {
    switch (rand() % 4)
    {
    case 0:
      runGameSync(launchButtonGame, "Button Game", 0);
      break;
    case 1:
      runGameSync(launchWireGame, "Wire Game", 0);
      break;
    case 2:
      runGameSync(launchCodeEntryGame, "Code Game", 0);
      break;
    case 3:
      runGameSync(launchMazeGame, "Maze Game", 0);
      break;
    }

    if (numWins == 5)
    {
      showWin();
    }
    if (numLives == 0)
    {
      showLose();
    }
  }

  delay(50); // adjust to your desired rate — BLE handles the heavy lifting
}

// ====================== UI functions (unchanged) ======================
bool isTouchIn(int x, int y, int w, int h, int tx, int ty)
{
  return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

/*
Print a simple startup menu
Returns true if user selected to be client, false if server
*/
bool pickIsClient()
{
  int BTN1_X = 40;
  int BTN1_Y = 120;
  int BTN_W = 100;
  int BTN_H = 60;
  int BTN2_X = 180;
  int BTN2_Y = 120;

  M5.Lcd.fillScreen(BLACK);

  // ===== Title Bar =====
  M5.Lcd.fillRect(0, 0, 320, 40, DARKGREY);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(60, 10);
  M5.Lcd.print("Some game idk"); // TODO: Pick better title

  // ===== Button 1 =====
  M5.Lcd.fillRect(BTN1_X, BTN1_Y, BTN_W, BTN_H, BLUE);
  M5.Lcd.setCursor(BTN1_X + 20, BTN1_Y + 20);
  M5.Lcd.print("Host");

  // ===== Button 2 =====
  M5.Lcd.fillRect(BTN2_X, BTN2_Y, BTN_W, BTN_H, RED);
  M5.Lcd.setCursor(BTN2_X + 20, BTN2_Y + 20);
  M5.Lcd.print("Join");

  int tx;
  int ty;

  // Small handler to update screen and await button press
  while (true)
  {
    M5.update();
    if (M5.Touch.ispressed())
    {
      touch = M5.Touch.getPressPoint();

      tx = touch.x;
      ty = touch.y;

      if (isTouchIn(BTN1_X, BTN1_Y, BTN_W, BTN_H, tx, ty))
      {
        M5.Lcd.fillRect(0, 200, 320, 40, BLACK);
        M5.Lcd.setCursor(40, 200);
        M5.Lcd.print("Hosting game...");
        return false;
      }

      if (isTouchIn(BTN2_X, BTN2_Y, BTN_W, BTN_H, tx, ty))
      {
        M5.Lcd.fillRect(0, 200, 320, 40, BLACK);
        M5.Lcd.setCursor(40, 200);
        M5.Lcd.print("Joining game...");
        return true;
      }
    }
  }
}

/*
Periodically check to see if new data exists
*/
void checkData()
{
  if (BLE_HasNewData())
  {
    newData = BLE_Read();
    if (data != newData)
    {
      data = newData;
      Serial.println("→ I received: " + data);
    }
  }
}

/*
Handle syncing srand between devices
*/
void syncRand()
{
  while (true)
  {
    doHeartbeat(isClient);
    checkData();
    if (!isClient)
    {
      BLE_Send((randEdge ? "A" : "B") + String(randSeed)); // swapping between A and B to update new data
      randEdge = !randEdge;
      if (data == "Received SRAND")
        return;
    }
    else
    {
      if (data.startsWith("A") || data.startsWith("B"))
      {
        randSeed = data.substring(1).toInt();
        srand(randSeed);
        Serial.print("Rand seeded with");
        Serial.println(randSeed);
        BLE_Send("Received SRAND");
        return;
      }
    }
    delay(50);
  }
}

// Generic function to run any game and handle server/client sync
void runGameSync(bool (*launchGame)(bool, int), String gameName, int difficulty)
{
  bool result = launchGame(isClient, difficulty); // call the passed-in function

  if (isClient)
  {
    BLE_Send(result ? "WIN" : "LOSE");
  }
  else
  {
    while (true)
    {
      doHeartbeat(isClient);
      checkData();
      if (data == "WIN")
      {
        result = true;
        break;
      }
      else if (data == "LOSE")
      {
        result = false;
        break;
      }
    }
  }

  BLE_Send("Intermission");

  Serial.println("I am " + String(isClient ? "client and I sent" : "server and I received") + (result ? " WIN " : " LOSE"));

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  // Role display
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.print(isClient ? "CLIENT" : "SERVER");

  // Game name
  M5.Lcd.setCursor(20, 60);
  M5.Lcd.print("Game: ");
  M5.Lcd.print(gameName);

  // Result
  M5.Lcd.setCursor(20, 100);
  if (result)
  {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.print("Result: WIN");
  }
  else
  {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("Result: LOSS");

    // Only decrement once per call cycle
    numLives--;
  }

  // Lives display
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(20, 140);
  M5.Lcd.print("Lives: ");
  M5.Lcd.print(numLives);

  M5.Lcd.setCursor(20, 180);
  M5.Lcd.print("Wins: ");
  M5.Lcd.print(++numWins);
  delay(5000);
  M5.Lcd.fillScreen(TFT_BLACK);
}