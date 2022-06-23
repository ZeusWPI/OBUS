// (c) 2022, chvp
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <ezButton.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

#define PIN_ADD_BUTTON 2
#define PIN_NEXT_BUTTON 3
#define PIN_FIRE_BUTTON 5
#define PIN_CLEAVER_BUTTON 6
#define PIN_PAN_BUTTON 9
#define PIN_EGG_BUTTON 10

ezButton all_buttons[] = {
  ezButton(PIN_ADD_BUTTON),
  ezButton(PIN_NEXT_BUTTON),
  ezButton(PIN_FIRE_BUTTON),
  ezButton(PIN_CLEAVER_BUTTON),
  ezButton(PIN_PAN_BUTTON),
  ezButton(PIN_EGG_BUTTON)
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char veal[] PROGMEM = "Veal";
const char beef[] PROGMEM = "Beef";
const char quail[] PROGMEM = "Quail";
const char filet_mignon[] PROGMEM = "Filet Mignon";
const char crab[] PROGMEM = "Crab";
const char scallop[] PROGMEM = "Scallop";
const char lobster[] PROGMEM = "Lobster";
const char sole[] PROGMEM = "Sole";
const char eel[] PROGMEM = "Eel";
const char sea_bass[] PROGMEM = "Sea Bass";
const char mussel[] PROGMEM = "Mussel";
const char cod[] PROGMEM = "Cod";
const char pumpkin[] PROGMEM = "Pumpkin";
const char zucchini[] PROGMEM = "Zucchini";
const char onion[] PROGMEM = "Onion";
const char tomato[] PROGMEM = "Tomato";
const char eggplant[] PROGMEM = "Eggplant";
const char carrot[] PROGMEM = "Carrot";
const char garlic[] PROGMEM = "Garlic";
const char celery[] PROGMEM = "Celery";
const char morel[] PROGMEM = "Morel";
const char porcini[] PROGMEM = "Porcini";
const char chantarelle[] PROGMEM = "Chantarelle";
const char portobello[] PROGMEM = "Portobello";
const char black_truffle[] PROGMEM = "Black Truffle";
const char oyster_mushroom[] PROGMEM = "Oyster Mushroom";
const char black_trumpet[] PROGMEM = "Black Trumpet";
const char miller_mushroom[] PROGMEM = "Miller Mushroom";
const char cloves[] PROGMEM = "Cloves";
const char rosemary[] PROGMEM = "Rosemary";
const char thyme[] PROGMEM = "Thyme";
const char bay_leaf[] PROGMEM = "Bay Leaf";
const char basil[] PROGMEM = "Basil";
const char dill[] PROGMEM = "Dill";
const char parsley[] PROGMEM = "Parsley";
const char saffron[] PROGMEM = "Saffron";
const char apricot[] PROGMEM = "Apricot";
const char gooseberry[] PROGMEM = "Gooseberry";
const char lemon[] PROGMEM = "Lemon";
const char orange[] PROGMEM = "Orange";
const char raspberry[] PROGMEM = "Raspberry";
const char pear[] PROGMEM = "Pear";
const char blackberry[] PROGMEM = "Blackberry";
const char apple[] PROGMEM = "Apple";
const char cheese[] PROGMEM = "Cheese";
const char chocolate[] PROGMEM = "Chocolate";
const char caviar[] PROGMEM = "Caviar";
const char butter[] PROGMEM = "Butter";
const char olive_oil[] PROGMEM = "Olive Oil";
const char cornichon[] PROGMEM = "Cornichon";
const char rice[] PROGMEM = "Rice";
const char honey[] PROGMEM = "Honey";
const char sour_cherry[] PROGMEM = "Sour Cherry";
const char strawberry[] PROGMEM = "Strawberry";
const char blood_orange[] PROGMEM = "Blood Orange";
const char banana[] PROGMEM = "Banana";
const char grapes[] PROGMEM = "Grapes";
const char melon[] PROGMEM = "Melon";
const char watermelon[] PROGMEM = "Watermelon";

const char *const ingredients[] PROGMEM = {
  /* 0 */ veal, /* 1 */ beef, /* 2 */ quail, /* 3 */ filet_mignon,
  /* 4 */ crab, /* 5 */ scallop, /* 6 */ lobster, /* 7 */ sole, /* 8 */ eel, /* 9 */ sea_bass, /* 10 */ mussel, /* 11 */ cod,
  /* 12 */ pumpkin, /* 13 */ zucchini, /* 14 */ onion, /* 15 */ tomato, /* 16 */ eggplant, /* 17 */ carrot, /* 18 */ garlic, /* 19 */ celery,
  /* 20 */ morel, /* 21 */ porcini, /* 22 */ chantarelle, /* 23 */ portobello, /* 24 */ black_truffle, /* 25 */ oyster_mushroom, /* 26 */ black_trumpet, /* 27 */ miller_mushroom,
  /* 28 */ cloves, /* 29 */ rosemary, /* 30 */ thyme, /* 31 */ bay_leaf, /* 32 */ basil, /* 33 */ dill, /* 34 */ parsley, /* 35 */ saffron,
  /* 36 */ apricot, /* 37 */ gooseberry, /* 38 */ lemon, /* 39 */ orange, /* 40 */ raspberry, /* 41 */ pear, /* 42 */ blackberry, /* 43 */ apple,
  /* 44 */ cheese, /* 45 */ chocolate, /* 46 */ caviar, /* 47 */ butter, /* 48 */ olive_oil, /* 49 */ cornichon, /* 50 */ rice, /* 51 */ honey,
  /* 52 */ sour_cherry, /* 53 */ strawberry, /* 54 */ blood_orange, /* 55 */ banana, /* 56 */ grapes, /* 57 */ melon, /* 58 */ watermelon
};

#define NUM_INGREDIENTS 59

struct recipe {
  const int8_t* ingredients;
  const int8_t numIngredients;
  const int8_t technique;
  const int8_t type;
};

const int8_t mushroom_terrine[] PROGMEM = {20, 22, 25, 26};
const int8_t scallop_poele[] PROGMEM = {5, 48};
const int8_t ratatouille[] PROGMEM = {13, 15, 16, 18, 31};
const int8_t lobster_mousse[] PROGMEM = {5, 6, 33};
const int8_t caprese_salad[] PROGMEM = {53, 15, 44, 32};
const int8_t consomme[] PROGMEM = {28, 19, 14};
const int8_t pumpkin_veloute[] PROGMEM = {12, 24};
const int8_t carrot_potage[] PROGMEM = {17, 14, 50, 47};
const int8_t crab_bisque[] PROGMEM = {4, 15, 17, 19, 48};
const int8_t cold_tomato_soup[] PROGMEM = {58, 15, 33};
const int8_t sole_meuniere[] PROGMEM = {7, 46};
const int8_t eel_matelote[] PROGMEM = {8, 31, 34};
const int8_t bouillabaisse[] PROGMEM = {10, 6, 11, 18, 35};
const int8_t sea_bass_en_papillote[] PROGMEM = {9, 32, 30, 48};
const int8_t seafood_tartare[] PROGMEM = {54, 6, 9, 48};
const int8_t sea_bass_poele[] PROGMEM = {56, 9, 13, 22, 34};
const int8_t sweetbread_poele[] PROGMEM = {0, 38, 21};
const int8_t beef_saute[] PROGMEM = {49, 16, 13, 1};
const int8_t beef_bourguignon[] PROGMEM = {29, 31, 30, 18, 1};
const int8_t stuffed_quail[] PROGMEM = {23, 50, 21, 27, 34, 2};
const int8_t filet_mignon_poele[] PROGMEM = {52, 47, 24, 29, 28, 3};
const int8_t chocolate_mousse[] PROGMEM = {45, 38, 47};
const int8_t fresh_fruit_compote[] PROGMEM = {41, 43, 36};
const int8_t crepes_suzette[] PROGMEM = {39, 51, 47};
const int8_t berries_au_fromage[] PROGMEM = {44, 38, 37, 40, 42};
const int8_t warm_banana_souffle[] PROGMEM = {55, 47, 51};
const int8_t fruit_gelee[] PROGMEM = {57, 41, 37};
const int8_t tarte_aux_fruits[] PROGMEM = {52, 53, 54, 55, 56, 57, 58};

const recipe recipes[] = {
  { mushroom_terrine, 4, 5, 0},
  { scallop_poele, 2, 2, 0},
  { ratatouille, 5, 4, 0},
  { lobster_mousse, 3, 5, 0},
  { caprese_salad, 4, 3, 0},
  { consomme, 3, 4, 1},
  { pumpkin_veloute, 2, 5, 1},
  { carrot_potage, 4, 3, 1},
  { crab_bisque, 5, 2, 1},
  { cold_tomato_soup, 3, 3, 1},
  { sole_meuniere, 2, 4, 2},
  { eel_matelote, 3, 2, 2},
  { bouillabaisse, 5, 3, 2},
  { sea_bass_en_papillote, 4, 3, 2},
  { seafood_tartare, 4, 3, 2},
  { sea_bass_poele, 5, 2, 2},
  { sweetbread_poele, 3, 2, 3},
  { beef_saute, 4, 3, 3},
  { beef_bourguignon, 5, 3, 3},
  { stuffed_quail, 6, 4, 3},
  { filet_mignon_poele, 6, 2, 3},
  { chocolate_mousse, 3, 5, 4},
  { fresh_fruit_compote, 3, 3, 4},
  { crepes_suzette, 3, 2, 4},
  { berries_au_fromage, 5, 5, 4},
  { warm_banana_souffle, 3, 5, 4},
  { fruit_gelee, 3, 4, 4},
  { tarte_aux_fruits, 7, 5, 4},
};

#define NUM_RECIPES 28
#define SHOWN_INGREDIENTS 10

char buffer[17];
int8_t available_ingredients[SHOWN_INGREDIENTS];
int8_t selected_ingredients[SHOWN_INGREDIENTS];
int8_t ingredientIndex;
int8_t selectedRecipe;
uint32_t last_time_left;
uint32_t millis_at_last_update;

void setup() {
  Serial.begin(115200);
  for (int8_t i = 0; i < 6; i++) {
    all_buttons[i].setDebounceTime(100);
  }
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for next");
  lcd.setCursor(0, 1);
  lcd.print("game...");

  obus_module::setup(OBUS_TYPE_PUZZLE, 6);
}

obus_can::message message;

void loop() {
  bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop, callback_info, callback_state);
  for (int8_t i = 0; i < 6; i++) {
    all_buttons[i].loop();
  }

  if (all_buttons[0].getCount() > 0) {
    selected_ingredients[ingredientIndex] = !selected_ingredients[ingredientIndex];
    showCurrentIngredient();
  }

  if (all_buttons[1].getCount() > 0) {
    ingredientIndex = (ingredientIndex + 1) % SHOWN_INGREDIENTS;
    showCurrentIngredient();
  }

  for (int8_t i = 2; i < 6; i++) {
    if (all_buttons[i].getCount() > 0) {
      uint8_t digit = lastTimeDigit();
      Serial.print(selectedRecipe);
      Serial.print(digit);
      Serial.print(selectedIngredientsMatch());
      if (recipes[selectedRecipe].technique == i && selectedIngredientsMatch() && digitCorrect(digit)) {
        obus_module::solve();
      } else {
        obus_module::strike();
      }
    }
  }

  for (int8_t i = 0; i < 6; i++) {
    all_buttons[i].resetCount();
  }
}

void callback_game_start(uint8_t puzzle_modules) {
  selectedRecipe = random(0, NUM_RECIPES);
  int8_t* recipe_ingredients = (int8_t*) pgm_read_word(&(recipes[selectedRecipe].ingredients));
  int8_t numAvailable = 0;
  for (int8_t i = 0; i < recipes[selectedRecipe].numIngredients; i++) {
    available_ingredients[numAvailable++] = recipe_ingredients[i];
  }

  while (numAvailable < SHOWN_INGREDIENTS) {
    numAvailable = addRandomToAvailable(numAvailable);
  }

  for (int8_t i = 0; i < SHOWN_INGREDIENTS; i++) {
    selected_ingredients[i] = false;
  }

  shuffle(available_ingredients, numAvailable);

  ingredientIndex = 0;
  showCurrentIngredient();
}

void callback_game_stop() {
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for next");
  lcd.setCursor(0, 1);
  lcd.print("game...");
}

void callback_state(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {
  (void)strikes;
  (void)max_strikes;
  (void)puzzle_modules_left;

  millis_at_last_update = millis();
  last_time_left = time_left;
}

void callback_info(uint8_t info_id, uint8_t infomessage[7]) {}

void showCurrentIngredient() {
  strcpy_P(buffer, (char *)pgm_read_word(&(ingredients[available_ingredients[ingredientIndex]])));
  lcd.home();
  lcd.clear();
  lcd.print(buffer);
  if (selected_ingredients[ingredientIndex]) {
    lcd.setCursor(0, 1);
    lcd.print("Added");
  }
}

int8_t addRandomToAvailable(int8_t numAvailable) {
  int8_t newIngredient = random(0, NUM_INGREDIENTS);
  for (int8_t i = 0; i < numAvailable; i++) {
    if (available_ingredients[i] == newIngredient) {
      return numAvailable;
    }
  }
  available_ingredients[numAvailable] = newIngredient;
  if (matchingRecipes(numAvailable + 1) == 1) {
    return numAvailable + 1;
  } else {
    return numAvailable;
  }
}

int8_t matchingRecipes(int8_t ingredientCount) {
  int8_t count = 0;
  for (int8_t i = 0; i < NUM_RECIPES; i++) {
    recipe current = recipes[i];
    int8_t j = 0;
    int8_t* recipe_ingredients = (int8_t *) pgm_read_word(&(current.ingredients));
    while (j < current.numIngredients && contains(available_ingredients, ingredientCount, recipe_ingredients[j])) {
      j++;
    }
    if (j == current.numIngredients) {
      count++;
    }
  }
  return count;
}

void shuffle(int8_t * arr, int8_t size) {
  for (int8_t i = size - 1; i > 1; i--) {
    int8_t j = random(0, i);
    int8_t temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

int8_t selectedIngredientsMatch() {
  int8_t count = 0;
  recipe r = recipes[selectedRecipe];
  int8_t* recipe_ingredients = (int8_t *) pgm_read_word(&(r.ingredients));
  for (int8_t i = 0; i < SHOWN_INGREDIENTS; i++) {
    if (selected_ingredients[i] != contains(recipe_ingredients, r.numIngredients, available_ingredients[i])) {
      return false;
    }
  }
  return true;
}

const uint8_t digits[] = {
  7, 1, 4, 3,
  5, 6, 0, 1,
  4, 5, 8, 9,
  0, 9, 2, 7,
  3, 6, 8, 2
};

int8_t digitCorrect(uint8_t digit) {
  recipe r = recipes[selectedRecipe];
  return digit == digits[r.type * 4 + r.technique - 2];
}

uint8_t lastTimeDigit() {
  return (uint8_t) (((last_time_left - (millis() - millis_at_last_update)) / 1000) % 10);
}

int8_t contains(int8_t * arr, int8_t len, int8_t elem) {
  for (int8_t i = 0; i < len; i++) {
    if (arr[i] == elem) {
      return true;
    }
  }
  return false;
}
