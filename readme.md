# M5Core2 Cooperative Puzzle (BLE)

## Overview

This project is a cooperative puzzle game designed for **two M5Core2 devices communicating over BLE**. Each player receives different pieces of information on their screen and must communicate with the other player to solve puzzles together.

---

## Core Concept

Each device shows unique puzzle data. Neither player has enough information to solve the puzzle alone.
This could be in the style of Keep Talking and Nobody Explodes - a cooperative bomb-diffusal game, one player has the manual, one player is actually working with the bomb - or some kind of asymmetric puzzle solfer - one player moves through the puzzle blind, the other player can actually see it.

Players should be able to:

1. Share the information shown on their screen
2. Coordinate actions
3. Solve puzzles that require combining both perspectives

---

## Example Puzzle Concepts

Possible puzzle ideas include:

### Asymmetric Code Puzzle

One player sees part of a code while the other sees rules for ordering the digits.

Example:

Player A screen:
```
Code: 3 _ 7
Symbols: △ ○ □
```

Player B screen:
```
Rules:
○ before △
□ after ○
Code: _ 2 _
```

Players must determine the correct final code.

---

### Blind Maze Navigation

One player controls a character but cannot see the maze.
The other player sees the maze but cannot move or see the character.
Players must verbally coordinate to navigate to the exit.

---

### Multi-System Repair Puzzle

Players control different subsystems of a failing machine.

Example subsystems:

* Power routing
* Cooling system
* Pressure valves
* Reactor output
Events occur that require coordination to stabilize the system.

---

## Technical Goals

The project demonstrates:

* BLE server/client communication
* Touchscreen input handling
* Screen rendering with M5Unified
* Event-based state synchronization
* Embedded game design