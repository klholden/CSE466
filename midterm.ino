
#define TFT_DC  9
#define TFT_CS 10

#include "SPI.h"
#include "ILI9341_t3.h"

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

// Struct to represent RGB colors
typedef struct {
  uint16_t red   : 5;
  uint16_t green : 6;
  uint16_t blue  : 5;
} Color;

// Values to represent the different screens (act like enums)
#define MAIN_MENU 0
#define REACTION_TEST 1
#define COLOR_TEST 2
#define CONTRAST_TEST 3
#define START_SCREEN -1

// General UI Values
#define BACKGROUND_COLOR ILI9341_BLACK
#define TEXT_COLOR ILI9341_WHITE
#define START_OPTIONS 3
#define MAX_MENU_IN 1024
#define OFFSET 10
#define MENU_TEXT_SIZE 2.7
#define MENU_TEXT_SPACE (MENU_TEXT_SIZE * 10 + 2)
#define TITLE_OFFSET 130
#define TOP_OFFSET 20

// Values for the reaction time test delineating the boundaries of the random timing
#define WAIT_TIME_MAX 4352
#define WAIT_TIME_MIN 867

// The color of the dot in the reaction time test
#define DOT_COLOR ILI9341_RED

// Value to represent the number of supurfluous numbers in the color blindness test
#define MAX_NOISE 50

// Value corresponding to the number of dots in the color blindness test
#define CONTRAST_COUNT 12

// Values to track UI values
uint32_t currentScreen; // Current displayed screen
uint32_t prevScreen; // Previously displayed
uint32_t prevMenuSel; // Previously selected menu item
uint32_t prevButton; // Previous button value

// Values for Reaction Test
uint32_t startTime;
uint32_t dotLocX;
uint32_t dotLocY;
uint32_t bestTime;

// Values for Color Blindness Test
Color CB_ORANGE = {0, 13, 10};
Color CB_LGRE = {5, 25, 5};
Color CB_RED = {3, 10, 10};
Color CB_DGRE = {5, 20, 5};
uint32_t correctAns;
uint32_t prevAns;

// Values for the Contrast Test
Color CT_LGREY = {12, 12, 12};
Color CT_DGREY = {10, 10, 10};

// Main Menu Options
char* startOptions[]={"Reaction Time", "Color Blindness", "Color Contrast"};

/***********************************************************************************************************************************/
void setup() {

  tft.begin();
  Serial.begin(9600);

  // Fills the screen with the set background color
  tft.fillScreen(BACKGROUND_COLOR);

  // Identifies pin 14 as analog
  pinMode(14, INPUT_PULLUP);
  
  // Initializes values
  currentScreen = MAIN_MENU;
  prevScreen = START_SCREEN;
  prevMenuSel = -1;
  prevButton = 1;
  bestTime = UINT32_MAX;
}

/***********************************************************************************************************************************/

// Method to print and manage the main menu screen
uint32_t mainMenuScreen() {
  // Calculates the dial entry value
  uint32_t menuSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (START_OPTIONS));
  
  // Maxes menu selection out with the last option
  menuSel = min(menuSel, START_OPTIONS - 1);
  
  // Initializes the current button and next screen values
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = MAIN_MENU;

  // If this is a new screen print everything, else only print changes    
  if (currentScreen != prevScreen) {
    // Print the title: Eye Games
    tft.setCursor(OFFSET * 1.5, TOP_OFFSET + OFFSET + 20);
    tft.setTextColor(DOT_COLOR);
    tft.setTextSize(MENU_TEXT_SIZE + 2);
    tft.print("EYE GAMES");

    // Print my name
    tft.setCursor(OFFSET * 1.5, TOP_OFFSET + OFFSET + 60);
    tft.setTextColor(TEXT_COLOR);
    tft.setTextSize(MENU_TEXT_SIZE * .75);
    tft.print("By: Krista Holden");
    
    // Print all menu options (and selected option as selected)
    tft.setTextSize(MENU_TEXT_SIZE);
    for (int i = 0; i < START_OPTIONS; i++) {
      menuPrint(i, i == menuSel);
    }
  } else if (menuSel != prevMenuSel) {
    // Only print changes so unselect the previous selection
    // and select the new selection
    menuPrint(menuSel, true);
    menuPrint(prevMenuSel, false);
  }

  // If a button was pressed move to that screen
  if (prevButton == 0 && currButton == 1) {
    nextScreen = menuSel + 1;
  }
  
  // Set previous values and move to the next screen
  prevMenuSel = menuSel;
  prevButton = currButton;
  return nextScreen;
}

// Helper method to print the menu value
void menuPrint(uint32_t i, bool selected) {
  // Prints the given menu item at position i either in a selected or unselected manner
  if (selected) {
    // If it is selected print a box to highlight the text and print the text in the background color
    tft.setTextColor(BACKGROUND_COLOR);
    tft.fillRect(OFFSET, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET / 2,
      240 - OFFSET * 2, MENU_TEXT_SIZE * 10, TEXT_COLOR);
  } else {
    // If it is not selected print a box to cover the prev selection and print the text normally
    tft.setTextColor(TEXT_COLOR);
    tft.fillRect(OFFSET, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET / 2,
      240 - OFFSET * 2, MENU_TEXT_SIZE * 10, BACKGROUND_COLOR);
  }
  tft.setCursor(OFFSET * 2, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET);
  tft.print(startOptions[i]);
}

// Method to print and manage the reaction time test
uint32_t reactionTimeScreen() {
  // Initialize temporary variables
  uint32_t nextScreen = REACTION_TEST;
  uint32_t currButton = digitalRead(14);
  
  // If this is a new screen print everything
  if (currentScreen != prevScreen) {
    // Print the title and instructions
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("REACTION TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Click the button");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("when a red dot");
    tft.setCursor(OFFSET * 2, OFFSET + 60);
    tft.print("appears.");
    
    // Set up the waiting for a button press
    delay(random(WAIT_TIME_MIN, WAIT_TIME_MAX));
    
    // Set up and print the randomly-located red dot
    dotLocX = random(40, 180);
    dotLocY = random(100, 200);
    tft.fillCircle(dotLocX, dotLocY, 10, DOT_COLOR);
    
    // Start timing
    startTime = millis();
    prevMenuSel = 0;
  } else {
    // Check for a button press
    if (prevButton == 0 && currButton == 1) {
      // Add 1 to the number of button presses previously seen
      prevMenuSel = prevMenuSel + 1;
      
      // If there have been two then set the next screen to main menu
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
        
      // Else record the time it took to react and print results
      } else {
        uint32_t reactTime = millis() - startTime;
        tft.setCursor(dotLocX - 30, dotLocY + 20);
        tft.print(reactTime / 1000.0);
        tft.print(" s");
        
        // If this time is better then the best record it and print
        if (reactTime < bestTime) {
          bestTime = reactTime;
          tft.setCursor(dotLocX - 40, dotLocY + 40);
          tft.print("NEW BEST");
        }
        
        // Print exit instructions
        tft.setCursor(OFFSET * 2, 260);
        tft.print("Click the button");
        tft.setCursor(OFFSET * 2, 280);
        tft.print("again to return");
        tft.setCursor(OFFSET * 2, 300);
        tft.print("home.");
      }
    }
  }
  
  // Assign previous variables to their new values and return
  prevButton = currButton;
  return nextScreen;
}

// Helper method for the color blind test to print numbers in a given color
void printNoiseNumbers(uint16_t color) {
  tft.setTextColor(color);
  for (int i = 0; i < MAX_NOISE; i++) {
    // Set the curser in a random location in the testing box
    tft.setCursor(random(30, 200), random(80, 250));
    // Print a random single digit number
    tft.print(random(0, 10));
  }
}

// Method to print and manage the color blindness test
uint32_t colorBlindScreen() {
  // Initialize temporary variables to track the user's answer, button presses, and the next screen
  uint32_t ansSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (20));
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = COLOR_TEST;
  
  // Reset text printing values to the default
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(MENU_TEXT_SIZE);
  
  // If this screen is new then print all original values
  if (currentScreen != prevScreen) {
    // Print the isntructions
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("COLOR TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Solve the red");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("addition problem.");
  
    // Print the background and noise numbers
    tft.fillRect(OFFSET * 2, 70, 200, 200, *(uint16_t*)&CB_ORANGE);
    tft.setTextSize(MENU_TEXT_SIZE * 1.25);
    printNoiseNumbers(*(uint16_t*)&CB_LGRE);
    printNoiseNumbers(*(uint16_t*)&CB_DGRE);

    // Print the randomly generated addition equation and store correct answer
    tft.setTextColor(*(uint16_t*)&CB_RED);
    tft.setTextSize(MENU_TEXT_SIZE * 1.5);
    uint32_t a = random(0, 10);
    uint32_t b = random(0, 10);
    tft.setCursor(random(50, 150), random(70, 260));
    tft.print(a);
    tft.print("+");
    tft.print(b);
    correctAns = a + b;
    
    // Print the answer selection values
    tft.setTextSize(MENU_TEXT_SIZE);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2, 300);
    tft.print("Your Answer: ");
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
    prevMenuSel = 0;
  }

  // If a button is not pressed yet and the user's anser changes
  // cover the old answer and print a new one
  if (ansSel != prevAns && prevMenuSel == 0) {
    tft.setTextColor(BACKGROUND_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(prevAns);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
  }

  // If a button is pressed check the result or exit to the menu
  if (prevButton == 0 && currButton == 1) {
      // Add 1 to the number of times the button had been pressed
      prevMenuSel = prevMenuSel + 1;
      
      // If it has already been pressed set the next screen to main menu
      // and continue
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
        
      // If the button has not been pressed check the given answer and
      // print its correctness.
      } else {
        // If it is correct print correct
        if (ansSel == correctAns) {
          tft.setTextSize(MENU_TEXT_SIZE * 1.5);
          tft.setCursor(40, 150);
          tft.print("CORRECT");
          
        // If it is incorrect print that and the actual answer
        } else {
          tft.setTextSize(MENU_TEXT_SIZE * 1.2);
          tft.setCursor(20, 150);
          tft.print("INCORRECT:");
          tft.print(correctAns);
        }
        
        // Reset the color and size of text to default
        tft.setTextSize(MENU_TEXT_SIZE);
        tft.setTextColor(TEXT_COLOR);
      }
  }
  
  // Set the previous values and return the next screen value
  prevAns = ansSel;
  prevButton = currButton;
  return nextScreen;
}


// Method to print and manage the color contrast test
uint32_t contrastTestScreen() {
  // Initialize temporary variable to moniter the user's input and the next screen
  uint32_t ansSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (CONTRAST_COUNT));
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = CONTRAST_TEST;
  
  // Reset the text color and size values
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(MENU_TEXT_SIZE);

  // If this is a new screen print everything
  if (currentScreen != prevScreen) {
    // Print title and instructions
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("CONTRAST TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Select the darker");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("circle.");

    // Print the white square background
    tft.fillRect(OFFSET * 2, 70, 200, 200, ILI9341_WHITE);

    // Pick a random circle to be darker
    correctAns = random(0, CONTRAST_COUNT);
    
    // Print all circles their correct color and id number in rows of 4
    tft.setTextColor(ILI9341_BLACK);
    for (int i = 0; i < CONTRAST_COUNT; i++) {
      Color cirColor = CT_LGREY;
      if (i == correctAns) {
        cirColor = CT_DGREY;
      }
      uint32_t posX = 55 + 41 * (i % 4);
      uint32_t posY = 100 + 60 * (i / 4);
      tft.fillCircle(posX, posY, 15, *(uint16_t*)&cirColor);
      
      tft.setCursor(posX - 5 * (1 + i / 10), posY + 20);
      tft.print(i);
    }
    
    // Print the user's input selection
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2, 300);
    tft.print("Your Answer: ");
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
    
    // Reset selection temp values
    prevAns = ansSel;
    prevMenuSel = 0;
  }

  // If the user's selection input changes overwrite the old value and
  // print the new one
  if (ansSel != prevAns && prevMenuSel == 0) {
    tft.setTextColor(BACKGROUND_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(prevAns);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
  }

  // If a user presses the button handle it
  if (prevButton == 0 && currButton == 1) {
      // Add one to the number of presses
      prevMenuSel = prevMenuSel + 1;
      
      // If this is the second press then set the next screen to the main menu
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
        
      // If this is the first press check the user's submission
      } else {
        tft.setTextColor(BACKGROUND_COLOR);
        
        // If it is correct print correct
        if (ansSel == correctAns) {
          tft.setTextSize(MENU_TEXT_SIZE * 1.5);
          tft.setCursor(40, 140);
          tft.print("CORRECT");
        
        // If it is incorrect print that and the correct answer
        } else {
          tft.setTextSize(MENU_TEXT_SIZE * 1.2);
          tft.setCursor(20, 140);
          tft.print("INCORRECT:");
          tft.print(correctAns);
        }
        
        // Reset the text size and color to the defaults
        tft.setTextSize(MENU_TEXT_SIZE);
        tft.setTextColor(TEXT_COLOR);
      }
  }

  // Set previous values to the new values and return the next screen
  prevAns = ansSel;
  prevButton = currButton;
  return nextScreen;
}

/***********************************************************************************************************************************/
void loop() {
  // If the screen changes reset the display by printing over it all
  if (currentScreen != prevScreen) {
    tft.fillScreen(BACKGROUND_COLOR);
  }
  
  // Print out the current screen and set the next screen
  uint32_t nextScreen;
  switch (currentScreen) {
    case REACTION_TEST:
      nextScreen = reactionTimeScreen();
      break;
    case COLOR_TEST:
      nextScreen = colorBlindScreen();
      break;
    case CONTRAST_TEST:
      nextScreen = contrastTestScreen();
      break;
    default:
      nextScreen = mainMenuScreen();
  }
  
  // Shift variables of current to previous and next to current for screen selection
  prevScreen = currentScreen;
  currentScreen = nextScreen;
}


