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
#define PIN_EGG_BUTTON A0

// Hardware

ezButton all_buttons[] = {
  ezButton(PIN_ADD_BUTTON),
  ezButton(PIN_NEXT_BUTTON),
  ezButton(PIN_FIRE_BUTTON),
  ezButton(PIN_CLEAVER_BUTTON),
  ezButton(PIN_PAN_BUTTON),
  ezButton(PIN_EGG_BUTTON)
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Game data

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
  int8_t* ingredients;
  int8_t num_ingredients;
  int8_t technique;
  int8_t type;
};

const int8_t mushroom_terrine_ingredients[] PROGMEM = {20, 22, 25, 26};
const int8_t scallop_poele_ingredients[] PROGMEM = {5, 48};
const int8_t ratatouille_ingredients[] PROGMEM = {13, 15, 16, 18, 31};
const int8_t lobster_mousse_ingredients[] PROGMEM = {5, 6, 33};
const int8_t caprese_salad_ingredients[] PROGMEM = {53, 15, 44, 32};
const int8_t consomme_ingredients[] PROGMEM = {28, 19, 14};
const int8_t pumpkin_veloute_ingredients[] PROGMEM = {12, 24};
const int8_t carrot_potage_ingredients[] PROGMEM = {17, 14, 50, 47};
const int8_t crab_bisque_ingredients[] PROGMEM = {4, 15, 17, 19, 48};
const int8_t cold_tomato_soup_ingredients[] PROGMEM = {58, 15, 33};
const int8_t sole_meuniere_ingredients[] PROGMEM = {7, 46};
const int8_t eel_matelote_ingredients[] PROGMEM = {8, 31, 34};
const int8_t bouillabaisse_ingredients[] PROGMEM = {10, 6, 11, 18, 35};
const int8_t sea_bass_en_papillote_ingredients[] PROGMEM = {9, 32, 30, 48};
const int8_t seafood_tartare_ingredients[] PROGMEM = {54, 6, 9, 48};
const int8_t sea_bass_poele_ingredients[] PROGMEM = {56, 9, 13, 22, 34};
const int8_t sweetbread_poele_ingredients[] PROGMEM = {0, 38, 21};
const int8_t beef_saute_ingredients[] PROGMEM = {49, 16, 13, 1};
const int8_t beef_bourguignon_ingredients[] PROGMEM = {29, 31, 30, 18, 1};
const int8_t stuffed_quail_ingredients[] PROGMEM = {23, 50, 21, 27, 34, 2};
const int8_t filet_mignon_poele_ingredients[] PROGMEM = {52, 47, 24, 29, 28, 3};
const int8_t chocolate_mousse_ingredients[] PROGMEM = {45, 38, 47};
const int8_t fresh_fruit_compote_ingredients[] PROGMEM = {41, 43, 36};
const int8_t crepes_suzette_ingredients[] PROGMEM = {39, 51, 47};
const int8_t berries_au_fromage_ingredients[] PROGMEM = {44, 38, 37, 40, 42};
const int8_t warm_banana_souffle_ingredients[] PROGMEM = {55, 47, 51};
const int8_t fruit_gelee_ingredients[] PROGMEM = {57, 41, 37};
const int8_t tarte_aux_fruits_ingredients[] PROGMEM = {52, 53, 54, 55, 56, 57, 58};

const recipe recipes[] PROGMEM = {
  { mushroom_terrine_ingredients, 4, 5, 0},
  { scallop_poele_ingredients, 2, 2, 0},
  { ratatouille_ingredients, 5, 4, 0},
  { lobster_mousse_ingredients, 3, 5, 0},
  { caprese_salad_ingredients, 4, 3, 0},
  { consomme_ingredients, 3, 4, 1},
  { pumpkin_veloute_ingredients, 2, 5, 1},
  { carrot_potage_ingredients, 4, 3, 1},
  { crab_bisque_ingredients, 5, 2, 1},
  { cold_tomato_soup_ingredients, 3, 3, 1},
  { sole_meuniere_ingredients, 2, 4, 2},
  { eel_matelote_ingredients, 3, 2, 2},
  { bouillabaisse_ingredients, 5, 3, 2},
  { sea_bass_en_papillote_ingredients, 4, 3, 2},
  { seafood_tartare_ingredients, 4, 3, 2},
  { sea_bass_poele_ingredients, 5, 2, 2},
  { sweetbread_poele_ingredients, 3, 2, 3},
  { beef_saute_ingredients, 4, 3, 3},
  { beef_bourguignon_ingredients, 5, 3, 3},
  { stuffed_quail_ingredients, 6, 4, 3},
  { filet_mignon_poele_ingredients, 6, 2, 3},
  { chocolate_mousse_ingredients, 3, 5, 4},
  { fresh_fruit_compote_ingredients, 3, 3, 4},
  { crepes_suzette_ingredients, 3, 2, 4},
  { berries_au_fromage_ingredients, 5, 5, 4},
  { warm_banana_souffle_ingredients, 3, 5, 4},
  { fruit_gelee_ingredients, 3, 4, 4},
  { tarte_aux_fruits_ingredients, 7, 5, 4},
};

const uint8_t digits[] = {
  7, 1, 4, 3,
  5, 6, 0, 1,
  4, 5, 8, 9,
  0, 9, 2, 7,
  3, 6, 8, 2
};

#define NUM_RECIPES 28
#define SHOWN_INGREDIENTS 10

// Global buffers

char text_buffer[17];
int8_t ingredients_buffer[7];

// Global variables

int8_t available_ingredients[SHOWN_INGREDIENTS];
int8_t selected_ingredients[SHOWN_INGREDIENTS];
int8_t ingredient_index;
int8_t selected_recipe_index;
recipe selected_recipe_val;
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
    selected_ingredients[ingredient_index] = !selected_ingredients[ingredient_index];
    show_current_ingredient();
  }

  if (all_buttons[1].getCount() > 0) {
    ingredient_index = (ingredient_index + 1) % SHOWN_INGREDIENTS;
    show_current_ingredient();
  }

  for (int8_t i = 2; i < 6; i++) {
    if (all_buttons[i].getCount() > 0) {
      uint8_t digit = last_time_digit();
      if (selected_recipe_val.technique == i && selected_ingredients_match() && digit_correct(digit)) {
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
  selected_recipe_index = random(0, NUM_RECIPES);
  memcpy_P(&selected_recipe_val, recipes + selected_recipe_index, sizeof(recipe));
  int8_t num_available = 0;
  for (int8_t i = 0; i < selected_recipe_val.num_ingredients; i++) {
    available_ingredients[num_available++] = pgm_read_byte(selected_recipe_val.ingredients + i);
  }

  while (num_available < SHOWN_INGREDIENTS) {
    num_available = add_random_to_available(num_available);
  }

  for (int8_t i = 0; i < SHOWN_INGREDIENTS; i++) {
    selected_ingredients[i] = false;
  }

  shuffle(available_ingredients, num_available);

  ingredient_index = 0;
  show_current_ingredient();
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

void show_current_ingredient() {
  strcpy_P(text_buffer, (char *)pgm_read_word(&(ingredients[available_ingredients[ingredient_index]])));
  lcd.home();
  lcd.clear();
  lcd.print(text_buffer);
  if (selected_ingredients[ingredient_index]) {
    lcd.setCursor(0, 1);
    lcd.print("Added");
  }
}

int8_t add_random_to_available(int8_t num_available) {
  int8_t new_ingredient = random(0, NUM_INGREDIENTS);
  for (int8_t i = 0; i < num_available; i++) {
    if (available_ingredients[i] == new_ingredient) {
      return num_available;
    }
  }
  available_ingredients[num_available] = new_ingredient;
  if (matching_recipes(num_available + 1) == 1) {
    return num_available + 1;
  } else {
    return num_available;
  }
}

int8_t matching_recipes(int8_t ingredient_count) {
  int8_t count = 0;
  for (int8_t i = 0; i < NUM_RECIPES; i++) {
    int8_t j = 0;
    recipe current;
    memcpy_P(&current, recipes + i, sizeof(recipe));
    while (j < current.num_ingredients && contains(available_ingredients, ingredient_count, pgm_read_byte(current.ingredients + j))) {
      j++;
    }
    if (j == current.num_ingredients) {
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

int8_t selected_ingredients_match() {
  int8_t count = 0;
  memcpy_P(ingredients_buffer, selected_recipe_val.ingredients, selected_recipe_val.num_ingredients);
  for (int8_t i = 0; i < SHOWN_INGREDIENTS; i++) {
    if (selected_ingredients[i] != contains(ingredients_buffer, selected_recipe_val.num_ingredients, available_ingredients[i])) {
      return false;
    }
  }
  return true;
}

int8_t digit_correct(uint8_t digit) {
  return digit == digits[selected_recipe_val.type * 4 + selected_recipe_val.technique - 2];
}

uint8_t last_time_digit() {
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
