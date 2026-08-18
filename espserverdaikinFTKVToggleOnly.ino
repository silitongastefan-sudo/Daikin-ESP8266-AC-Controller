#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>

// ======================
// WiFi
// ======================

const char* ssid = "SSID";
const char* password = "PASS";

IPAddress local_IP(192, 168, 1, 240);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(1, 1, 1, 1);

// ======================
// Hardware
// ======================

const uint16_t IR_LED_PIN = 14;     // GPIO14 / D5
const uint16_t LED_PIN = LED_BUILTIN;

IRsend irsend(IR_LED_PIN);

Adafruit_AHTX0 aht;
bool ahtAvailable = false;

ESP8266WebServer server(80);

// ======================
// Daikin FTV raw toggle
// ======================

const uint16_t rawToggle[] = {
  4678, 2432,
  438, 292, 440, 862, 436, 866, 438, 290,
  438, 864, 436, 290, 438, 292, 438, 290,
  436, 292, 438, 866, 438, 290, 438, 292,
  436, 292, 438, 866, 436, 294, 436, 294,
  438, 292, 438, 292, 438, 866, 436, 292,
  436, 866, 438, 290, 438, 290, 438, 290,
  438, 290, 436, 292, 438, 292, 438, 292,
  438, 290, 438, 292, 436, 292, 438, 292,
  438, 290, 436, 292, 436, 292, 436, 866,
  436, 290, 438, 292, 438, 292, 436, 292,
  436, 292, 436, 292, 438, 292, 438, 864,
  436, 292, 438, 290, 436, 292, 436, 292,
  438, 290, 438, 864, 436, 292, 438, 290,
  438, 864, 436, 292, 436, 292, 434, 866,
  436, 292, 438, 290, 436, 864, 436, 866,
  434, 294, 436, 294, 436, 292, 436, 20192,
  4700
};

// ======================
// Sensor values
// ======================

float roomTemp = 0.0;
float roomHumidity = 0.0;

unsigned long lastSensorRead = 0;

// ======================
// Send IR
// ======================

void sendPowerToggle() {

  Serial.println("Sending Daikin FTV POWER TOGGLE...");

  // Blink built-in LED
  digitalWrite(LED_PIN, LOW);

  // Send 3 times for reliability
  for (int i = 0; i < 3; i++) {

    irsend.sendRaw(
      rawToggle,
      sizeof(rawToggle) / sizeof(rawToggle[0]),
      38
    );

    delay(120);
  }

  digitalWrite(LED_PIN, HIGH);

  Serial.println("IR transmission complete.");
}

// ======================
// Read AHT10
// ======================

void readAHT() {

  if (!ahtAvailable)
    return;

  sensors_event_t humidity, temp;

  aht.getEvent(&humidity, &temp);

  roomTemp = temp.temperature;
  roomHumidity = humidity.relative_humidity;
}

// ======================
// Web page
// ======================
void handleRoot() {

  readAHT();

  String html = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta http-equiv="refresh" content="2">
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>Stefan AC</title>

<style>

body {
    margin: 0;
    background: #1c1f26;
    color: white;
    font-family: Arial, Helvetica, sans-serif;
}

.header {
    background: #11161d;
    color: #00bfff;
    padding: 20px;
    text-align: center;
    font-size: 30px;
    font-weight: bold;
    border-bottom: 2px solid #00bfff;
}

.container {
    width: 95%;
    max-width: 850px;
    margin: auto;
    padding: 20px;
}

.card {
    background: #2b313a;
    border: 1px solid #555;
    border-radius: 12px;
    padding: 20px;
    margin-bottom: 20px;
}

.sectionTitle {
    color: #00d0ff;
    font-size: 22px;
    margin-bottom: 15px;
    font-weight: bold;
}

.status {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    border-bottom: 1px solid #444;
    font-size: 18px;
}

.status:last-child {
    border-bottom: none;
}

.value {
    color: #00ff80;
    font-weight: bold;
}

.powerCard {
    display: flex;
    justify-content: center;
}

.powerCard form {
    width: 100%;
}

button {
    width: 100%;
    height: 60px;
    border: none;
    border-radius: 8px;
    font-size: 21px;
    background: #0078d7;
    color: white;
    cursor: pointer;
}

button:hover {
    background: #1090ff;
}

button:active {
    background: #005fa8;
}

.info {
    text-align: center;
    color: #aaa;
    margin-top: 12px;
    font-size: 16px;
}

</style>

</head>

<body>

<div class="header">
Stefan AC
</div>

<div class="container">

<!-- ================= STATUS ================= -->

<div class="card">

<div class="sectionTitle">
Daikin Air Conditioner
</div>

<div class="status">
<span>Room Temperature</span>
<span class="value">%%ROOMTEMP%% &deg;C</span>
</div>

<div class="status">
<span>Room Humidity</span>
<span class="value">%%HUMIDITY%% % RH</span>
</div>

<div class="status">
<span>Configuration</span>
<span class="value">24&deg;C / Fan 3</span>
</div>

</div>


<!-- ================= POWER ================= -->

<div class="card powerCard">

<form action="/power">

<button>
POWER ON / OFF
</button>

</form>

</div>

<div class="info">
IR remote control &bull; Daikin FTV Series
</div>

</div>

</body>

</html>
)rawliteral";


  if (ahtAvailable) {

    html.replace(
      "%%ROOMTEMP%%",
      String(roomTemp, 1)
    );

    html.replace(
      "%%HUMIDITY%%",
      String(roomHumidity, 1)
    );

  } else {

    html.replace(
      "%%ROOMTEMP%%",
      "N/A"
    );

    html.replace(
      "%%HUMIDITY%%",
      "N/A"
    );

  }

  server.send(200, "text/html", html);
}
// ======================
// Power button
// ======================

void handlePower() {

  sendPowerToggle();

  server.sendHeader("Location", "/");
  server.send(303);
}

// ======================
// Setup
// ======================

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // IR
  irsend.begin();

  Serial.println();
  Serial.println("================================");
  Serial.println(" Daikin FTV IR Controller");
  Serial.println("================================");

  // AHT10
  Wire.begin();

  if (aht.begin()) {

    Serial.println("AHT10 initialized.");
    ahtAvailable = true;

  } else {

    Serial.println("AHT10 NOT FOUND!");
    ahtAvailable = false;

  }

  // WiFi
  WiFi.mode(WIFI_STA);

  WiFi.config(
    local_IP,
    gateway,
    subnet,
    primaryDNS,
    secondaryDNS
  );

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");

  }

  Serial.println();
  Serial.println("WiFi connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Web server
  server.on("/", handleRoot);
  server.on("/power", handlePower);

  server.begin();

  Serial.println("HTTP server started.");
}

// ======================
// Loop
// ======================

void loop() {

  server.handleClient();

  // Update sensor every 2 seconds
  if (millis() - lastSensorRead > 2000) {

    readAHT();

    lastSensorRead = millis();

    Serial.print("Room: ");
    Serial.print(roomTemp, 1);
    Serial.print(" C | Humidity: ");
    Serial.print(roomHumidity, 1);
    Serial.println(" %");

  }
}
