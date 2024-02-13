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
int state = 0;

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0x69, 0x64} ;

typedef struct ready_message {
  char message[32]; //message
  int id;  //player number 
  bool is_ready;  // ready or not
} ready_message;

typedef struct bet_message {
  char message[32]; //message
  int id;  //player number 
  int amount;  // ready or not
} bet_message;

typedef struct hit_card_message {
  char message[32]; //message
  int id;  //player number 
  bool hit;  //player hit request 1 = hit, 0 = stand
} hit_card_message;

typedef struct draw_card_message {
  char message[32]; //message
  int card[5];  //player number 
} draw_card_message;

typedef struct dealer_message {
  char message[32]; //message
  int player_state;  //state number 0 = init , 1=bet , 2 = playing, 3 = waiting
} dealer_message;

typedef struct result_message {
  char message[32]; //message
  int result;  //result number 0 = lose, 1 = win, 2 = draw
} result_message;

// struct_message myData;
ready_message readyMessage;
bet_message betMessage;
hit_card_message hitMessage;
draw_card_message drawCard;
dealer_message stateChange;
result_message result;

esp_now_peer_info_t peerInfo;

//Start display
void InitDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Press button to start");
  display.display();
}


//Recieve cards data and display
void CardRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&drawCard, incomingData, sizeof(drawCard));

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Your cards: ");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,9);
  display.print(drawCard.card[0]);
  display.print(" ");
  display.print(drawCard.card[1]);
  for(int i=2;i<5;i++){
    if(drawCard.card[i]!=0){
      display.print(drawCard.card[i]);
      display.print(" ");
    }
  }
  display.println("");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,27);
  display.println("Hit or Stand");

  display.display();
}

void InitToBetDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for others");
  display.display();
}

void BetToPlayingDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for others bidding");
  display.display();
}

//State recieve from dealer
void StateRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&stateChange, incomingData, sizeof(stateChange));
  Serial.println(stateChange.player_state);
  Serial.println(stateChange.player_state);
}

//Result recieve from dealer
void ResultRecieve(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&result, incomingData, sizeof(result));
}

void OnReady(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSend Ready Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  button1.setDebounceTime(50); 
  button2.setDebounceTime(50);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);


  if (esp_now_init() != ESP_OK) {
    Serial.println("Here2");
    Serial.println("Error initializeing ESP-NOW");
    return;
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  display.clearDisplay();
  InitDisplay();
  // esp_now_register_recv_cb(OnDataRecv);


  // esp_now_register_send_cb(OnReady);
  //Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  ////Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Fail to add peer");
    return;
  }