#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Mega_2.4G_DE0A";
const char* password = "RRPbGYxf";
const char* API_BASE = "http://gannet-trusted-ray.ngrok-free.app";

const int LED_PIN = LED_BUILTIN;
const unsigned long POLL_INTERVAL = 3000;
unsigned long lastPollTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi conectado");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, String(API_BASE) + "/device/commands/latest");

      int httpResponseCode = http.GET();

      if (httpResponseCode == 200) {
        String payload = http.getString();

        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, payload);

        if (!err) {
          const char* command = doc["command"];
          int id = doc["id"];

          if (command && id) {
            Serial.print("🟡 Comando recibido: ");
            Serial.println(command);

            bool success = executeCommand(String(command));

            String confirmUrl = String(API_BASE) + "/device/commands/" + String(id) + (success ? "/complete" : "/failed");
            WiFiClient confirmClient;
            HTTPClient confirm;
            confirm.begin(confirmClient, confirmUrl);
            confirm.addHeader("Content-Type", "application/json");
            int resultCode = confirm.PATCH("");
            Serial.print("📤 Confirmación enviada: ");
            Serial.println(resultCode);
            confirm.end();
          }
        } else {
          Serial.println("⚠️ Error al parsear JSON");
        }
      } else {
        Serial.println("⚠️ Error HTTP: " + String(httpResponseCode));
      }

      http.end();
    } else {
      Serial.println("⚠️ WiFi desconectado, reintentando...");
      WiFi.reconnect();
    }
  }
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
