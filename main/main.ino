#include <MKRWAN.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// display vars
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool useLEDOnly = false;

// lorawan vars
LoRaModem modem;
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;
const int MAX_WAIT_TIME = 60000;

// button vars
const int BUTTON_PIN = 2;

// loading snake vars
const int FRAMES = 12;
const int FRAME_DELAY = 100;
const float ANGLE_INCREMENT = 2 * PI / FRAMES;
const int SNAKE_LEN = 8;
int currentFrame = 0;
float angles[SNAKE_LEN];

void blinkLoading() {
  int WaitTime = 1500;
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void blinkPassed() {
  int WaitTime = 100;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void blinkFailed() {
  int WaitTime = 600;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void drawWaiting() {
  if (useLEDOnly) {
    blinkLoading();
    return;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18, 10);
  display.println("Press Button to");
  display.setCursor(30, 20);
  display.println("Start Test...");
  display.display();
}

void initializeSnakeAngles() {
  for (int i = 0; i < SNAKE_LEN; i++) {
    angles[i] = i * 2 * PI / SNAKE_LEN;
  }
}

void drawLoadingSnake(int frame) {
  if (useLEDOnly) {
    blinkLoading();
    return;
  }

  display.clearDisplay();
  for (int i = 0; i <= SNAKE_LEN; i++) {
    float angle = angles[i] + frame * ANGLE_INCREMENT;
    int x = constrain(SCREEN_WIDTH / 2 + 10 * cos(angle), 0, SCREEN_WIDTH - 1); 
    int y = constrain(SCREEN_HEIGHT / 2 + 10 * sin(angle), 0, SCREEN_HEIGHT - 1);
    display.fillCircle(x, y, 2, SSD1306_WHITE);
  }
  display.display();
}

void drawPassed(const char* message) {
  if (useLEDOnly) {
    blinkPassed();
    return;
  }

  display.clearDisplay();
  display.drawLine(SCREEN_WIDTH / 2, 20, 105, 0, SSD1306_WHITE); 
  display.drawLine(50, 15, SCREEN_WIDTH / 2, 20, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH / 2.5, SCREEN_HEIGHT - 8);
  display.println(message);
  display.display();
  delay(15000);
}

void drawFailed(const char* message) {
  if (useLEDOnly) {
    blinkFailed();
    return;
  }

  display.clearDisplay();
  display.drawLine(30, 5, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 8, SSD1306_WHITE);
  display.drawLine(30, SCREEN_HEIGHT - 8, SCREEN_WIDTH - 20, 5, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH / 2.5, SCREEN_HEIGHT - 8);
  display.println(message);
  display.display();
  delay(15000);
}

void writeStarting() {
  if (useLEDOnly) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }

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

void Loading() {
  drawLoadingSnake(currentFrame);
  currentFrame = (currentFrame + 1) % FRAMES;
}

void myDelay(int waitTime) {
  int iter = waitTime / FRAME_DELAY;
  for (int i = 0; i < iter; i++) {
    Loading();
    delay(FRAME_DELAY);
  }
}

bool joinNetwork() {
  int connected = modem.joinOTAA(appEui, appKey, 15000);
  return connected;
}

bool SendPacket() {
  uint8_t payload[30];
  int ValueToSend = 1;
  int idx = 0;
  payload[idx++] = 1;
  payload[idx++] = 0;
  payload[idx++] = ValueToSend;

  modem.beginPacket();
  for (uint8_t c = 0; c < idx; c++) {
    modem.write(payload[c]);
  }
  int err = modem.endPacket(true);
  return err > 0;
}

bool LorawanTest() {
  modem.begin(US915);
  myDelay(200);

  modem.minPollInterval(60);
  myDelay(200);
  modem.setPort(10);
  myDelay(200);
  modem.dataRate(3);
  myDelay(200);
  modem.setADR(true);
  myDelay(200);

  int waitTime = 8000;
  while (true) {
    if (!joinNetwork()) {
      drawPassed("Join Pass");
      break;
    }
    myDelay(waitTime);
    waitTime *= 2;
    if (waitTime > MAX_WAIT_TIME) {
      drawFailed("Join Fail");
      return false;
    }
  }

  waitTime = 8000;
  while (true) {
    if (SendPacket()) {
      drawPassed("Packet Sent");
      return true;
    }
    myDelay(waitTime);
    waitTime *= 2;
    if (waitTime > MAX_WAIT_TIME) {
      drawFailed("Packet Fail");
      return false;
    }
  }
  return false;
}

void setup() {
  delay(5000);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    useLEDOnly = true;
  } else {
    display.display();
    display.clearDisplay();
    initializeSnakeAngles();
  }
}

void loop() {
  currentFrame = 0;
  drawWaiting();
  writeStarting();
  LorawanTest();
  if (digitalRead(BUTTON_PIN) == HIGH) { // Button is pressed...
    writeStarting();
    if(LorawanTest()){
      drawPassed("Test Passed");
    } else {
      drawFailed("Test Failed");
    }
  }
}
