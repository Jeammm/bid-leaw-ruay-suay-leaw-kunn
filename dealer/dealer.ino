#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "card_icon.h"
#include "card_printer.h"

#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

int currentState = 0;

//MAC Address
uint8_t broadcastAddress1[] = {0x3C,0x71,0xBF,0x10,0x5C,0x3C} ;
uint8_t broadcastAddress2[] = {0x3C,0x61,0x05,0x04,0x3F,0x18} ;

esp_now_peer_info_t peerInfo;

const char* suits[]= { "Spades", "Hearts", "Diamonds", "Clubs" };

const char* ranks[]= { "0", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

//game state variables
bool player1_ready = false;
bool player2_ready = false;
bool player1_bet = false;
bool player2_bet = false;

int player1_bet_amount = 0;
int player2_bet_amount = 0;

int player1_count = 0;
int player2_count = 0;

bool player1_stand = false;
bool player2_stand = false;

int dealerSum = 0;
int countCardDealer = 0;

int player1Sum = 0;
int player2Sum = 0;

typedef struct game_state_message {
  int state;
  int id;
  char message[32];
  bool is_ready; // use in init and bet state
  int bet_amount;
  bool hit;
  int WithdrawCredit;
} game_state_message;

typedef struct dealer_message {
  char message[32]; //message
  int player_state;  //state number 0 = init , 1=bet , 2 = playing, 3 = waiting
  int player1_result; //result number 0 = lose, 1 = win, 2 = draw
  int player2_result; //result number 0 = lose, 1 = win, 2 = draw
  int player1_card[5];
  int player2_card[5];
  int dealer_card[5];
  int FromWho; //if 0 = Dealer, 1 = Coin master 
  int DepositCredit;
} dealer_message;

game_state_message gameStateMessage;
dealer_message dealerMessage;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&gameStateMessage, incomingData, sizeof(gameStateMessage));

  if (gameStateMessage.state == 0) {
    if (gameStateMessage.id == 1) {
      player1_ready = gameStateMessage.is_ready;
    } else {
      player2_ready = gameStateMessage.is_ready;
    }
  } 
  else if (gameStateMessage.state == 1) {
    if (gameStateMessage.id == 1) {
      player1_bet = gameStateMessage.is_ready;
      player1_bet_amount = gameStateMessage.bet_amount;
    } else {
      player2_bet = gameStateMessage.is_ready;
      player2_bet_amount = gameStateMessage.bet_amount;
    }
  } 
  else if (gameStateMessage.state == 2) {
    if(gameStateMessage.hit) {
      if(gameStateMessage.id == 1 && player1_count < 5) {
        dealerMessage.player1_card[player1_count++] = random(1,14);
      }  else if (gameStateMessage.id == 2 && player2_count < 5) {
        dealerMessage.player2_card[player2_count++] = random(1,14);
      }
    } else {
      if(gameStateMessage.id == 1) {
        player1_stand = true;
      }  else {
        player2_stand = true;
      }
    }
    if(player1_count == 5) {
      player1_stand = true;
    }
    if(player2_count == 5) {
      player2_stand = true;
    }
  } else if (gameStateMessage.state == 3) {
    //
  }
}

void SendStateToPlayer1() {
  dealerMessage.player_state = currentState;
  esp_err_t result1 = esp_now_send(broadcastAddress1, (uint8_t *) &dealerMessage, sizeof(dealerMessage));
  while(result1 != ESP_OK)
  {
    result1 = esp_now_send(broadcastAddress1, (uint8_t *) &dealerMessage, sizeof(dealerMessage));
  }
  return;
}

void SendStateToPlayer2() {
  dealerMessage.player_state = currentState;
  esp_err_t result2 = esp_now_send(broadcastAddress2, (uint8_t *) &dealerMessage, sizeof(dealerMessage));
  while(result2 != ESP_OK)
  {
    result2 = esp_now_send(broadcastAddress2, (uint8_t *) &dealerMessage, sizeof(dealerMessage));
  }
  return;
}

void handlePlayerReadyState();
void handlePlayerBetState();
void handlePlayerPlayState();
void handleDealerPlayState();

void IdleDisplay();
void IconDisplayTest();
void WaitingForPlayerDisplay();
void WaitingForBetsDisplay();
void DealerCardDisplay();
void DealerPlayDisplay();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  display.clearDisplay();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializeing ESP-NOW");
    return;
  }

  //Register send callback
  esp_now_register_send_cb(OnDataSent);
  
  //Register receive callback
  esp_now_register_recv_cb(OnDataRecv);

  //Register peer
  memset(peerInfo.peer_addr, 0, sizeof(peerInfo));
  for(int i=0; i<6; ++i) {
    peerInfo.peer_addr[i] = (uint8_t) broadcastAddress1[i];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  //Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fail to add peer");
    return;
  }

  for(int i=0; i<6; ++i) {
    peerInfo.peer_addr[i] = (uint8_t) broadcastAddress2[i];
  }
  //Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fail to add peer");
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // StateDisplay();
  switch(currentState) {
    case 0:
      handlePlayerReadyState();
      break;
    case 1:
      handlePlayerBetState();
      break;
    case 2:
      handlePlayerPlayState();
      break;
    case 3:
      handleDealerPlayState();
      break;
    }
}

void IdleDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.print("          ");

  display.setCursor(2,0);
  display.print("BLACK JACK");

  IconDisplayTest();

  display.display();
}

void IconDisplayTest() {
  display.setTextColor(WHITE);
  display.drawBitmap(
    8,
    17,
    heart_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.drawBitmap(
    40,
    17,
    spade_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.drawBitmap(
    72,
    17,
    club_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.drawBitmap(
    104,
    17,
    diamond_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
}

void WaitingForPlayerDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.print("Starting..");
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(5,16);
  display.print("Player 1");
  display.setCursor(3,24);
  if (player1_ready) {
    display.print("Ready");
  } else {
    display.print("Not ready");
  }
  
  display.setCursor(72,16);
  display.print("Player 2");
  display.setCursor(70,24);
  if (player2_ready) {
    display.print("Ready");
  } else {
    display.print("Not ready");
  }
  display.display();
}

void WaitingForBetsDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.print("          ");

  display.setCursor(15,0);
  display.print("Bet Now!");
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(5,16);
  display.print("Player 1");
  display.setCursor(3,24);
  if (player1_bet) {
    display.print(player1_bet_amount);
    display.print(" C");
  } else {
    display.print("Not ready");
  }
  
  display.setCursor(72,16);
  display.print("Player 2");
  display.setCursor(70,24);
  if (player2_bet) {
    display.print(player2_bet_amount);
    display.print(" C");
  } else {
    display.print("Not ready");
  }
  display.display();
}

void DealerCardDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  for (int i = 0; i < 4; i++) {
    print_card_no(dealerMessage.dealer_card[i], i%3, i); // card_value, card_icon, card_no
  }

  display.setCursor(26, 24);
  display.println("HIT or STAND ?");
  display.display();
}

void DealerPlayDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  for (int i = 0; i < 5; i++) {
    print_card_no(dealerMessage.dealer_card[i], i%3, i); // card_value, card_icon, card_no
  }

  display.setCursor(26, 24);
  display.print("Point: ");
  display.println(dealerSum);
  display.display();
}

void handlePlayerReadyState() {
  // Your code for handling player ready state
  if (player1_ready || player2_ready) {
    WaitingForPlayerDisplay();
  } else {
    IdleDisplay();
  }
  // Change state when conditions are met
  if (player1_ready && player2_ready) {
    WaitingForPlayerDisplay();
    delay(1000);
    currentState = 1;
    SendStateToPlayer1();
    SendStateToPlayer2();
  }
}

void handlePlayerBetState() {
  WaitingForBetsDisplay();
  if (player1_bet && player2_bet) {
    currentState = 2;
    dealerMessage.player1_card[player1_count++] = random(1,14);
    dealerMessage.player1_card[player1_count++] = random(1,14);
    dealerMessage.player2_card[player2_count++] = random(1,14);
    dealerMessage.player2_card[player2_count++] = random(1,14);
    dealerMessage.dealer_card[countCardDealer++] = random(1,14);
    if(dealerMessage.dealer_card[0] > 10) {
      dealerSum += 10;
    } else {
      dealerSum += dealerMessage.dealer_card[0];
    }
    WaitingForBetsDisplay();
    delay(1000);
    SendStateToPlayer1();
    SendStateToPlayer2();
  }
}

void handlePlayerPlayState() {
  DealerCardDisplay();
  SendStateToPlayer1();
  SendStateToPlayer2();
  if (player1_stand && player2_stand) {
    for(int i=0; i<5; i++) {
      if(dealerMessage.player1_card[i] > 10) {
        player1Sum += 10;
      } 
      else {
        player1Sum += dealerMessage.player1_card[i];
      }

      if(dealerMessage.player2_card[i] > 10) {
        player2Sum += 10;
      }
      else {
        player2Sum += dealerMessage.player2_card[i];
      }
    }

    while (dealerSum < 17 && countCardDealer < 5) {   //do once
      dealerMessage.dealer_card[countCardDealer] = random(1,14);
      if(dealerMessage.dealer_card[countCardDealer] > 10) {
        dealerSum += 10;
      } else {
        dealerSum += dealerMessage.dealer_card[countCardDealer];
      }
      DealerCardDisplay();
      delay(300);
      countCardDealer++;
    }
    Serial.print("Player 1: ");
    Serial.println(player1Sum);
    Serial.print("Player 2: ");
    Serial.println(player2Sum);
    Serial.print("Dealer: ");
    Serial.println(dealerSum);

    if(player1Sum > 21 || (player1Sum < dealerSum && dealerSum < 22)) {
      dealerMessage.player1_result = 0;
    } else if (player1Sum == dealerSum){
      dealerMessage.player1_result = 2; // draw
    } else {
      dealerMessage.player1_result = 1;
    }

    if(player2Sum > 21 || (player2Sum < dealerSum && dealerSum < 22)) {
      dealerMessage.player2_result = 0;
    } else if (player2Sum == dealerSum){
      dealerMessage.player2_result = 2; // draw
    } else {
      dealerMessage.player2_result = 1;
    }

    currentState = 3;
    SendStateToPlayer1();
    SendStateToPlayer2();
  }
}

void handleDealerPlayState() {
  DealerPlayDisplay();
  delay(10000);
  currentState = 0;
  player1_ready = false;
  player2_ready = false;
  player1_bet = false;
  player2_bet = false;

  player1_count = 0;
  player2_count = 0;

  player1_stand = false;
  player2_stand = false;

  dealerSum = 0;
  countCardDealer = 0;

  player1Sum = 0;
  player2Sum = 0;

  dealerMessage.player_state = 0;  
  dealerMessage.player1_result = 0; 
  dealerMessage.player2_result = 0; 
  memset(dealerMessage.player1_card, 0, sizeof(dealerMessage.player1_card));
  memset(dealerMessage.player2_card, 0, sizeof(dealerMessage.player2_card));
  memset(dealerMessage.dealer_card, 0, sizeof(dealerMessage.dealer_card));
  SendStateToPlayer1();
  SendStateToPlayer2();
}