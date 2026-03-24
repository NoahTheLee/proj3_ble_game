#include <Arduino.h>
#include <M5Core2.h>
#include "BLEHandler.h" // BLE Setup and communication handler

// ====================== UI only ======================
TouchPoint_t touch;
bool isClient;
String data;

bool isTouchIn(int x, int y, int w, int h, int tx, int ty);
bool pickIsClient();
void checkData();
void handleHeartbeat();

// ====================== SETUP ======================
void setup()
{
  M5.begin();
  Serial.begin(115200);

  isClient = pickIsClient(); // false = Host/Server, true = Join/Client

  Serial.println(isClient ? "I am CLIENT" : "I am SERVER");
  BLE_Begin(isClient); // ← one line setup, handles everything
}

// ====================== LOOP ======================
void loop()
{
  M5.update();

  // ====================== SEND EXAMPLE (both devices do this the same way) ======================
  if (M5.BtnA.wasPressed())
  { // or any trigger you want
    BLE_Send("Hello from " + String(isClient ? "CLIENT" : "SERVER") + " at " + String(millis()));
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
  M5.Lcd.print("Wire Game"); // TODO: Pick better title

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
    data = BLE_Read();
    Serial.println("→ I received: " + data);

    // Example: show on screen
    M5.Lcd.fillRect(0, 200, 320, 40, BLACK);
    M5.Lcd.setCursor(10, 210);
    M5.Lcd.print("RX: " + data.substring(0, 20)); // shorten if too long
  }
}

/*
Periodically update the client/server data
*/
void handleHeartbeat()
{
  if (isClient)
  {
    BLE_ClientHandle(); // keep client scanning & connected
  }
  else
  {
    BLE_ServerPollReceive(); // server must poll for incoming data
  }

  if (BLE_IsConnected())
  {
    M5.Lcd.fillRect(0, 0, 100, 20, GREEN);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.print("CONNECTED");
  }
}