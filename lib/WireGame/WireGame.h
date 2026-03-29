#pragma once

#include <M5Core2.h>

bool launchWireGame(boolean isClient, int difficulty);
bool doWireGame(uint16_t colors[], int colorCount, int correctWire);
