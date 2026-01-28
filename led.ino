 // sda d1 scl d2 nha:) nút là d6
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ssid = "TEN WIFI"; // nhập tên wf của mn vào vd Khong Biet
const char* password = "Pass wifi"; // nhập mật khẩu ví dụ : KhongBiet
const long utcOffsetInSeconds = 7 * 3600;
#define LED_PIN     15 
#define LED_COUNT   16 // có thể đổi nha:) dùng led bao nhiêu led thì thay bằng số đó
#define BTN_MODE    12 // nút, có thể thay chân khác
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

bool ledEnabled = true;
enum LedMode { RAINBOW, ONELED };
LedMode currentMode = RAINBOW;
int scrollPosition = 0;
unsigned long lastScrollUpdate = 0;
unsigned long lastOLEDUpdate = 0;
unsigned long lastBtnModePress = 0;
unsigned long btnPressStartTime = 0;
bool btnWasPressed = false;
const unsigned long debounceDelay = 200;
const unsigned long longPressTime = 1000;

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(BTN_MODE, INPUT_PULLUP);
  Wire.begin(5, 4);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Lỗi OLED!"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println(F("Connecting WiFi"));
  display.setCursor(10, 35);
  display.print(F("SSID: "));
  display.println(ssid);
  display.display();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    timeClient.begin();
    timeClient.update();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 25);
    display.println(F("WiFi Connected!"));
    display.display();
    delay(1500);
  } else {
    Serial.println("\nWiFi connection failed!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 25);
    display.println(F("WiFi Failed!"));
    display.display();
    delay(2000);
  }
  strip.begin();
  strip.setBrightness(100);
  strip.show();
  
  startupAnimation();
  
  Serial.println("Khởi động xong!");
}

void loop() {
  static unsigned long lastNTPUpdate = 0;
  if(millis() - lastNTPUpdate >= 60000 || lastNTPUpdate == 0) {
    timeClient.update();
    lastNTPUpdate = millis();
  }
  handleButtons();
  
  static bool ledCleared = false;
  
  if(ledEnabled) {
    if(currentMode == RAINBOW) {
      rainbowCycle(10);
    } else {
      oneLedMode();
    }
    ledCleared = false;
  } else {
    if(!ledCleared) {
      for(int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
      strip.show();
      ledCleared = true;
    }
  }

  if(millis() - lastOLEDUpdate >= 1000) {
    lastOLEDUpdate = millis();
    updateMenuOLED();
  }
  
  if(millis() - lastScrollUpdate >= 50) {
    lastScrollUpdate = millis();
    scrollPosition -= 2;
    if(scrollPosition < -250) scrollPosition = 128;
  }
}

void startupAnimation() {
  Serial.println("Chạy hiệu ứng khởi động...");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("Led"));
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.println(F("Dang khoi dong..."));
  display.display();
  delay(1500);
  
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, Wheel((i * 256 / LED_COUNT) & 255));
    strip.show();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 20);
    display.print(F("Led Dang Khoi Dong"));
    for(int d = 0; d < (i % 4); d++) display.print(F("."));
    
    display.setCursor(35, 35);
    display.print(F("LED: "));
    display.print(i + 1);
    display.print(F("/16"));
    display.drawRect(14, 50, 100, 8, SSD1306_WHITE);
    display.fillRect(16, 52, (i + 1) * 6, 4, SSD1306_WHITE);
    display.display();
    
    delay(100);
  }
  
  delay(300);
  for(int i = LED_COUNT - 1; i >= 0; i--) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
    
    int progress = LED_COUNT - i;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(25, 20);
    display.print(F("Sap Xong Roi:))"));
    for(int d = 0; d < (progress % 4); d++) display.print(F("."));
    
    display.setCursor(30, 35);
    display.print(F("Progress: "));
    display.print(progress);
    display.print(F("/16"));
    display.drawRect(14, 50, 100, 8, SSD1306_WHITE);
    display.fillRect(16, 52, progress * 6, 4, SSD1306_WHITE);
    display.display();
    
    delay(100);
  }
  
  delay(300);
  
  display.clearDisplay(); 
  display.setTextSize(1.5);
  display.setCursor(52, 8);
  display.println(F("Tiktok"));
  display.setTextSize(1.5);
  display.setCursor(10, 25);
  display.println(F("Bidayne161"));
  display.setTextSize(1);
  display.setCursor(20, 48);
  display.println(F("Hethong SanSang"));
  display.display();
  delay(2000);
}

void updateMenuOLED() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println(F("Bi DANG CAP"));
  
  display.setCursor(0, 12);
  display.print(F("led mode: "));
  display.print(currentMode == RAINBOW ? "RAINBOW" : "ONELED ");
  
  if(currentMode == RAINBOW) {
    display.fillCircle(110, 15, 3, SSD1306_WHITE);
    display.drawCircle(118, 15, 3, SSD1306_WHITE);
  } else {
    display.fillRect(110, 12, 6, 6, SSD1306_WHITE);
  }
  
  display.setCursor(0, 24);
  display.print(F("led on/off: "));
  display.print(ledEnabled ? "ON " : "OFF");
  
  if(ledEnabled) {
    display.fillCircle(110, 27, 4, SSD1306_WHITE);
  } else {
    display.drawCircle(110, 27, 4, SSD1306_WHITE);
  }
  
  display.setTextSize(2);
  display.setCursor(15, 38);
  
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();
  
  char timeStr[9];
  sprintf(timeStr, "%02d.%02d.%02d", hours, minutes, seconds);
  display.print(timeStr);
  

  display.setTextSize(1);
  display.drawLine(0, 56, 128, 56, SSD1306_WHITE);
  display.setCursor(scrollPosition, 58);
  display.print(F("Code duoc thiet ke boi Bi"));
  
  display.display();
}

void rainbowCycle(uint8_t wait) {
  static uint16_t j = 0;
  
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / LED_COUNT) + j) & 255));
  }
  strip.show();
  delay(wait);
  
  j++;
  if(j >= 256) j = 0;
}

void oneLedMode() {
  static uint16_t colorOffset = 0;
  
  uint32_t color = Wheel(colorOffset & 255);
  
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
  delay(10);
  
  colorOffset++;
  if(colorOffset >= 256) colorOffset = 0;
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void handleButtons() {
  bool btnPressed = (digitalRead(BTN_MODE) == LOW);
  
  if(btnPressed && !btnWasPressed) {
    btnPressStartTime = millis();
    btnWasPressed = true;
  }
  
  if(!btnPressed && btnWasPressed) {
    unsigned long pressDuration = millis() - btnPressStartTime;
    
    if(pressDuration >= longPressTime) {
      toggleLED();
      Serial.print("LED toggled to: ");
      Serial.println(ledEnabled ? "ON" : "OFF");
    }
    else if(pressDuration >= debounceDelay) {
      if(ledEnabled) { 
        toggleMode();
        Serial.print("Mode changed to: ");
        Serial.println(currentMode == RAINBOW ? "RAINBOW" : "ONELED");
      }
    }
    
    btnWasPressed = false;
  }
}

void toggleMode() {
  if(currentMode == RAINBOW) {
    currentMode = ONELED;
  } else {
    currentMode = RAINBOW;
  }
}

void toggleLED() {
  ledEnabled = !ledEnabled;
}