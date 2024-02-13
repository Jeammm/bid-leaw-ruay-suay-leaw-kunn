#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

//id
int id=1;

//button
ezButton button1(4);
ezButton button2(5);

//variables
int currentState = 0;

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0x69, 0x64};

bool isReady = false;
bool betPlaced = false;
bool pickStand = false;

// typedef struct ready_message {
//   char message[32]; //message
//   int id;  //player number 
//   bool is_ready;  // ready or not
// } ready_message;

// typedef struct bet_message {
//   char message[32]; //message
//   int id;  //player number 
//   int amount;  // ready or not
// } bet_message;

// typedef struct hit_card_message {
//   char message[32]; //message
//   int id;  //player number 
//   bool hit;  //player hit request 1 = hit, 0 = stand
// } hit_card_message;

// typedef struct draw_card_message {
//   char message[32]; //message
//   int card[5];  //player number 
// } draw_card_message;

// typedef struct dealer_message {
//   char message[32]; //message
//   int player_state;  //state number 0 = init , 1=bet , 2 = playing, 3 = waiting
// } dealer_message;

// typedef struct result_message {
//   char message[32]; //message
//   int result;  //result number 0 = lose, 1 = win, 2 = draw
// } result_message;

// ready_message readyMessage;
// bet_message betMessage;
// hit_card_message hitMessage;
// draw_card_message drawCard;
// dealer_message stateChange;
// result_message result;


typedef struct game_state_message {
  int state;
  int id;
  char message[32];
  bool is_ready;
  int bet_amount;
  bool hit;
} game_state_message;

typedef struct dealer_message {
  char message[32]; //message
  int player_state;  //state number 0 = init , 1=bet , 2 = playing, 3 = waiting
  int player1_result; //result number 0 = lose, 1 = win, 2 = draw
  int player2_result; //result number 0 = lose, 1 = win, 2 = draw
  int player1_card[5];
  int player2_card[5];
  int dealer_card[5];
} dealer_message;

game_state_message gameStateMessage;
dealer_message dealerMessage;

esp_now_peer_info_t peerInfo;

//Recieve cards data and display
// void CardRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
//   memcpy(&drawCard, incomingData, sizeof(drawCard));

//   display.clearDisplay();

//   display.setTextSize(1);
//   display.setTextColor(WHITE);
//   display.setCursor(0,0);
//   display.println("Your cards: ");

//   display.setTextSize(1);
//   display.setTextColor(WHITE);
//   display.setCursor(0,9);
//   display.print(drawCard.card[0]);
//   display.print(" ");
//   display.print(drawCard.card[1]);
//   for(int i=2;i<5;i++){
//     if(drawCard.card[i]!=0){
//       display.print(drawCard.card[i]);
//       display.print(" ");
//     }
//   }
//   display.println("");

//   display.setTextSize(1);
//   display.setTextColor(WHITE);
//   display.setCursor(0,27);
//   display.println("Hit or Stand");

//   display.display();
// }

//Start display
void InitDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Press button to start");
  display.display();
}

void WaitingForJoinDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for others to join");
  display.display();
}

void PlaceYourBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Place your bet");
  display.display();
}

void WaitingForOthersBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for others betting");
  display.display();
}

void DecisionDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hit or Stand ?");
  display.display();
}

void StandPickedDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("You picked Stand");
  display.display();
}

void GameResultDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

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
  display.display();
}

//State recieve from dealer
void OnStateRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&dealerMessage, incomingData, sizeof(dealerMessage));
  currentState = dealerMessage.player_state;
}

//Result recieve from dealer
// void ResultRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
//   memcpy(&result, incomingData, sizeof(result));
// }

// void OnReady(const uint8_t *mac_addr, esp_now_send_status_t status) {
//   Serial.print("\r\nSend Ready Status:\t");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
// }

void handlePlayerIdleState();
void handlePlayerPlaceBetState();
void handlePlayerPlayingState();
void handleDealerPlayState();

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void SendStateToDealer() {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
    result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
  return;
}

void setup() {
  button1.setDebounceTime(50); 
  button2.setDebounceTime(50);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializeing ESP-NOW");
    return;
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  display.clearDisplay();
  InitDisplay();

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  ////Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fail to add peer");
    return;
  }

  //Register send callback
  esp_now_register_send_cb(OnDataSent);

  //Register receive callback
  esp_now_register_recv_cb(OnStateRecieve);
}

// void loop (){

//   while(state==0){
//     button1.loop();
//     button2.loop();  
//     // Serial.println(state);
//     if(button1.isPressed()){
//       Serial.println("state 0 button pressed");
//       readyMessage.id=id;
//       readyMessage.is_ready=1;
//       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &readyMessage, sizeof(readyMessage));
//       delay(100);
//       break;
//     }
//   }

//   InitToBetDisplay();
//   while(stateChange.player_state==0){
//     delay(10);
//   }
//   state = stateChange.player_state;

//   Serial.println("state 1 begin");
//   BetToPlayingDisplay();
//   while(state==1){
//     button1.loop();
//     button2.loop();  
//     if(button1.isPressed()){
//       betMessage.id=id;
//       betMessage.amount=10;
//       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &betMessage, sizeof(betMessage));
//       delay(100);
//       break;
//     }
//   }

//   while(stateChange.player_state==2){
//     delay(10);
//   }
//   state = stateChange.player_state;
  

//   Serial.println("state 2 begin");
//   esp_now_register_recv_cb(CardRecieve);
//   while(state==2){
//     button1.loop();
//     button2.loop();  
//     if(button1.isPressed()){
//       hitMessage.id=id;
//       hitMessage.hit=1;
//       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &hitMessage, sizeof(hitMessage));
//     }
//     else if(button2.isPressed()){
//       hitMessage.id=id;
//       hitMessage.hit=0;
//       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &hitMessage, sizeof(hitMessage));
//       delay(100);
//       break;
//     }
//   }

//   while(stateChange.player_state==3){
//   }
//   state = stateChange.player_state;
//   Serial.println("state 3 begin");

//   while(state==3){
//     button1.loop();
//     button2.loop();  
//     esp_now_register_recv_cb(ResultRecieve);
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setTextColor(WHITE);
//     display.setCursor(0,0);
//     if (result.result == 0){
//       display.println("You Lose");
//     }
//     else if (result.result == 1){
//       display.println("You win");
//     }
//     else if (result.result == 2){
//       display.println("Draw");
//     }
//     display.display();
//     delay(10000);
//     break;
//   }

//   // if(stateChange.player_state!=state){
//   //     state = stateChange.player_state;
//   // }
// }

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
    }
  } else {
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
      gameStateMessage.bet_amount=100;
      betPlaced = true;
      SendStateToDealer();
    }
  } else {
    WaitingForOthersBetDisplay();
  }
}

void handlePlayerPlayingState() {
  if(button1.isPressed()){ //if hit
    Serial.println("You Pick Hit!");
    gameStateMessage.state=currentState;
    gameStateMessage.id=id;
    hitMessage.hit=true;
    pickStand=false;
    SendStateToDealer();
  }
  else if(button2.isPressed()){ // if stand
    Serial.println("You Pick Stand!");
    gameStateMessage.state=currentState;
    gameStateMessage.id=id;
    hitMessage.hit=false;
    pickStand=true;
    SendStateToDealer();
  }
  if (!pickStand) {
    DecisionDisplay();
  } else {
    StandPickedDisplay();
  }
}

void handleDealerPlayState() {
  GameResultDisplay();
}