
import java.util.Random;
import java.util.Scanner;

public class WireGame {

    // ===== ENUMS =====
    enum Color {
        RED, BLUE, GREEN, YELLOW, WHITE, BLACK
    }

    // ===== WIRE CLASS =====
    static class Wire {

        Color color;
        int index;

        Wire(Color color, int index) {
            this.color = color;
            this.index = index;
        }
    }

    // ===== RANDOM =====
    private Random rand = new Random();

    private Color randomColor() {
        Color[] values = Color.values();
        return values[rand.nextInt(values.length)];
    }

    private int randRange(int min, int maxInclusive) {
        return rand.nextInt(maxInclusive - min + 1) + min;
    }

    // ===== GAME STATE =====
    private Wire[] wires;
    private int correctWireIndex;

    // ===== GAME SETUP =====
    public void startGame() {
        generateWires();
        correctWireIndex = evaluateCorrectWire();

        printWires();
        runPlayerLoop();
    }

    private void generateWires() {
        int numWires = randRange(3, 6);
        wires = new Wire[numWires];

        for (int i = 0; i < numWires; i++) {
            wires[i] = new Wire(randomColor(), i);
        }
    }

    // ===== RULE ENGINE =====
    private int evaluateCorrectWire() {

        int wireCount = wires.length;

        int redCount = 0;
        int blueCount = 0;
        int yellowCount = 0;

        for (Wire w : wires) {
            if (w.color == Color.RED) {
                redCount++;
            }
            if (w.color == Color.BLUE) {
                blueCount++;
            }
            if (w.color == Color.YELLOW) {
                yellowCount++;
            }
        }

        // Example rules (KTANE-inspired but simplified)
        // Rule set for 3 wires
        if (wireCount == 3) {
            if (redCount == 0) {
                return 2; // last wire
            } else if (wires[2].color == Color.WHITE) {
                return 2;
            } else if (blueCount > 1) {
                return getLastIndexOf(Color.BLUE);
            } else {
                return 2;
            }
        }

        // Rule set for 4 wires
        if (wireCount == 4) {
            if (redCount > 1) {
                return getLastIndexOf(Color.RED);
            } else if (wires[3].color == Color.YELLOW && redCount == 0) {
                return 0;
            } else if (blueCount == 1) {
                return 0;
            } else if (yellowCount > 1) {
                return 3;
            } else {
                return 1;
            }
        }

        // Rule set for 5 wires
        if (wireCount == 5) {
            if (wires[4].color == Color.BLACK) { // won't happen yet, placeholder
                return 3;
            } else if (redCount == 1 && yellowCount > 1) {
                return 0;
            } else if (blueCount == 0) {
                return 1;
            } else {
                return 0;
            }
        }

        // Rule set for 6 wires
        if (wireCount == 6) {
            if (yellowCount == 0) {
                return 2;
            } else if (yellowCount == 1 && redCount > 1) {
                return 3;
            } else {
                return 4;
            }
        }

        return 0; // fallback
    }

    private int getLastIndexOf(Color color) {
        for (int i = wires.length - 1; i >= 0; i--) {
            if (wires[i].color == color) {
                return i;
            }
        }
        return 0;
    }

    // ===== UI (TEXT) =====
    private void printWires() {
        System.out.println("=== WIRE PANEL ===");
        for (Wire w : wires) {
            System.out.println("[" + w.index + "] " + w.color);
        }
    }

    private void runPlayerLoop() {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Cut which wire? Enter index: ");
        int choice = scanner.nextInt();

        if (choice == correctWireIndex) {
            System.out.println("✅ Correct! Module solved.");
        } else {
            System.out.println("❌ Strike!");
            System.out.println("Correct wire was: " + correctWireIndex);
        }
    }

    // ===== MAIN =====
    public static void main(String[] args) {
        new WireGame().startGame();
    }
}
