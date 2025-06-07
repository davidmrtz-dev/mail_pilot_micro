#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int scrollX = SCREEN_WIDTH;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
}

void loop() {
  display.clearDisplay();

  const char* text = "Scrolling Hello";
  int y = (SCREEN_HEIGHT - 16) / 2;
  display.setCursor(scrollX, y);
  display.print(text);
  display.display();

  scrollX--;

  int textWidth = strlen(text) * 6 * 2;
  if (scrollX < -textWidth) {
    scrollX = SCREEN_WIDTH;
  }

  delay(20);
}
