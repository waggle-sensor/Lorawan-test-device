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
const int MAX_WAIT_TIME = 180000; // Maximum wait time (3 minutes)

//Button vars
const int BUTTON_PIN = 2; 

// Loading snake animation vars
const int FRAMES = 12;
const int FRAME_DELAY = 100; // milliseconds per frame
const float ANGLE_INCREMENT = 2 * PI / FRAMES;
const int SNAKE_LEN = 8; // Length of the snake
int currentFrame = 0;
float angles[SNAKE_LEN]; // Array to store angles for each segment of the snake

void blinkLoading() {
  int WaitTime=1500;
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void blinkPassed() {
  int WaitTime=100;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void blinkFailed() {
  int WaitTime=600;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WaitTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(WaitTime);
  }
}

void drawWaiting() {
  display.clearDisplay();

  // Write text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18, 10);
  display.println("Press Button to");
  display.setCursor(30, 20);
  display.println("Start Test...");

  display.display();
}

void initializeSnakeAngles() {
  // Initialize angles for each segment of the snake
  for (int i = 0; i < SNAKE_LEN; i++) {
    angles[i] = i * 2 * PI / SNAKE_LEN;
  }
}

void drawLoadingSnake(int frame) {
  display.clearDisplay();

  // Draw each segment of the snake
  for (int i = 0; i <= SNAKE_LEN; i++) {
    float angle = angles[i] + frame * ANGLE_INCREMENT;
    int x = constrain(SCREEN_WIDTH / 2 + 10 * cos(angle), 0, SCREEN_WIDTH - 1); 
    int y = constrain(SCREEN_HEIGHT / 2 + 10 * sin(angle), 0, SCREEN_HEIGHT - 1);
    display.fillCircle(x, y, 2, SSD1306_WHITE);
  }
  display.display();
}

void drawPassed() {
  display.clearDisplay();

  // Draw the line for the checkmark
  display.drawLine(SCREEN_WIDTH/2, 20, 105, 0, SSD1306_WHITE); 

  // Draw the lower line of the checkmark (shorter)
  display.drawLine(50, 15, SCREEN_WIDTH/2, 20, SSD1306_WHITE);

  // Draw text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH/2.5, SCREEN_HEIGHT - 8);
  display.println("Passed");

  // Display on OLED
  display.display();
  delay(15000);
}

void drawFailed() {
  display.clearDisplay();

  // Draw the first diagonal line of the X
  display.drawLine(30, 5, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 8, SSD1306_WHITE);

  // Draw the second diagonal line of the X
  display.drawLine(30, SCREEN_HEIGHT - 8, SCREEN_WIDTH - 20, 5, SSD1306_WHITE);

  // Draw text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH/2.5, SCREEN_HEIGHT - 8);
  display.println("Failed");

  // Display on OLED
  display.display();
  delay(15000);
}

void writeStarting() {
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
  int connected = modem.joinOTAA(appEui, appKey, 15000); // 15 sec timeout
  if (connected) {
    return true;
  } else {
    return false;
  }
}

bool SendPacket() {
  //Cayenne encoding
  uint8_t payload[30]; // Allow for up to 30 byte payload
  int ValueToSend = 1;
  int idx = 0;

  //Cayenne encoding
  payload[idx++] = 1; //data channel
  payload[idx++] = 0; // Cayenne data type for digital input (data type 0)
  payload[idx++] = ValueToSend; // The actual value to send (either 0 or 1)

  //send packets
  modem.beginPacket();
  for (uint8_t c = 0; c < idx; c++) {
    modem.write(payload[c]);
  }
  int err = modem.endPacket(true);

  if (err > 0) {
    return true;
  }
  else
  {
    return false;
  }
}

bool LorawanTest() {

  modem.begin(US915);
  myDelay(200);
  
  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  myDelay(200);
  // NOTE: independent of this setting, the modem will
  // not allow sending more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.
  modem.setPort(10);
  myDelay(200);
  modem.dataRate(3);
  myDelay(200);
  modem.setADR(true);
  myDelay(200);

  int waitTime = 8000; // Initial wait time
  while (true) {
    if (!joinNetwork()) {
      break;
    }
    
    myDelay(waitTime);
    // Double the wait time
    waitTime *= 2;
    if (waitTime > MAX_WAIT_TIME) {
      return false; // Test failed, Cannot join network (maximum wait time reached)
    }
  }

  waitTime = 8000;
  while (true) {
    if (SendPacket()) {
      return true; // Test passed
    }
    
    // Retry logic
    myDelay(waitTime);
    // Double the wait time
    waitTime *= 2;
    if (waitTime > MAX_WAIT_TIME) {
      return false; // Test failed, packet was not sent (maximum wait time reached)
    }
  }

  return false; //default response
}

void setup() {
  delay(5000); //allow some downtime to upload a new sketch
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  display.clearDisplay();
  initializeSnakeAngles();
}

void loop() {
  //reset frame
  currentFrame = 0;
  drawWaiting();
  if (digitalRead(BUTTON_PIN) == HIGH) { // Button is pressed...
    writeStarting();
    if(LorawanTest()){
      drawPassed();
    } else {
      drawFailed();
    }
  }
}
