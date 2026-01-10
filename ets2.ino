#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned long DISPLAY_UPDATE_MS = 100;

String line = "";
long rpm = 0;
int gear = 0;
int gearDash = 0;
float speedKmh = 0.0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastRecvTime = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) { delay(1000); }
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  displayWelcome();
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (line.length() > 0) {
        processLine(line);
        line = "";
      }
    } else {
      line += c;
      if (line.length() > 200) line = "";
    }
  }

  unsigned long now = millis();
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_MS) {
    updateDisplay();
    lastDisplayUpdate = now;
  }
}

void displayWelcome() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ETS2 Telemetry Display");
  display.println("Waiting for data...");
  display.display();
}

void processLine(const String &ln) {
  String s = ln;
  s.trim();
  if (s.length() == 0) return;

  int i1 = s.indexOf(',');
  int i2 = s.indexOf(',', i1 + 1);
  int i3 = s.indexOf(',', i2 + 1);
  if (i1 < 0 || i2 < 0 || i3 < 0) return;

  rpm = s.substring(0, i1).toInt();
  gear = s.substring(i1 + 1, i2).toInt();
  gearDash = s.substring(i2 + 1, i3).toInt();
  speedKmh = s.substring(i3 + 1).toFloat();
  speedKmh = speedKmh * 3.6;
  lastRecvTime = millis();
}

void updateDisplay() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("RPM");
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(rpm);

  display.setTextSize(2);
  display.setCursor(88, 0);
  display.print("G");
  display.setTextSize(2);
  display.setCursor(88, 16);
  gear = gearDash;
  if (gear == 0) display.print("N");
  else if(gear < 0){
    display.print("R");
    display.setCursor(100, 16);
    gear = gear * (-1);
    display.print(gear);
  }
  else if(gear > 0){
    if(gear == 1 || gear == 2){
      display.print("C");
      display.setCursor(100, 16);
      display.print(gear);  
    }
    else{
      display.print("A");
      display.setCursor(100, 16);
      display.print(gear - 2);
    }
  }

  display.setTextSize(1);
  display.setCursor(0, 52);
  display.print("Speed:");
  display.setTextSize(2);
  display.setCursor(44, 48);
  if(speedKmh < 0){
    speedKmh = speedKmh * (-1);
  }
  display.print(speedKmh, 1);
  display.print(" km/h");

  display.setTextSize(1);
  display.setCursor(0, 40);
  if (millis() - lastRecvTime > 3000) {
    // display.setTextColor(SSD1306_RED);
    display.print("Waiting for data...");
  } else {
    display.print("Last recv: ");
    display.print((millis() - lastRecvTime) / 1000);
    display.print("s ago");
  }

  display.display();
}