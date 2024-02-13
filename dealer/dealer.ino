#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

int currentState = 0;

//MAC Address
uint8_t broadcastAddress1[] = {0x3C,0x71,0xBF,0x10,0x5C,0x3C} ;
uint8_t broadcastAddress2[] = {0x3C,0x61,0x05,0x04,0x3F,0x18} ;

esp_now_peer_info_t peerInfo;

//game state variables
bool player1_ready = false;
bool player2_ready = false;
bool player1_bet = false;
bool player2_bet = false;

int player1_count = 0;
int player2_count = 0;

bool player1_stand = false;
bool player2_stand = false;

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
    } else {
      player2_bet = gameStateMessage.is_ready;
    }
  } 
  else if (gameStateMessage.state == 2) {
    if(gameStateMessage.hit) {
      if(gameStateMessage.id == 1) {
        dealerMessage.player1_card[player1_count++] = random(1,14);
      }  else {
        dealerMessage.player2_card[player2_count++] = random(1,14);
      }
    } else {
      if(gameStateMessage.id == 1) {
        player1_stand = true;
      }  else {
        player2_stand = true;
      }
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

void handlePlayerReadyState() {
  // Your code for handling player ready state
  
  // Change state when conditions are met
  if (player1_ready && player2_ready) {
    currentState = 1;
    SendStateToPlayer1();
    SendStateToPlayer2();
  }
}

void handlePlayerBetState() {
  if (player1_bet && player2_bet) {
    currentState = 2;
    SendStateToPlayer1();
    SendStateToPlayer2();
  }
}

void handlePlayerPlayState() {

}
void handleDealerPlayState() {
  
}

