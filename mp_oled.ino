#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TEXT_BUFFER_SIZE 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

char currentText[TEXT_BUFFER_SIZE] = "Esperando texto...";
int scrollX = SCREEN_WIDTH;

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
}

void scrollText(const char* text) {
  display.clearDisplay();

  int y = (SCREEN_HEIGHT - 32) / 2;
  display.setCursor(scrollX, y);
  display.print(text);
  display.display();

  scrollX--;

  int textWidth = strlen(text) * 6 * 2;
  if (scrollX < -textWidth) {
    scrollX = SCREEN_WIDTH;
  }
}

void checkSerialInput() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0 && input.length() < TEXT_BUFFER_SIZE) {
      input.toCharArray(currentText, TEXT_BUFFER_SIZE);
      scrollX = SCREEN_WIDTH; // Reinicia el scroll
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupDisplay();
}

void loop() {
  checkSerialInput();
  scrollText(currentText);
  delay(2);
}
