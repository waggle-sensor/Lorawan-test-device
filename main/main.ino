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
const int PACKET_INTERVAL = 20000; // Send packet every 20 seconds

// button vars
const int BUTTON_PIN = 2;

// loading snake vars
const int FRAMES = 12;
const int FRAME_DELAY = 100;
const float ANGLE_INCREMENT = 2 * PI / FRAMES;
const int SNAKE_LEN = 8;
int currentFrame = 0;
float angles[SNAKE_LEN];

//wait time
const int MAX_WAIT_TIME = 60000;

// packet sizes
int packetSizes[] = {16, 32, 64}; //any bytes higher than 80 fail
// Data rates for US915 (no DR0, data size is too small)
int dataRates[] = {1, 2, 3}; 
// Max application payload size (bytes) per DR for US915
int maxPayloadPerDR[] = {
  53,   // DR1: SF9 / 125 kHz
  125,  // DR2: SF8 / 125 kHz
  222   // DR3: SF7 / 125 kHz
};
int numPacketSizes = sizeof(packetSizes) / sizeof(packetSizes[0]);

// Global counter
int packetId = 0;  

void blinkLoading(int waitTime) {
  int blinkDelay = 1500;
  int iter = waitTime / blinkDelay;
  for (int i = 0; i < iter; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blinkDelay);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blinkDelay);
  }
}

void blinkPassed() {
  int blinkDelay = 100;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blinkDelay);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blinkDelay);
  }
}

void blinkFailed() {
  int blinkDelay = 600;
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blinkDelay);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blinkDelay);
  }
}

void drawWaiting() {
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
  delay(3000);
  display.stopscroll();
}

void Loading() {
  drawLoadingSnake(currentFrame);
  currentFrame = (currentFrame + 1) % FRAMES;
}

void myDelay(int waitTime) {
  if (useLEDOnly) {
    blinkLoading(waitTime);
    return;
  }

  int iter = waitTime / FRAME_DELAY;
  for (int i = 0; i < iter; i++) {
    Loading();
    delay(FRAME_DELAY);
  }
}

bool joinNetwork() {
  int waitTime = 8000;

  while (true) {
    if (modem.joinOTAA(appEui, appKey, 60000)) {
      
      // Try sending a short test packet to confirm the join really worked
      uint8_t testPayload[3] = {0x01, 0x01, 0x03};  // Simple 3-byte test packet
      modem.beginPacket();
      modem.write(testPayload, sizeof(testPayload));
      bool success = modem.endPacket(true) > 0;

      if (success) {
        drawPassed("Join + Send OK");
        delay(5000);
        return true;
      } else {
        drawFailed("Join OK, Send Fail");
        delay(5000);
        return false;  // Joined but failed to send — not fully working
      }
    }

    myDelay(waitTime);
    waitTime *= 2;
    if (waitTime > MAX_WAIT_TIME) {
      drawFailed("Join Fail");
      return false;
    }
  }
}

bool SendPacketWithSize(int size, int packetId, int dataRate) {
  if (size > 255) return false;

  uint8_t payload[size];

  payload[0] = packetId; // Packet ID in first byte

  for (int i = 1; i < size - 1; i++) {
    payload[i] = 1;       // Fill with 1
  }

  payload[size - 1] = size;     // Packet size in last byte

  modem.dataRate(dataRate);  // https://github.com/arduino/mkrwan1300-fw/issues/6#issuecomment-896926106
  modem.beginPacket();
  modem.write(payload, size);
  bool success = modem.endPacket(true) > 0;

  return success;
}

void displayPacketSize(int size, int packetId, int dataRate) {
  if (useLEDOnly) {
    for (int i = 0; i < size / 32; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
    delay(3000);
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Pkt ID: ");
  display.print(packetId);

  display.setCursor(0, 12);
  display.print("Size: ");
  display.print(size);
  display.print("B");

  display.setCursor(0, 24);
  display.print("DR: ");
  display.print(dataRate);

  display.display();
  delay(3000);
}

void setup() {
  delay(5000);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  Wire.begin();

  // Manually check if the OLED responds on I2C
  Wire.beginTransmission(SCREEN_ADDRESS);
  if (Wire.endTransmission() != 0) {
    useLEDOnly = true;
  } else {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      useLEDOnly = true;
    } else {
      display.display();
      display.clearDisplay();
      initializeSnakeAngles();
    }
  }

  writeStarting();

  modem.begin(US915);
  myDelay(200);
  modem.minPollInterval(60);
  modem.setPort(3);
  modem.dataRate(0);
  modem.setADR(true);
  myDelay(1000);

  if (!joinNetwork()) {
    // join failed, stop further execution
    while (true) {
      blinkFailed();
      delay(5000);
    }
  }
}

void loop() {
  for (int i = 0; i < numPacketSizes; i++) { // Loop through packet sizes
    int size = packetSizes[i];

    for (int dr = 0; dr < 4; dr++) { // Loop through data rates
      int dataRate = dataRates[dr];

      // Skip if packet size exceeds allowed payload size for this DR
      if (size > maxPayloadPerDR[dr]) {
        continue;  // Don't try to send this combo
      }

      for (int j = 0; j < 3; j++) { // Send 3 packets per valid combination of size and DR
        displayPacketSize(size, packetId, dataRate);

        if (SendPacketWithSize(size, packetId, dataRate)) {
          drawPassed("Packet Sent");
          delay(5000);
        } else {
          drawFailed("Packet Fail");
          delay(5000);
        }

        packetId++;
        myDelay(PACKET_INTERVAL);
      }
    }
  }

  // Done sending all packet sizes
  if (useLEDOnly) {
    while (true) {
      blinkPassed();
      delay(3000);
    }
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 12);
    display.println("Test Complete!");
    display.display();
    while (true); // Halt after test
  }
}
