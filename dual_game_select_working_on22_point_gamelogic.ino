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

enum GameMode { GAME_40_POINTS, GAME_22_POINTS }; // Enumeration for game modes
int selectedGame = GAME_40_POINTS; // Default selected game mode

void updateGameSelectionScreen();
void updatePlayerSelectionScreen();
void updateDisplay();
void updateLEDsBasedOnScore(int score);
void updateLEDsRandomColor();
void startGame();
bool gameSelected = false;



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

  updateGameSelectionScreen(); // Initial display of game selection screen
}

void loop() {
  if (!gameSelected) {
    // Game selection mode
    if (digitalRead(BUTTON_MINUS_1) == LOW) {
      delay(200); // debounce
      selectedGame--;
      if (selectedGame < GAME_40_POINTS) {
        selectedGame = GAME_22_POINTS;
      }
      updateGameSelectionScreen();
    }

    if (digitalRead(BUTTON_PLUS_1) == LOW) {
      delay(200); // debounce
      selectedGame++;
      if (selectedGame > GAME_22_POINTS) {
        selectedGame = GAME_40_POINTS;
      }
      updateGameSelectionScreen();
    }

    if (digitalRead(BUTTON_NEXT) == LOW) {
      delay(200); // debounce
      while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
      gameSelected = true;
      updatePlayerSelectionScreen();
    }
  } else {
    // Player selection mode
    if (digitalRead(BUTTON_MINUS_1) == LOW) {
      delay(200); // debounce
      numberOfPlayers--;
      if (numberOfPlayers < 1) {
        numberOfPlayers = 1;
      }
      updatePlayerSelectionScreen();
    }

    if (digitalRead(BUTTON_PLUS_1) == LOW) {
      delay(200); // debounce
      numberOfPlayers++;
      if (numberOfPlayers > 8) {
        numberOfPlayers = 8;
      }
      updatePlayerSelectionScreen();
    }

    if (digitalRead(BUTTON_NEXT) == LOW) {
      delay(200); // debounce
      while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
      // Start the game or do whatever you need to do next
      startGame();
    }
  }
}


void startGame() {
  // Add game mode-specific logic here
  if (selectedGame == GAME_40_POINTS) {
    // Existing 40 Point Game logic
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
          Serial.println("Minus 1 pressed. Score decremented.");
        }
        if (digitalRead(BUTTON_PLUS_1) == LOW) {
          delay(200); // debounce
          playerScore++;
          Serial.println("Plus 1 pressed. Score incremented.");
        }
        if (digitalRead(BUTTON_PLUS_3) == LOW) {
          delay(200); // debounce
          playerScore += 3;
          Serial.println("Plus 3 pressed. Score incremented by 3 (this is first bit of startgame).");
        }
        if (digitalRead(BUTTON_PLUS_5) == LOW) {
          delay(200); // debounce
          playerScore += 5;
          Serial.println("Plus 5 pressed. Score incremented by 5 (this is first bit of startgame).");
        }

        // Check if NEXT button is pressed to move to the next player
        if (digitalRead(BUTTON_NEXT) == LOW) {
          delay(200); // debounce
          while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
          
          // Game logic
          if (playerScore < -10) {
            // Player is eliminated
            playerScores[currentPlayer - 1] = -100; // Mark the player as eliminated
            Serial.println("Player eliminated.");
            break; // Exit the loop to move to the next player
          } else if (playerScore > 40) {
            // Player score reset to 22 on the next round
            playerScores[currentPlayer - 1] = 22;
            Serial.println("Player score reset to 22.");
            break; // Exit the loop to move to the next player
          } else {
            // Store player score and move to the next player
            playerScores[currentPlayer - 1] = playerScore; // Store player score
            Serial.println("Player score updated.");
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
      Serial.println("Game over. Winner announced.");
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
      Serial.println("Shootout initiated.");
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
                Serial.println("Minus 1 pressed. Score decremented.");
              }
              if (digitalRead(BUTTON_PLUS_1) == LOW) {
                delay(200); // debounce
                playerScore++;
                Serial.println("Plus 1 pressed. Score incremented.");
              }
              if (digitalRead(BUTTON_PLUS_3) == LOW) {
                delay(200); // debounce
                playerScore += 3;
                Serial.println("Plus 3 pressed. Score incremented by 3.");
              }
              if (digitalRead(BUTTON_PLUS_5) == LOW) {
                delay(200); // debounce
                playerScore += 5;
                Serial.println("Plus 5 pressed. Score incremented by 5.");
              }

              // Check if NEXT button is pressed to move to the next player
              if (digitalRead(BUTTON_NEXT) == LOW) {
                delay(200); // debounce
                while (digitalRead(BUTTON_NEXT) == LOW); // Wait until button is released
                
                // Store shootout player score and move to the next player
                shootoutScores[i] = playerScore; // Store shootout player score
                Serial.println("Shootout player score updated.");
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
          Serial.println("Shootout winner announced.");
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
    Serial.println("Round completed.");
    delay(2000); // Let the user read the message

    // Automatically start the next round
    startGame();

  } else if (selectedGame == GAME_22_POINTS) {
  // New 22 Point Game logic
  bool shootout = false; // Flag to indicate shootout
  int shootoutPlayers[8] = {0}; // Array to store players in shootout
  int shootoutScores[8] = {0}; // Array to store shootout scores
  int shootoutRound = 1; // Initialize shootout round
  int shootoutWinner = 0; // Initialize shootout winner
  int maxScore = -1; // Max score achieved in shootout
  int maxPlayer = -1; // Player who achieved max score

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
        } else if (playerScore >= 22) {
          // Player wins
          gameOver = true;
          maxScore = playerScore;
          maxPlayer = currentPlayer;
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
    if (playerScores[i] >= 22) {
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
      if (playerScores[i] >= 22) {
        shootoutPlayers[i] = 1;
      }
    }
  }

  // Check if any other players need an additional attempt
  for (int i = 0; i < numberOfPlayers; i++) {
    if (playerScores[i] < 22 && playerScores[i] != -100) {
      shootoutPlayers[i] = 1;
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
}


void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Game Mode: ");
  display.println(selectedGame == GAME_40_POINTS ? "40 Points" : "22 Points");
  display.print("Number of Players: ");
  display.println(numberOfPlayers);
  display.display();
}

void updateLEDsBasedOnScore(int score) {
  if (score <= 10) {
    strip.fill(strip.Color(255, 0, 0));
  } else if (score > 10 && score <= 20) {
    strip.fill(strip.Color(255, 255, 0));
  } else if (score > 20 && score <= 30) {
    strip.fill(strip.Color(0, 255, 0));
  } else {
    strip.fill(strip.Color(0, 0, 255));
  }
  strip.show();
}

void updateLEDsRandomColor() {
  for(int i=0; i<LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(random(0,255), random(0,255), random(0,255)));
  }
  strip.show();
}



void updateGameSelectionScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("SELECT GAME:");
  display.setTextSize(2);
  display.setCursor(0, 20);
  if(selectedGame == GAME_40_POINTS) {
    display.println("40 Points");
  } else {
    display.println("22 Points");
  }
  display.display();
}

void updatePlayerSelectionScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("SELECT PLAYERS:");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(numberOfPlayers);
  display.display();
}                
