
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

enum selection {
  mainMenu,
  reactionTime,
  colorBlind,
  contrastTest
};

#define BACKGROUND_COLOR ILI9341_BLACK
#define TEXT_COLOR ILI9341_WHITE
#define START_OPTIONS 3
#define OFFSET 10
#define MENU_TEXT_SIZE 2.7
#define MENU_TEXT_SPACE (MENU_TEXT_SIZE * 10 + 2)

selection currentScreen;
selection prevScreen;
uint32_t prevMenuSel;

char* startOptions[]={"Reaction Test", "Color Test", "Contrast Test"};

/***********************************************************************************************************************************/
void setup() {

  tft.begin();
  Serial.begin(9600);

  // in here is specific code for the display. change if you are using a different screen.
  tft.fillScreen(BACKGROUND_COLOR);

  pinMode(14, INPUT_PULLUP);
  currentScreen = mainMenu;
  prevScreen = mainMenu;
  prevMenuSel = -1;
}

/***********************************************************************************************************************************/
void loop() {
  if (currentScreen != prevScreen) {
    tft.fillScreen(BACKGROUND_COLOR);
  }
  
  switch (currentScreen) {
    case reactionTime:
      reactionTimeScreen();
      break;
    case colorBlind:
      colorBlindScreen();
      break;
    case contrastTest:
      contrastTestScreen();
      break;
    default:
      mainMenuScreen();
  }
  prevScreen = currentScreen;
}
/***********************************************************************************************************************************/

void mainMenuScreen() {
  uint32_t menuSel = 0; // CALCULATE SELECTION
  if (currentScreen != prevScreen) {

  }
  uint32_t menuSel = 0;
  if (menuSel != prevMenuSel) {
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(MENU_TEXT_SIZE);
  for (int i = 0; i < START_OPTIONS; i++) {
    tft.setTextColor(TEXT_COLOR);
    if (i == menuSel && (menuSel != prevMenuSel || ) {
      tft.setTextColor(BACKGROUND_COLOR);
      tft.fillRect(OFFSET, i * MENU_TEXT_SIZE * 10 + OFFSET / 2, 240 - OFFSET * 2, MENU_TEXT_SIZE * 10, TEXT_COLOR);
    }
    tft.setCursor(OFFSET * 2, i * MENU_TEXT_SIZE * 10 + OFFSET);
    tft.print(startOptions[i]);
  }
  prevMenuSel = menuSel;
}


void reactionTimeScreen() {
  
}

void colorBlindScreen() {
  
}

void contrastTestScreen() {
  
}

