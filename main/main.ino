#include <MKRWAN.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//display vars
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // -1 sharing Arduino reset pin
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//lorawan vars
LoRaModem modem;
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;
int UplinkTime = 60000;

// Loading snake animation vars
const int numFrames = 12;
const int frameDelay = 100; // milliseconds per frame
const float angleIncrement = 2 * PI / numFrames;
const int snakeLength = 8; // Length of the snake
int currentFrame = 0;
float angles[snakeLength]; // Array to store angles for each segment of the snake

void initializeSnakeAngles() {
  // Initialize angles for each segment of the snake
  for (int i = 0; i < snakeLength; i++) {
    angles[i] = i * 2 * PI / snakeLength;
  }
}

void drawLoadingSnake(int frame) {
  display.clearDisplay();

  // Draw each segment of the snake
  for (int i = 0; i <= snakeLength; i++) {
    float angle = angles[i] + frame * angleIncrement;
    int x = constrain(SCREEN_WIDTH / 2 + 10 * cos(angle), 0, SCREEN_WIDTH - 1); 
    int y = constrain(SCREEN_HEIGHT / 2 + 10 * sin(angle), 0, SCREEN_HEIGHT - 1);
    display.fillCircle(x, y, 2, SSD1306_WHITE);
  }
  display.display();
}

void blink() {
  int WaitTime=300;
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

bool joinNetwork() {  
  blink();
  int connected = modem.joinOTAA(appEui, appKey, 3000); // 3 sec timeout
  if (connected) {
    return true;
  } else {
    return false;
  }
}

void writeStarting(void) {
  display.clearDisplay();

  display.setTextSize(1.8);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, SCREEN_HEIGHT / 2);
  display.print("Starting Test...");
  display.display();
  delay(100);

  display.startscrollleft(0x00, 0x0F);
  delay(5000);
  display.stopscroll();
}

void writeResult(bool result) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
  if (result){
    display.print("Passed");
  }
  else {
    display.print("Failed");
  }
  display.display();
  delay(5000);
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  display.clearDisplay();
  initializeSnakeAngles();
  writeStarting();
  delay(3000);
  writeResult(1);
}

void loop() {
  drawLoadingSnake(currentFrame);
  currentFrame = (currentFrame + 1) % numFrames;
  delay(frameDelay);
}
