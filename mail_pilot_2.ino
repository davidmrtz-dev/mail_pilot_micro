#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "Mega_2.4G_DE0A";
const char* password = "RRPbGYxf";
const char* API_BASE = "http://gannet-trusted-ray.ngrok-free.app";

Servo doorServo;
const int SERVO_PIN = D5;
const int LED_PIN = D4;
const unsigned long POLL_INTERVAL = 3000;

unsigned long lastPollTime = 0;

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
    showMessage("LED ON");
    return true;
  } else if (command == "LED_OFF") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("✅ LED OFF");
    showMessage("LED OFF");
    return true;
  } else if (command == "DOOR_OPEN") {
    doorServo.write(180);
    Serial.println("🚪 Puerta abierta");
    showMessage("DOOR OPENED");
    return true;
  } else if (command == "DOOR_CLOSE") {
    doorServo.write(0);
    Serial.println("🚪 Puerta cerrada");
    showMessage("DOOR CLOSED");
    return true;
  }

  Serial.println("❌ Comando no reconocido");
  showMessage("❌ Comando invalido");
  return false;
}

void showMessage(const String& message) {
  int textWidth = message.length() * 12; // 6 px * text size 2
  int x = SCREEN_WIDTH;

  while (x > -textWidth) {
    display.clearDisplay();
    display.setCursor(x, (SCREEN_HEIGHT - 24) / 2);
    display.print(message);
    display.display();

    x--;
    delay(2); // velocidad de scroll
  }

  display.clearDisplay();
  display.display();
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

  delay(2);
}
