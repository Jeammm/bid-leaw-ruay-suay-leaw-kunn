#include "card_printer.h"

int no_0_card_x = 0;
int no_0_card_y = 0;
int no_0_card_val_x = 3;
int no_0_card_val_y = 1;

int no_1_card_x = 26;
int no_1_card_y = 0;
int no_1_card_val_x = 29;
int no_1_card_val_y = 1;

int no_2_card_x = 52;
int no_2_card_y = 0;
int no_2_card_val_x = 55;
int no_2_card_val_y = 1;

int no_3_card_x = 78;
int no_3_card_y = 0;
int no_3_card_val_x = 81;
int no_3_card_val_y = 1;

int no_4_card_x = 104;
int no_4_card_y = 0;
int no_4_card_val_x = 107;
int no_4_card_val_y = 1;


void print_card_no(int card_value, int card_icon, int card_no) {

  int loc_card_x;
  int loc_card_y;
  int loc_card_val_x;
  int loc_card_val_y;

  switch(card_no){
    case 0:
      loc_card_x = no_0_card_x;
      loc_card_y = no_0_card_y;
      loc_card_val_x = no_0_card_val_x;
      loc_card_val_y = no_0_card_val_y;
      break;
    case 1:
      loc_card_x = no_1_card_x;
      loc_card_y = no_1_card_y;
      loc_card_val_x = no_1_card_val_x;
      loc_card_val_y = no_1_card_val_y;
      break;
    case 2:
      loc_card_x = no_2_card_x;
      loc_card_y = no_2_card_y;
      loc_card_val_x = no_2_card_val_x;
      loc_card_val_y = no_2_card_val_y;
      break;
    case 3:
      loc_card_x = no_3_card_x;
      loc_card_y = no_3_card_y;
      loc_card_val_x = no_3_card_val_x;
      loc_card_val_y = no_3_card_val_y;
      break;
    case 4:
      loc_card_x = no_4_card_x;
      loc_card_y = no_4_card_y;
      loc_card_val_x = no_4_card_val_x;
      loc_card_val_y = no_4_card_val_y;
      break;
  }

  display.setCursor(loc_card_val_x, loc_card_val_y);
  if (card_value == 0) {
    return;
  } else if (card_value == 1){
    display.print("A");
  } else if (card_value < 11) {
    display.print(card_value);
  } else if (card_value == 11) {
    display.print("J");
  } else if (card_value == 12) {
    display.print("Q");
  } else if (card_value == 13) {
    display.print("K");
  }

  switch(card_icon) {
    case 0:
      display.drawBitmap(
        loc_card_x,
        loc_card_y,
        spade_card_bmp, CARD_WIDTH, CARD_HEIGHT, 1);
      break;
    case 1:
      display.drawBitmap(
        loc_card_x,
        loc_card_y,
        diamond_card_bmp, CARD_WIDTH, CARD_HEIGHT, 1);
      break;
    case 2:
      display.drawBitmap(
        loc_card_x,
        loc_card_y,
        heart_card_bmp, CARD_WIDTH, CARD_HEIGHT, 1);
      break;
    case 3:
      display.drawBitmap(
        loc_card_x,
        loc_card_y,
        club_card_bmp, CARD_WIDTH, CARD_HEIGHT, 1);
      break;
  }
}