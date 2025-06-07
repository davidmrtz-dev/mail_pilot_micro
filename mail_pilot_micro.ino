#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "xxxx";
const char* password = "xxxx";
const char* API_BASE = "xxxx";

const int LED_PIN = 2;
const unsigned long POLL_INTERVAL = 3000;

unsigned long lastPollTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  connectToWiFi();
}

void loop() {
  if (millis() - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = millis();
    handleCommandPolling();
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
  }

  Serial.println("❌ Comando no reconocido");
  return false;
}
