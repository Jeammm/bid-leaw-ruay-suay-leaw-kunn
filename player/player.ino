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
int id;

//button
ezButton button1(4);
ezButton button2(5);

//variables
int currentState = 0;
int gameEnded = false;

//pot value
const int potPin = 35;
int potValue = 0;

// bet
int bet_amount = 0;
int MyCredit=1000;

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0x69, 0x64};
uint8_t Coin1MacAddress[] = {0xA4, 0xCF, 0x12, 0x8F, 0xBA, 0x18};
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
} dealer_message;

game_state_message gameStateMessage;
dealer_message dealerMessage;

esp_now_peer_info_t peerInfo;

//Start display
void InitDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5,0);
  display.println("Letf to Ready"); 
  display.setCursor(0,8);
  display.println("Right to Withdraw");
  display.setCursor(25,16);
  display.print("Credit: ");
  display.print(MyCredit);
  display.print(" C");
  display.display();
}

void MyCardDisplay() {
  for (int i; i < 5; i++) {
    int myCard;
    if (id == 1) {
      myCard = dealerMessage.player1_card[i];
    } else {
      myCard = dealerMessage.player2_card[i];
    }

    if (myCard == 1) {
      display.print("A"); 
    } else if (myCard <= 10 && myCard >= 2) {
      display.print(myCard); 
    } else if (myCard == 11) {
      display.print("J"); 
    } else if (myCard == 12) {
      display.print("Q"); 
    } else if (myCard == 13) {
      display.print("K"); 
    }
    display.print(" ");
  }
  display.println("");
}

void WaitingForJoinDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for others to join");
  display.setCursor(25,16);
  display.print("Credit: ");
  display.print(MyCredit);
  display.print(" C");
  display.display();
}

void PlaceYourBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Place your bet");
  display.setCursor(25,16);
  display.print("Credit: ");
  display.print(MyCredit);
  display.println(" C");
  display.println(bet_amount);
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
  MyCardDisplay();

  display.setCursor(0,8);
  display.println("Hit or Stand ?");
  display.display();
}

void StandPickedDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  MyCardDisplay();

  display.setCursor(0,8);
  display.println("You picked Stand");
  display.display();
}

void GameResultDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  MyCardDisplay();

  display.setTextSize(2);
  display.setCursor(0,8);
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

void SendStateToDealer() {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
    result = esp_now_send(broadcastAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  }
  return;
}

void SendWithdrawToCoinMaster1() {
  esp_err_t result = esp_now_send(Coin1MacAddress, (uint8_t *) &gameStateMessage, sizeof(gameStateMessage));
  while(result != ESP_OK)
  {
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
  gameStateMessage.state = 0;
  gameStateMessage.bet_amount = 100;
  gameStateMessage.hit = true;
}

void setup() {
  button1.setDebounceTime(400); 
  button2.setDebounceTime(400);
  
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

  //Register send callback
  esp_now_register_send_cb(OnDataSent);

  //Register receive callback
  esp_now_register_recv_cb(OnStateRecieve);
}

void loop () {
  potValue = analogRead(potPin);
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
  Serial.println(potValue);
  if (!betPlaced) {
    PlaceYourBetDisplay();
    if (potValue <= 400){
      bet_amount = MyCredit*0.1;
    }else if (potValue <= 800){
      bet_amount = MyCredit*0.2;
    }else if (potValue <= 1200){
      bet_amount = MyCredit*0.3;
    }else if (potValue <= 1600){
      bet_amount = MyCredit*0.4;
    }else if (potValue <= 2000){
      bet_amount = MyCredit*0.5;
    }else if (potValue <= 2400){
      bet_amount = MyCredit*0.6;
    }else if (potValue <= 2800){
      bet_amount = MyCredit*0.7;
    }else if (potValue <= 3200){
      bet_amount = MyCredit*0.8;
    }else if (potValue <= 3600){
      bet_amount = MyCredit*0.9;
    }else if (potValue <= 4096){
      bet_amount = MyCredit;
    }

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
}

void handlePlayerPlayingState() {
  if(button1.isPressed()){ //if hit
    Serial.println("You Pick Hit!");
    gameStateMessage.state=currentState;
    gameStateMessage.id=id;
    gameStateMessage.hit=true;
    SendStateToDealer();
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
  if (!pickStand) {
    DecisionDisplay();
  } else {
    StandPickedDisplay();
  }
}

void handleDealerPlayState() {
  GameResultDisplay();
}