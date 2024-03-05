#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#include "card_icon.h"
#include "card_printer.h"
#include <ESP32RotaryEncoder.h>
#include <string.h>

#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

//id
int id;

//button
ezButton button1(4);
ezButton button2(5);

//variables
int currentState = 0;
int gameEnded = false;
int cardLoading = false;

//pot value
// const int potPin = 35;
const int potPin = 36;
int potValue = 0;

// bet
int bet_amount = 0;
int MyCredit = 0;

//rotary
const uint8_t DI_ENCODER_A   = 34; // Might be labeled CLK
const uint8_t DI_ENCODER_B   = 35; // Might be labeled DT
const int8_t  DI_ENCODER_SW  = 32; // SW Pin
RotaryEncoder rotaryEncoder( DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW );
uint8_t rotary_percentage = 0;
uint8_t last_rotary_percentage = 0;

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0x69, 0x64};
uint8_t Coin1MacAddress[] = {0x3C, 0x61, 0x05, 0x03, 0xB4, 0x50};
uint8_t Coin2MacAddress[] = {0x24, 0x6F, 0x28, 0x50, 0xA6, 0x78};
uint8_t player1MacAddress[] = {0x3C, 0x71, 0xBF, 0x10, 0x5C, 0x3C};

bool isReady = false;
bool betPlaced = false;
bool pickStand = false;
int cardCount = 2;

typedef struct game_state_message {
  int state;
  int id;
  char message[32];
  bool is_ready;
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
  int ForWho;
} dealer_message;

game_state_message gameStateMessage;
dealer_message dealerMessage;

esp_now_peer_info_t peerInfo;

void knobCallback( long value )
{
    rotary_percentage = value;
    if (currentState == 1) {
      if (!betPlaced) {
        if(rotary_percentage > last_rotary_percentage && bet_amount != MyCredit) { //value increased and still not max yet
          bet_amount += 100 * (rotary_percentage - last_rotary_percentage);
          if (bet_amount > MyCredit) {
            bet_amount = MyCredit;
          }
        }

        if(rotary_percentage < last_rotary_percentage && bet_amount > 0) { //value decreased and still not 0 yet
          bet_amount -= 100 * (last_rotary_percentage - rotary_percentage);
          if (bet_amount < 0) {
            bet_amount = 0;
          }
        }
      }
    }
    last_rotary_percentage = rotary_percentage;
    Serial.printf( "Value: %i\n", value );
}

void buttonCallback( unsigned long duration )
{
    Serial.printf( "boop! button was down for %u ms\n", duration );
}

//Start display
void InitDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,12);
  display.println("Credit");
  display.print(MyCredit);
  display.println(" C");

  // display.setCursor(95,0);
  // display.println(">>>>>");
  display.setCursor(95,5);
  display.println("Leave");

  display.setCursor(95,14);
  display.println(">>>>>");
  display.setCursor(95,23);
  display.println("Ready");

  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(39, 6);
  display.println("  ");

  display.setCursor(45, 7);
  if (id == 1) {
    display.println("P1");
  } else {
    display.println("P2");
  }

  display.display();
}

void MyCardDisplay() {

  if (id == 1) {
    for (int i = 0; i < 5; i++) {
      print_card_no(dealerMessage.player1_card[i], i%3, i); // card_value, card_icon, card_no
    }
  } else {
    for (int i = 0; i < 5; i++) {
      print_card_no(dealerMessage.player2_card[i], i%3, i); // card_value, card_icon, card_no
    }
  }
}

void WaitingForJoinDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(24,0);
  display.print("Credit: ");
  display.print(MyCredit);
  display.println(" C");

  display.setCursor(15,10);
  display.print("Waiting For P");
  if (id == 1) {
    display.print(2);
  } else {
    display.print(1);
  }
  display.println("...");

  display.display();
}

void LoadingDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("Loading...");
  display.display();
}

void PlaceYourBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);

  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.println("                     ");

  display.setCursor(15,0);
  display.println("Place your bet");

  display.setTextColor(WHITE);
  
  display.setCursor(0,8);
  display.print("Remaining");
  display.setCursor(73,8);
  display.print(": ");
  display.print(MyCredit - bet_amount);
  display.println(" C");

  display.setCursor(0,16);
  display.print("Bet amount");
  display.setCursor(73,16);
  display.print(": ");
  display.print(bet_amount);
  display.println(" C");

  display.setCursor(0,24);
  display.print("Total Prize");
  display.setCursor(73,24);
  display.print(": ");
  display.print(bet_amount * 2);
  display.println(" C");

  display.display();
}

void WaitingForOthersBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(18,0);
  display.print("Your bet : ");
  display.print(bet_amount);
  display.println(" C");

  display.setCursor(15,10);
  display.print("Waiting For P");
  if (id == 1) {
    display.print(2);
  } else {
    display.print(1);
  }
  display.println("...");

  display.display();
}

void DecisionDisplay(){
  display.clearDisplay();
  display.setTextColor(WHITE);

  MyCardDisplay();

  display.setTextSize(1);
  display.setCursor(26,24);
  display.println("HIT or STAND ?");
  display.display();
}

void StandPickedDisplay(){
  display.clearDisplay();
  display.setTextColor(WHITE);

  MyCardDisplay();

  display.setTextSize(1);
  display.setCursor(17,24);
  display.println("YOU PICKED STAND");
  display.display();
}

void GameResultDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  MyCardDisplay();

  display.setTextSize(1);
  display.setCursor(26, 24);
  if (id == 1) {
    switch(dealerMessage.player1_result) {
      case 0:
        display.println("You Lose");
        break;
      case 1:
        display.println("You Win");
        break;
      case 2:
        display.println("You Draw");
        break;
    }
  } else {
    switch(dealerMessage.player2_result) {
      case 0:
        display.println("You Lose");
        break;
      case 1:
        display.println("You Win");
        break;
      case 2:
        display.println("You Draw");
        break;
    }

  }
  display.display();
}

//State recieve from dealer
void OnStateRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&dealerMessage, incomingData, sizeof(dealerMessage));
  if (dealerMessage.FromWho == 1) {
    MyCredit += dealerMessage.DepositCredit;
    Serial.println("deposit 100 credit");
  } else {
    currentState = dealerMessage.player_state;
    
    if (id == 1) {
      if (dealerMessage.ForWho == 1) {
        cardLoading = false;
      }
    } else {
      if (dealerMessage.ForWho == 2) {
      // if (!memcmp(dealerMessage.message, "2", sizeof(dealerMessage.message) == 0)) {
        cardLoading = false;
      }
    }

    if (dealerMessage.player_state == 0) {
      ResetGame();
    } else if (dealerMessage.player_state == 3) {
      Serial.println(MyCredit);
      Serial.println(bet_amount);
      Serial.println(MyCredit + bet_amount);
      if (id == 1) {
        switch(dealerMessage.player1_result) {
          case 0:
            MyCredit = MyCredit - bet_amount;
            break;
          case 1:
            MyCredit = MyCredit + bet_amount;
            break;
          case 2:
            break;
        }
      } else {
        switch(dealerMessage.player2_result) {
          case 0:
            MyCredit = MyCredit - bet_amount;
            break;
          case 1:
            MyCredit = MyCredit + bet_amount;
            break;
          case 2:
            break;
        }
      }
    }
  }
}

void handlePlayerIdleState();
void handlePlayerPlaceBetState();
void handlePlayerPlayingState();
void handleDealerPlayState();

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void SendHitToDealer() {
  cardLoading = true;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK) {
    result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
}

void SendStateToDealer() {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
    result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
}

void SendWithdrawToCoinMaster1() {
  esp_err_t result = esp_now_send(Coin1MacAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
    Serial.println("retying");
    result = esp_now_send(Coin1MacAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
  return;
}

void SendWithdrawToCoinMaster2() {
  esp_err_t result = esp_now_send(Coin2MacAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
    result = esp_now_send(Coin2MacAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
  return;
}

void ResetGame() {
  isReady = false;
  betPlaced = false;
  pickStand = false;
  cardCount = 2;
  gameEnded = false;
  bet_amount = 0;
  gameStateMessage.state = 0;
  gameStateMessage.bet_amount = 0;
  gameStateMessage.hit = true;
}

void setup() {
  button1.setDebounceTime(250); 
  button2.setDebounceTime(250);
  
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializeing ESP-NOW");
    return;
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  display.clearDisplay();
  InitDisplay();

  uint8_t myMacAddress[6];
  WiFi.macAddress(myMacAddress);

  if (memcmp(myMacAddress, player1MacAddress, 6) == 0) {
    Serial.println("This is player 1");
    id = 1;
  } else {
    Serial.println("This is player 2");
    id = 2;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  ////Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fail to add peer");
    return;
  }

  if (id == 1) {
    for(int i=0; i<6; ++i) {
      peerInfo.peer_addr[i] = (uint8_t) Coin1MacAddress[i];
    }
    //Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Fail to add peer");
      return;
    }
  } else {
    for(int i=0; i<6; ++i) {
      peerInfo.peer_addr[i] = (uint8_t) Coin2MacAddress[i];
    }
    //Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Fail to add peer");
      return;
    }

  }


  //Register send callback
  esp_now_register_send_cb(OnDataSent);

  //Register receive callback
  esp_now_register_recv_cb(OnStateRecieve);

  // This tells the library that the encoder has its own pull-up resistors
  rotaryEncoder.setEncoderType( EncoderType::HAS_PULLUP );

  // Range of values to be returned by the encoder: minimum is 1, maximum is 10
  // The third argument specifies whether turning past the minimum/maximum will
  // wrap around to the other side:
  //  - true  = turn past 10, wrap to 1; turn past 1, wrap to 10
  //  - false = turn past 10, stay on 10; turn past 1, stay on 1
  rotaryEncoder.setBoundaries( 1, 10, false );

  // The function specified here will be called every time the knob is turned
  // and the current value will be passed to it
  rotaryEncoder.onTurned( &knobCallback );

  // The function specified here will be called every time the button is pushed and
  // the duration (in milliseconds) that the button was down will be passed to it
  rotaryEncoder.onPressed( &buttonCallback );

  // This is where the inputs are configured and the interrupts get attached
  rotaryEncoder.begin();
}

void loop () {
  button1.loop();
  button2.loop();
  switch(currentState) {
    case 0:
      handlePlayerIdleState();
      break;
    case 1:
      handlePlayerPlaceBetState();
      break;
    case 2:
      handlePlayerPlayingState();
      break;
    case 3:
      handleDealerPlayState();
      break;
    }
}

void handlePlayerIdleState() {
  if(!isReady) {
    InitDisplay();
    if(button1.isPressed()){ //press to ready
      Serial.println("state 0 button pressed");
      gameStateMessage.state = currentState;
      gameStateMessage.id=id;
      gameStateMessage.is_ready=1;
      isReady = true;
      SendStateToDealer();
    } else if (button2.isPressed()){
      Serial.println("Withdraw all credit");
      gameStateMessage.WithdrawCredit = MyCredit;
      MyCredit = 0;
      gameStateMessage.id=id;
      if(id == 1) {
        SendWithdrawToCoinMaster1();
      } else {
        SendWithdrawToCoinMaster2();
      }
    } 
  }
  else {
    WaitingForJoinDisplay();
  }
}

void handlePlayerPlaceBetState() {
  if (!betPlaced) {
    PlaceYourBetDisplay();
    if(button1.isPressed()){ // press to place bet
      Serial.println("state 1 button pressed");
      gameStateMessage.state = currentState;
      gameStateMessage.id=id;
      gameStateMessage.is_ready=1;
      gameStateMessage.bet_amount=bet_amount;
      betPlaced = true;
      SendStateToDealer();
    }
  } else {
    WaitingForOthersBetDisplay();
  }
  // last_rotary_percentage = rotary_percentage;
}

void handlePlayerPlayingState() {

  if(button1.isPressed() && !cardLoading){ //if hit
    Serial.print("card loading");
    Serial.println(cardLoading);
    Serial.println("You Pick Hit!");
    gameStateMessage.state=currentState;
    gameStateMessage.id=id;
    gameStateMessage.hit=true;
    SendHitToDealer();
    cardCount++;
    if (cardCount == 5) {
      pickStand=true;
    }
  }
  else if(button2.isPressed()){ // if stand
    Serial.println("You Pick Stand!");
    gameStateMessage.state=currentState;
    gameStateMessage.id=id;
    gameStateMessage.hit=false;
    pickStand=true;
    SendStateToDealer();
  }
  if (cardLoading) {
    LoadingDisplay();
  }
  else if (!pickStand) {
    DecisionDisplay();
  }
  else {
    StandPickedDisplay();
  }
}

void handleDealerPlayState() {
  GameResultDisplay();
}