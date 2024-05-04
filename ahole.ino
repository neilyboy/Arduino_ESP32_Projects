#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_MINUS_1 32
#define BUTTON_PLUS_1 33
#define BUTTON_PLUS_3 25
#define BUTTON_PLUS_5 26
#define BUTTON_NEXT 27

#define LED_PIN 14
#define LED_COUNT 24
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int numberOfPlayers = 1; // Initialize number of players to 1
int currentPlayer = 1; // Initialize current player to 1
int playerScores[8] = {0}; // Array to store player scores, maximum 8 players
bool gameOver = false; // Flag to indicate if the game is over

void updateDisplay();
void updateLEDsAttractMode();
void updateLEDsBasedOnScore(int score);

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_MINUS_1, INPUT_PULLUP);
  pinMode(BUTTON_PLUS_1, INPUT_PULLUP);
  pinMode(BUTTON_PLUS_3, INPUT_PULLUP);
  pinMode(BUTTON_PLUS_5, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display(); // Display initialization
  display.clearDisplay();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  updateDisplay(); // Initial display of number of players
}

void loop() {
  if (gameOver) {
    return; // Exit loop if game is over
  }
  updateLEDsRandomColor();
  // Check for button presses to change number of players
  if (digitalRead(BUTTON_MINUS_1) == LOW) {
    delay(200); // debounce
    numberOfPlayers--;
    if (numberOfPlayers < 1) {
      numberOfPlayers = 1;
    }
    updateDisplay();
  }

  if (digitalRead(BUTTON_PLUS_1) == LOW) {
    delay(200); // debounce
    numberOfPlayers++;
    if (numberOfPlayers > 8) {
      numberOfPlayers = 8;
    }
    updateDisplay();
  }

  if (digitalRead(BUTTON_PLUS_3) == LOW) {
    delay(200); // debounce
    numberOfPlayers += 3;
    if (numberOfPlayers > 8) {
      numberOfPlayers = 8;
    }
    updateDisplay();
  }

  if (digitalRead(BUTTON_PLUS_5) == LOW) {
    delay(200); // debounce
    numberOfPlayers += 5;
    if (numberOfPlayers > 8) {
      numberOfPlayers = 8;
    }
    updateDisplay();
  }

  if (digitalRead(BUTTON_NEXT) == LOW) {
    delay(200); // debounce
    while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
    if (gameOver) {
      return; // Exit loop if game is over
    }
    startGame();
  }
}

void startGame() {
  bool shootout = false; // Flag to indicate shootout
  int shootoutPlayers[8] = {0}; // Array to store players in shootout
  int shootoutScores[8] = {0}; // Array to store shootout scores
  int shootoutRound = 1; // Initialize shootout round
  int shootoutWinner = 0; // Initialize shootout winner

  // Loop through each player
  for (int i = 0; i < numberOfPlayers; i++) {
    currentPlayer = i + 1;
    // Check if the player has been eliminated
    if (playerScores[currentPlayer - 1] == -100) {
      continue; // Skip this player if they have been eliminated
    }
    int playerScore = playerScores[currentPlayer - 1]; // Get previous score
    while (true) {
      // Display current score
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.print("PLAYER ");
      display.print(currentPlayer);
      display.setCursor(0, 30);
      display.print("Score: ");
      display.print(playerScore);
      display.display();

      updateLEDsBasedOnScore(playerScore); // Update LEDs based on player score

      // Check for button presses to update score
      if (digitalRead(BUTTON_MINUS_1) == LOW) {
        delay(200); // debounce
        playerScore--;
      }
      if (digitalRead(BUTTON_PLUS_1) == LOW) {
        delay(200); // debounce
        playerScore++;
      }
      if (digitalRead(BUTTON_PLUS_3) == LOW) {
        delay(200); // debounce
        playerScore += 3;
      }
      if (digitalRead(BUTTON_PLUS_5) == LOW) {
        delay(200); // debounce
        playerScore += 5;
      }

      // Check if NEXT button is pressed to move to the next player
      if (digitalRead(BUTTON_NEXT) == LOW) {
        delay(200); // debounce
        while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
        
        // Game logic
        if (playerScore < -10) {
          // Player is eliminated
          playerScores[currentPlayer - 1] = -100; // Mark the player as eliminated
          break; // Exit the loop to move to the next player
        } else if (playerScore > 40) {
          // Player score reset to 22 on the next round
          playerScores[currentPlayer - 1] = 22;
          break; // Exit the loop to move to the next player
        } else {
          // Store player score and move to the next player
          playerScores[currentPlayer - 1] = playerScore; // Store player score
          break; // Exit the loop to move to the next player
        }
      }
    }
  }

  // Check if any player wins the game
  int winners = 0;
  int winningPlayer = 0;
  for (int i = 0; i < numberOfPlayers; i++) {
    if (playerScores[i] == 40) {
      winners++;
      winningPlayer = i + 1;
    }
  }

  if (winners == 1) {
    gameOver = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("PLAYER ");
    display.print(winningPlayer);
    display.println(" WINS!");
    display.display();
    delay(2000); // Let the user read the message
    return;
  }

  if (winners > 1) {
    shootout = true;
    for (int i = 0; i < numberOfPlayers; i++) {
      if (playerScores[i] == 40) {
        shootoutPlayers[i] = 1;
      }
    }
  }

  if (shootout) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("SHOOTOUT!");
    display.display();
    delay(2000); // Let the user read the message

    // Run shootout rounds
    while (true) {
      // Display shootout round
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.print("Shootout Round ");
      display.println(shootoutRound);
      display.display();

      // Loop through shootout players
      int maxScore = -1;
      int winningPlayer = 0;
      bool tie = false;
      for (int i = 0; i < numberOfPlayers; i++) {
        if (shootoutPlayers[i] == 1) {
          currentPlayer = i + 1;
          int playerScore = shootoutScores[i]; // Get shootout player score
          while (true) {
            // Display current score
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 10);
            display.print("PLAYER ");
            display.print(currentPlayer);
            display.setCursor(0, 30);
            display.print("Score: ");
            display.print(playerScore);
            display.display();

            // Check for button presses to update score
            if (digitalRead(BUTTON_MINUS_1) == LOW) {
              delay(200); // debounce
              playerScore--;
            }
            if (digitalRead(BUTTON_PLUS_1) == LOW) {
              delay(200); // debounce
              playerScore++;
            }
            if (digitalRead(BUTTON_PLUS_3) == LOW) {
              delay(200); // debounce
              playerScore += 3;
            }
            if (digitalRead(BUTTON_PLUS_5) == LOW) {
              delay(200); // debounce
              playerScore += 5;
            }

            // Check if NEXT button is pressed to move to the next player
            if (digitalRead(BUTTON_NEXT) == LOW) {
              delay(200); // debounce
              while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
              
              // Store shootout player score and move to the next player
              shootoutScores[i] = playerScore; // Store shootout player score
              break; // Exit the loop to move to the next player
            }
          }
          if (playerScore > maxScore) {
            maxScore = playerScore;
            winningPlayer = currentPlayer;
            tie = false;
          } else if (playerScore == maxScore) {
            tie = true;
          }
        }
      }

      if (!tie) {
        gameOver = true;
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 10);
        display.print("PLAYER ");
        display.print(winningPlayer);
        display.println(" WINS THE SHOOTOUT!");
        display.display();
        delay(2000); // Let the user read the message
        return;
      }

      shootoutRound++;
    }

  }

  // Game round completed
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("ROUND COMPLETED");
  display.display();
  delay(2000); // Let the user read the message

  // Automatically start the next round
  startGame();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("ENTER NUMBER OF");
  display.println("PLAYERS 1-8:");
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print(numberOfPlayers);
  display.display();
}

void updateLEDsBasedOnScore(int score) {
  if (score >= -10 && score <= 0) {
    strip.fill(0xF6D6D6);
    strip.show();
  } else if (score >= 1 && score <= 10) {
    strip.fill(0xF6F7C4);
    strip.show();
  } else if (score >= 11 && score <= 20) {
    strip.fill(0xFFD9B7);
    strip.show();
  } else if (score >= 21 && score <= 30) {
    strip.fill(0xA1EEBD);
    strip.show();
  } else if (score >= 31 && score <= 40) {
    strip.fill(0x7BD3EA);
    strip.show();
  }
}

void updateLEDsRandomColor() {
  static unsigned long lastChange = 0;
  static uint32_t currentColor = strip.Color(random(256), random(256), random(256));
  static uint32_t targetColor = strip.Color(random(256), random(256), random(256));
  static int fadeAmount = 5; // how many points to fade the LED by
  static int brightness = 0; // how bright the LED is

  // Check if it's time to change colors
  if (millis() - lastChange >= random(500, 2000)) {
    lastChange = millis();
    targetColor = strip.Color(random(256), random(256), random(256));
  }

  // Fade in/out effect
  brightness = brightness + fadeAmount;
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  currentColor = strip.Color(
    (uint8_t)(((currentColor >> 16) & 0xFF) + fadeAmount),
    (uint8_t)(((currentColor >> 8) & 0xFF) + fadeAmount),
    (uint8_t)((currentColor & 0xFF) + fadeAmount)
  );

  // Blend current color towards target color
  uint8_t r = ((currentColor >> 16) & 0xFF) + (((targetColor >> 16) & 0xFF) - ((currentColor >> 16) & 0xFF)) / 10;
  uint8_t g = ((currentColor >> 8) & 0xFF) + (((targetColor >> 8) & 0xFF) - ((currentColor >> 8) & 0xFF)) / 10;
  uint8_t b = (currentColor & 0xFF) + (((targetColor) & 0xFF) - (currentColor & 0xFF)) / 10;
  currentColor = strip.Color(r, g, b);

  // Set the color of all pixels
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currentColor);
  }
  strip.show();
}
