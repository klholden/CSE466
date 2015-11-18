
#define TFT_DC  9
#define TFT_CS 10

#include "SPI.h"
#include "ILI9341_t3.h"

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

typedef struct {
  uint16_t red   : 5;
  uint16_t green : 6;
  uint16_t blue  : 5;
} Color;

#define MAIN_MENU 0
#define REACTION_TEST 1
#define COLOR_TEST 2
#define CONTRAST_TEST 3
#define START_SCREEN -1

#define BACKGROUND_COLOR ILI9341_BLACK
#define TEXT_COLOR ILI9341_WHITE
#define START_OPTIONS 3
#define MAX_MENU_IN 1024
#define OFFSET 10
#define MENU_TEXT_SIZE 2.7
#define MENU_TEXT_SPACE (MENU_TEXT_SIZE * 10 + 2)
#define TITLE_OFFSET 130
#define TOP_OFFSET 20

#define WAIT_TIME_MAX 4352
#define WAIT_TIME_MIN 867
#define DOT_COLOR ILI9341_RED

#define MAX_NOISE 50

#define CONTRAST_COUNT 12


uint32_t currentScreen;
uint32_t prevScreen;
uint32_t prevMenuSel;
uint32_t prevButton;

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


char* startOptions[]={"Reaction Time", "Color Blindness", "Color Contrast"};

/***********************************************************************************************************************************/
void setup() {

  tft.begin();
  Serial.begin(9600);

  // in here is specific code for the display. change if you are using a different screen.
  tft.fillScreen(BACKGROUND_COLOR);

  pinMode(14, INPUT_PULLUP);
  // pinMode(17, INPUT_PULLUP);
  currentScreen = MAIN_MENU;
  prevScreen = START_SCREEN;
  prevMenuSel = -1;
  prevButton = 1;
  bestTime = UINT32_MAX;
}

/***********************************************************************************************************************************/


uint32_t mainMenuScreen() {
  // CALCULATE SELECTION
  uint32_t menuSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (START_OPTIONS));
  menuSel = min(menuSel, START_OPTIONS - 1);
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = MAIN_MENU;
    
  if (currentScreen != prevScreen) {
    tft.setCursor(OFFSET * 1.5, TOP_OFFSET + OFFSET + 20);
    tft.setTextColor(DOT_COLOR);
    tft.setTextSize(MENU_TEXT_SIZE + 2);
    tft.print("EYE GAMES");

    tft.setCursor(OFFSET * 1.5, TOP_OFFSET + OFFSET + 60);
    tft.setTextColor(TEXT_COLOR);
    tft.setTextSize(MENU_TEXT_SIZE * .75);
    tft.print("By: Krista Holden");
    
    tft.setTextSize(MENU_TEXT_SIZE);
    for (int i = 0; i < START_OPTIONS; i++) {
      menuPrint(i, i == menuSel);
    }
  } else if (menuSel != prevMenuSel) {
    menuPrint(menuSel, true);
    menuPrint(prevMenuSel, false);
  }

  if (prevButton == 0 && currButton == 1) {
    nextScreen = menuSel + 1;
  }  
  prevMenuSel = menuSel;
  prevButton = currButton;
  return nextScreen;
}

void menuPrint(uint32_t i, bool selected) {
  if (selected) {
    tft.setTextColor(BACKGROUND_COLOR);
    tft.fillRect(OFFSET, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET / 2,
      240 - OFFSET * 2, MENU_TEXT_SIZE * 10, TEXT_COLOR);
  } else {
    tft.setTextColor(TEXT_COLOR);
    tft.fillRect(OFFSET, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET / 2,
      240 - OFFSET * 2, MENU_TEXT_SIZE * 10, BACKGROUND_COLOR);
  }
  tft.setCursor(OFFSET * 2, TOP_OFFSET + TITLE_OFFSET + i * MENU_TEXT_SIZE * 10 + OFFSET);
  tft.print(startOptions[i]);
}

uint32_t reactionTimeScreen() {
  uint32_t nextScreen = REACTION_TEST;
  uint32_t currButton = digitalRead(14);
  
  if (currentScreen != prevScreen) {    
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("REACTION TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Click the button");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("when a red dot");
    tft.setCursor(OFFSET * 2, OFFSET + 60);
    tft.print("appears.");
    prevMenuSel = 0;
    delay(random(WAIT_TIME_MIN, WAIT_TIME_MAX));
    dotLocX = random(40, 180);
    dotLocY = random(100, 200);
    tft.fillCircle(dotLocX, dotLocY, 10, DOT_COLOR);
    startTime = millis();
    prevMenuSel = 0;
  } else {
    if (prevButton == 0 && currButton == 1) {
      prevMenuSel = prevMenuSel + 1;
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
      } else {
        uint32_t reactTime = millis() - startTime;
        tft.setCursor(dotLocX - 30, dotLocY + 20);
        tft.print(reactTime / 1000.0);
        tft.print(" s");
        if (reactTime < bestTime) {
          bestTime = reactTime;
          tft.setCursor(dotLocX - 40, dotLocY + 40);
          tft.print("NEW BEST");
        }
        tft.setCursor(OFFSET * 2, 260);
        tft.print("Click the button");
        tft.setCursor(OFFSET * 2, 280);
        tft.print("again to return");
        tft.setCursor(OFFSET * 2, 300);
        tft.print("home.");
      }
    }
  }
  prevButton = currButton;
  return nextScreen;
}

void printNoiseNumbers(uint16_t color) {
  tft.setTextColor(color);
  for (int i = 0; i < MAX_NOISE; i++) {
    tft.setCursor(random(30, 200), random(80, 250));
    tft.print(random(0, 10));
  }
}

uint32_t colorBlindScreen() {
  uint32_t ansSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (20));
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = COLOR_TEST;
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(MENU_TEXT_SIZE);
    
  if (currentScreen != prevScreen) { 
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("COLOR TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Solve the red");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("addition problem.");
  
    tft.fillRect(OFFSET * 2, 70, 200, 200, *(uint16_t*)&CB_ORANGE);
    tft.setTextSize(MENU_TEXT_SIZE * 1.25);
    printNoiseNumbers(*(uint16_t*)&CB_LGRE);
    // printNoiseNumbers(*(uint16_t*)&CB_ORANGE);
    printNoiseNumbers(*(uint16_t*)&CB_DGRE);

    tft.setTextColor(*(uint16_t*)&CB_RED);
    tft.setTextSize(MENU_TEXT_SIZE * 1.5);
    uint32_t a = random(0, 10);
    uint32_t b = random(0, 10);
    tft.setCursor(random(50, 150), random(70, 260));
    tft.print(a);
    tft.print("+");
    tft.print(b);
    correctAns = a + b;
    
    tft.setTextSize(MENU_TEXT_SIZE);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2, 300);
    tft.print("Your Answer: ");
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
    prevMenuSel = 0;
  }

  if (ansSel != prevAns && prevMenuSel == 0) {
    tft.setTextColor(BACKGROUND_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(prevAns);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
  }

  if (prevButton == 0 && currButton == 1) {
      prevMenuSel = prevMenuSel + 1;
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
      } else {
        // tft.setTextColor(BACKGROUND_COLOR);
        if (ansSel == correctAns) {
          tft.setTextSize(MENU_TEXT_SIZE * 1.5);
          tft.setCursor(40, 150);
          tft.print("CORRECT");
        } else {
          tft.setTextSize(MENU_TEXT_SIZE * 1.2);
          tft.setCursor(20, 150);
          tft.print("INCORRECT:");
          tft.print(correctAns);
        }
        tft.setTextSize(MENU_TEXT_SIZE);
        tft.setTextColor(TEXT_COLOR);
      }
  }
  
  prevAns = ansSel;
  prevButton = currButton;
  return nextScreen;
}



uint32_t contrastTestScreen() {
  uint32_t ansSel = (uint32_t) ((MAX_MENU_IN - analogRead(A3)) / (double)MAX_MENU_IN * (CONTRAST_COUNT));
  uint32_t currButton = digitalRead(14);
  uint32_t nextScreen = CONTRAST_TEST;
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(MENU_TEXT_SIZE);

  if (currentScreen != prevScreen) {
    tft.setCursor(OFFSET * 2, OFFSET);
    tft.print("CONTRAST TEST:");
    tft.setCursor(OFFSET * 2, OFFSET + 20);
    tft.print("Select the darker");
    tft.setCursor(OFFSET * 2, OFFSET + 40);
    tft.print("circle.");

    tft.fillRect(OFFSET * 2, 70, 200, 200, ILI9341_WHITE);

    correctAns = random(0, CONTRAST_COUNT);
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
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2, 300);
    tft.print("Your Answer: ");
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
    
    prevAns = ansSel;
    prevMenuSel = 0;
  }

  if (ansSel != prevAns && prevMenuSel == 0) {
    tft.setTextColor(BACKGROUND_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(prevAns);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(OFFSET * 2 + 170, 300);
    tft.print(ansSel);
  }

  if (prevButton == 0 && currButton == 1) {
      prevMenuSel = prevMenuSel + 1;
      if (prevMenuSel > 1) {
        nextScreen = MAIN_MENU;
      } else {
        tft.setTextColor(BACKGROUND_COLOR);
        if (ansSel == correctAns) {
          tft.setTextSize(MENU_TEXT_SIZE * 1.5);
          tft.setCursor(40, 140);
          tft.print("CORRECT");
        } else {
          tft.setTextSize(MENU_TEXT_SIZE * 1.2);
          tft.setCursor(20, 140);
          tft.print("INCORRECT:");
          tft.print(correctAns);
        }
        tft.setTextSize(MENU_TEXT_SIZE);
        tft.setTextColor(TEXT_COLOR);
      }
  }

  prevAns = ansSel;
  prevButton = currButton;
  return nextScreen;
}

/***********************************************************************************************************************************/
void loop() {
  if (currentScreen != prevScreen) {
    tft.fillScreen(BACKGROUND_COLOR);
  }
  
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
  prevScreen = currentScreen;
  currentScreen = nextScreen;
}


