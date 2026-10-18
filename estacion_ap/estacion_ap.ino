#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <WebServer.h>

DHT dht(26, DHT11);
Adafruit_BMP085 bmp;

const char* nombreAP     = "Estacion-Galleguillo";
const char* contraseniaAP = "clima1234";

WebServer server(80);

unsigned long ultimoPrint = 0;

void handleRoot() {
  float temp     = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure() / 100.0;   
  float altitude = bmp.readAltitude();

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="2">
<title>Estación Meteorológica Galleguillo</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  body {
    min-height: 100vh;
    font-family: -apple-system, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    background: linear-gradient(135deg, #1e1b4b 0%, #312e81 45%, #0f766e 100%);
    color: #f1f5f9;
    display: flex; flex-direction: column; align-items: center;
    padding: 40px 20px;
  }
  header { text-align: center; margin-bottom: 36px; }
  header h1 { font-size: 2rem; font-weight: 700; letter-spacing: 0.5px; }
  header p { opacity: 0.7; margin-top: 6px; font-size: 0.95rem; }
  .live {
    display: inline-block; width: 9px; height: 9px; border-radius: 50%;
    background: #4ade80; margin-right: 6px;
    box-shadow: 0 0 8px #4ade80; animation: pulse 1.5s infinite;
  }
  @keyframes pulse { 50% { opacity: 0.3; } }
  .grid {
    display: grid; gap: 20px; width: 100%; max-width: 720px;
    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  }
  .card {
    background: rgba(255, 255, 255, 0.08);
    border: 1px solid rgba(255, 255, 255, 0.15);
    border-radius: 20px; padding: 24px 18px; text-align: center;
    backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px);
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.25);
    transition: transform 0.2s;
  }
  .card:hover { transform: translateY(-4px); }
  .icon { font-size: 2.2rem; }
  .label {
    font-size: 0.8rem; text-transform: uppercase; letter-spacing: 1.5px;
    opacity: 0.7; margin: 10px 0 6px;
  }
  .value { font-size: 1.9rem; font-weight: 700; }
  .unit { font-size: 1rem; font-weight: 400; opacity: 0.7; margin-left: 2px; }
  footer { margin-top: 40px; font-size: 0.8rem; opacity: 0.5; }
</style>
</head>
<body>
  <header>
    <h1>🌦️ Estación Meteorológica</h1>
    <p><span class="live"></span>Galleguillo · en vivo</p>
  </header>
  <div class="grid">
    <div class="card">
      <div class="icon">🌡️</div>
      <div class="label">Temperatura</div>
      <div class="value">%TEMP%<span class="unit">°C</span></div>
    </div>
    <div class="card">
      <div class="icon">🧭</div>
      <div class="label">Presión</div>
      <div class="value">%PRES%<span class="unit">hPa</span></div>
    </div>
    <div class="card">
      <div class="icon">💧</div>
      <div class="label">Humedad</div>
      <div class="value">%HUM%<span class="unit">%</span></div>
    </div>
    <div class="card">
      <div class="icon">⛰️</div>
      <div class="label">Altitud</div>
      <div class="value">%ALT%<span class="unit">m</span></div>
    </div>
  </div>
  <footer>Actualiza cada 2 segundos</footer>
</body>
</html>
)rawliteral";

  html.replace("%TEMP%", isnan(temp)     ? String("--") : String(temp, 1));
  html.replace("%PRES%", String(pressure, 1));
  html.replace("%HUM%",  isnan(humidity) ? String("--") : String(humidity, 1));
  html.replace("%ALT%",  String(altitude, 0));
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("No se encontró el BMP180/BMP085, revisá el cableado");
  }
  delay(2000);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(nombreAP, contraseniaAP);

  Serial.println("Red WiFi creada: " + String(nombreAP));
  Serial.println("Entrá desde el navegador a: http://" + WiFi.softAPIP().toString());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();  


  if (millis() - ultimoPrint >= 2000) {
    ultimoPrint = millis();
    Serial.print("Temp: ");      Serial.print(dht.readTemperature());
    Serial.print(" C  Humedad: "); Serial.print(dht.readHumidity());
    Serial.print(" %  Presion: "); Serial.print(bmp.readPressure() / 100.0);
    Serial.print(" hPa  Altitud: "); Serial.print(bmp.readAltitude());
    Serial.println(" m");
  }
}
