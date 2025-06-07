#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TEXT_BUFFER_SIZE 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "xxxx";
const char* password = "xxxx";
const char* API_BASE = "xxxxx";

Servo doorServo;
const int SERVO_PIN = D5;
const int LED_PIN = D4;
const unsigned long POLL_INTERVAL = 3000;

unsigned long lastPollTime = 0;
char currentText[TEXT_BUFFER_SIZE] = "Esperando texto...";
int scrollX = SCREEN_WIDTH;

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
}

void setupServo(){
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);
}

void scrollText(const char* text) {
  display.clearDisplay();
  int y = (SCREEN_HEIGHT - 24) / 2;
  display.setCursor(scrollX, y);
  display.print(text);
  display.display();

  scrollX--;

  int textWidth = strlen(text) * 12; // 6 * 2 textSize
  if (scrollX < -textWidth) {
    scrollX = SCREEN_WIDTH;
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi conectado");
}

void processCommandPayload(const String& payload) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.println("⚠️ Error al parsear JSON");
    return;
  }

  const char* command = doc["command"];
  int id = doc["id"];

  if (command && id) {
    Serial.print("🟡 Comando recibido: ");
    Serial.println(command);

    bool success = executeCommand(String(command));
    confirmCommandStatus(id, success);
  }
}

void handleCommandPolling() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado, reintentando...");
    WiFi.reconnect();
    return;
  }

  String payload;
  if (fetchLatestCommand(payload)) {
    processCommandPayload(payload);
  }
}

bool fetchLatestCommand(String& payload) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, String(API_BASE) + "/device/commands/latest");

  int responseCode = http.GET();
  if (responseCode == 200) {
    payload = http.getString();
    http.end();
    return true;
  } else {
    Serial.println("⚠️ Error HTTP: " + String(responseCode));
    http.end();
    return false;
  }
}

void confirmCommandStatus(int id, bool success) {
  String confirmUrl = String(API_BASE) + "/device/commands/" + String(id) + (success ? "/complete" : "/failed");

  WiFiClient client;
  HTTPClient http;
  http.begin(client, confirmUrl);
  http.addHeader("Content-Type", "application/json");

  int resultCode = http.PATCH("");
  Serial.print("📤 Confirmación enviada: ");
  Serial.println(resultCode);
  http.end();
}

bool executeCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command == "LED_ON") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("✅ LED ON");
    return true;
  } else if (command == "LED_OFF") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("✅ LED OFF");
    return true;
  } else if (command.startsWith("DISPLAY:")) {
    String text = command.substring(8); // lo que viene después de "DISPLAY:"
    text.toCharArray(currentText, TEXT_BUFFER_SIZE);
    scrollX = SCREEN_WIDTH; // reiniciar scroll
    Serial.println("🖥️ Mostrando en pantalla: " + text);
    return true;
  } else if (command == "DOOR_OPEN") {
    doorServo.write(180);
    Serial.println("🚪 Puerta abierta");
    return true;
  } else if (command == "DOOR_CLOSE") {
    doorServo.write(0);
    Serial.println("🚪 Puerta cerrada");
    return true;
  }

  Serial.println("❌ Comando no reconocido");
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  connectToWiFi();
  setupDisplay();
  setupServo();
}

void loop() {
  if (millis() - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = millis();
    handleCommandPolling();
  }

  scrollText(currentText);
  delay(2);
}
