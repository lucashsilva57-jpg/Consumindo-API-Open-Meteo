#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Credenciais da rede Wi-Fi
const char* ssid = "POCO X6 Pro 5G";
const char* password = "leonardo";

WebServer server(80);

// Variáveis do clima
float currentTemp = 20;
float currentWind = 13;
float currentWindDirection = 0;

void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // API Open-Meteo para Barueri
    String apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=-23.5106&longitude=-46.8761&current_weather=true";

    if (http.begin(client, apiUrl)) {
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // Temperatura, velocidade e direção do vento
          currentTemp = doc["current_weather"]["temperature"];
          currentWind = doc["current_weather"]["windspeed"];
          currentWindDirection = doc["current_weather"]["winddirection"];

          Serial.printf(
            "Temperatura: %.1f C | Vento: %.1f km/h | Direcao: %.0f graus\n",
            currentTemp,
            currentWind,
            currentWindDirection
          );
        } else {
          Serial.print("Falha ao ler JSON: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.printf("Erro HTTP: %d\n", httpCode);
      }

      http.end();
    }
  }
}

void handleRoot() {
  fetchWeatherData();

  String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Painel Climático - ESP32</title>";

  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #0f172a; color: #f8fafc; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; }";
  html += ".card { background: #1e293b; padding: 2rem; border-radius: 12px; text-align: center; max-width: 400px; width: 90%; border: 1px solid #334155; box-shadow: 0 10px 25px rgba(0,0,0,0.3); }";
  html += "h1 { color: #38bdf8; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 1.5rem; }";
  html += ".metric { font-size: 2.5rem; font-weight: bold; color: #f1f5f9; margin: 10px 0; }";
  html += ".sub-metric { font-size: 1.1rem; color: #94a3b8; margin: 10px 0; }";
  html += ".btn { background: #38bdf8; color: #0f172a; border: none; padding: 12px 24px; font-weight: bold; border-radius: 6px; cursor: pointer; transition: 0.2s; margin-top: 15px; }";
  html += ".btn:hover { background: #0284c7; color: #ffffff; }";
  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "<h1>Clima em Tempo Real (Barueri)</h1>";

  // Temperatura
  html += "<div class='metric'>";
  html += String(currentTemp, 1) + " &deg;C";
  html += "</div>";

  // Velocidade do vento
  html += "<div class='sub-metric'>";
  html += "Velocidade do vento: " + String(currentWind, 1) + " km/h";
  html += "</div>";

  // Direção do vento, abaixo da velocidade
  html += "<div class='sub-metric'>";
  html += "Direção do vento: " + String(currentWindDirection, 0) + "&deg;";
  html += "</div>";

  html += "<button class='btn' onclick='location.reload()'>Atualizar Dados</button>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP do ESP32: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Servidor HTTP iniciado com sucesso.");
}

void loop() {
  server.handleClient();
}