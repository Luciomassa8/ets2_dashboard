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
bool retro = 0;
String truckBrand;
String truckName;
int engineRpmMax;
float fuel;
int gear_to_print;
int gears;
int gears_reverse;

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

  // rpm, gear, gear_dashboard, speed, truckBrand, truckName, engineRpmMax, fuel, gears, gears_reverse
  // int  int    int            float   string      string     int          float int     int

  String s = ln;
  s.trim();
  if (s.length() == 0) return;

  int i1  = s.indexOf(',');          // index of rpm
  int i2  = s.indexOf(',', i1 + 1);  // index of gear
  int i3  = s.indexOf(',', i2 + 1);  // index of gear_dasboard
  int i4  = s.indexOf(',', i3 + 1);  // index of speed
  int i5  = s.indexOf(',', i4 + 1);  // index of truckBrand
  int i6  = s.indexOf(',', i5 + 1);  // index of truckName
  int i7  = s.indexOf(',', i6 + 1);  // index of engineRpm
  int i8  = s.indexOf(',', i7 + 1);  // index of fuel
  int i9  = s.indexOf(',', i8 + 1);  // index of gears
  int i10 = s.indexOf(',', i9 + 1);  // index of gears_reverse
  if (i1 < 0 || i2 < 0 || i3 < 0 || i4 < 0 || i5 < 0 || i6 < 0 || i7 < 0 || i8 < 0 || i9 < 0 || i10 < 0) return;

  rpm = s.substring(0, i1).toInt();
  gear = s.substring(i1 + 1, i2).toInt();
  gearDash = s.substring(i2 + 1, i3).toInt();
  speedKmh = (s.substring(i3 + 1, i4).toFloat()) * 3.6;
  truckBrand = s.substring(i4 + 1, i5);
  truckName = s.substring(i5 + 1, i6);
  engineRpmMax = s.substring(i6 + 1, i7).toInt();
  fuel = s.substring(i7 + 1, i8).toFloat();
  gears = s.substring(i8 + 1, i9).toInt();
  gears_reverse = s.substring(i9 + 1, i10).toInt();
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
  if (gear == 0) {
    display.print("N");
  }else{
    if(gear < 0){
      gear = gear * (-1);
      retro = 1;
    }else{
      retro = 0;
    }
    if(truckBrand == "Volvo"){
      cambio_volvo(gear, retro);
    }else if(truckBrand == "Scania"){
      cambio_scania(gear, retro);
    }else{
      altri_cambi(gear, retro);
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

void gear_H_L(int gear){
  display.setCursor(112, 16);
  if(gear % 2 == 0){
    // marcia pari, quindi H
    display.print("H");
  }else{
    // marcia dispari, quindi L
    display.print("L");
  }
}

void cambio_scania(int gear, bool retro){
  // se retro == 0 marcia avanti, altrimenti marcia indietro
  if(retro){
    display.setCursor(88, 16);
    display.print("R");
    gear_H_L(gear);
  }else{
    if(gears == 14){
      scania_volvo(gear);
    }else{
      altri_cambi(gear, retro);
    }
  }
}

void cambio_volvo(int gear, bool retro){
  // se retro == 0 marcia avanti, altrimenti marcia indietro
  if(retro){
    display.setCursor(88, 16);
    display.print("R");
    display.setCursor(100, 16);
    if(gear % 2 != 0){
      gear_to_print = (gear / 2) + 1;
    }else{
      gear_to_print = gear / 2;
    }
    display.print(gear_to_print);
    gear_H_L(gear);
  }else{
    if(gears == 14){
      scania_volvo(gear);
    }else{
      altri_cambi(gear, retro);
    }
  }
}

void scania_volvo(int gear){
  if(gear <= 2){
    display.print("C");
    gear_H_L(gear);
  }else{
    display.print("A");
    gear = gear - 2;
    display.setCursor(100, 16);
    if(gear % 2 != 0){
      gear_to_print = (gear / 2) + 1;
    }else{
      gear_to_print = gear / 2;
    }
    display.print(gear_to_print);
    gear_H_L(gear);
  }
}

void altri_cambi(int gear, bool retro){
  if(retro){
    display.print("R");
    display.setCursor(100, 16);
    display.print(gear);
  }else{
    display.print("A");
    display.setCursor(100, 16);
    display.print(gear);
  }
}