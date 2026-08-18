// ================================================================
// Daikin FTV Timer IR Frames
// ================================================================
//
// These are complete raw IR frames captured from a Daikin FTV
// remote while configuring its built-in timer.
//
// IMPORTANT:
// These frames do NOT represent a generic "Timer ON" or
// "Timer OFF" command. The captured frames contain the remote's
// clock/time information as well as the timer configuration.
//
// Current captured frames:
//
// rawTimerOn:
//   - Sets the ON timer to 00:30
//   - Sets the remote clock to approximately 00:29
//
// rawTimerOff:
//   - Sets the OFF timer to 00:30
//   - Sets the remote clock to approximately 00:29
//
// The frames are replayed as raw 38 kHz IR timings using
// IRremoteESP8266.
//
// These values are specific to the captured remote state and
// are not yet dynamically configurable.
//
// ================================================================



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
#include <WiFiUdp.h>
#include <SNMP_Agent.h>
WiFiUDP udp;
SNMPAgent snmp("public","private");
const char* oidRoomTemp  = ".1.3.6.1.4.1.53864.2.1.0";
const char* oidHumidity  = ".1.3.6.1.4.1.53864.2.2.0";
const char* oidPower     = ".1.3.6.1.4.1.53864.2.5.0";
const char* oidRSSI      = ".1.3.6.1.4.1.53864.2.9.0";
const char* oidUptime    = ".1.3.6.1.4.1.53864.2.10.0";
int snmpRSSI;
int snmpRoomTemp;
int snmpHumidity;
int snmpPower;
uint32_t snmpUptime = 0;
const char* ssid = "ssid";
const char* password = "pass";

IPAddress local_IP(192, 168, 1, 240);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(1, 1, 1, 1);

// ======================
// Hardware
// ======================

const uint16_t IR_LED_PIN = 16;     // GPIO16 / D0
const uint16_t LED_PIN = LED_BUILTIN;
bool acPower = false;
IRsend irsend(IR_LED_PIN);

Adafruit_AHTX0 aht;
bool ahtAvailable = false;

ESP8266WebServer server(80);

// ======================
// Daikin FTV raw frames
// ======================

// Existing power toggle frame
const uint16_t rawToggle[133] = {4674, 2436,  434, 296,  458, 842,  458, 844,  458, 266,  438, 868,  458, 266,  
462, 270,  460, 268,  462, 264,  440, 868,  458, 266,  464, 266,  438, 294,  434, 868,  458, 266,  462, 268,  
462, 268,  460, 266,  438, 296,  434, 294,  434, 294,  458, 268,  438, 296,  458, 268,  460, 266,  438, 296,  
434, 296,  434, 294,  434, 294,  434, 296,  458, 266,  462, 266,  438, 296,  434, 294,  456, 270,  460, 268,  
462, 846,  458, 266,  438, 294,  434, 294,  458, 266,  462, 266,  462, 266,  462, 268,  462, 844,  458, 268,  
438, 296,  432, 296,  458, 270,  460, 266,  438, 868,  458, 266,  438, 296,  458, 842,  458, 268,  438, 296,  
456, 846,  432, 296,  434, 294,  456, 846,  456, 268,  462, 266,  438, 870,  456, 844,  458, 20174,  4698};  


// Timer OFF frame
const uint16_t rawTimerOff[133] = {4692, 2412,  456, 270,  458, 846,  456, 846,  454, 270,  458, 846,  454, 268,  460, 270,  458, 270,  458, 270,  460, 846,  454, 
272,  458, 270,  458, 270,  458, 846,  454, 272,  456, 272,  458, 
846,  456, 270,  458, 270,  458, 846,  454, 270,  458, 848,  454, 270,  458, 270,  458, 270,  458, 272,  432, 298,  
454, 272,  456, 274,  456, 270,  432, 298,  430, 300,  454, 272,  432, 298,  430, 300,  452, 274,  432, 870,  430, 
298,  430, 296,  430, 298,  432, 298,  430, 298,  430, 300,  428, 298,  430, 298,  428, 298,  430, 872,  428, 872,  
428, 300,  428, 298,  430, 872,  428, 300,  428, 298,  428, 874,  428, 298,  430, 298,  430, 298,  430, 300,  428, 300,  430, 
298,  428, 874,  428, 300,  430, 298,  430, 872,  428, 20190,  4690}; 

// Timer ON frame
const uint16_t rawTimerOn[133] = {4634, 2474,  396, 330,  392, 910,  392, 910,  390, 338,  394, 908,  
392, 338,  396, 332,  388, 340,  394, 336,  360, 940,  392, 334,  392, 338,  402, 326,  396, 906,  398, 330,  
400, 328,  396, 906,  400, 328,  394, 334,  400, 902,  394, 314,  384, 938,  398, 332,  390, 338,  400, 326,  392, 340,  396, 
332,  404, 326,  396, 330,  392, 338,  394, 334,  398, 330,  392, 336,  394, 334,  396, 334,  388, 338,  402, 328,  396, 332,  396, 906,  402, 898,  364, 364,  
390, 340,  406, 322,  394, 334,  386, 914,  364, 364,  388, 342,  396, 330,  394, 336,  398, 332,  398, 904,  408, 322,  390, 338,  396, 906,  426, 302,  406, 322,  
390, 912,  400, 328,  424, 304,  400, 328,  398, 330,  396, 906,  400, 328,  400, 902,  398, 20222,  4624}; 

// ======================
// Sensor values
// ======================

float roomTemp = 0.0;
float roomHumidity = 0.0;

unsigned long lastSensorRead = 0;

// ======================
// Generic raw IR sender
// ======================

void sendRawFrame(const uint16_t *data, size_t length, const char *name) {

  Serial.print("Sending ");
  Serial.println(name);

  digitalWrite(LED_PIN, LOW);


  irsend.sendRaw(data, length, 38);


  digitalWrite(LED_PIN, HIGH);

  Serial.println("IR transmission complete.");
}

// ======================
// Power
// ======================

void sendPowerToggle() {

  sendRawFrame(
    rawToggle,
    sizeof(rawToggle) / sizeof(rawToggle[0]),
    "Daikin POWER TOGGLE"
  );
}

// ======================
// Timer ON
// ======================

void sendTimerOn() {

  sendRawFrame(
    rawTimerOn,
    sizeof(rawTimerOn) / sizeof(rawTimerOn[0]),
    "Daikin TIMER ON"
  );
}

// ======================
// Timer OFF
// ======================

void sendTimerOff() {

  sendRawFrame(
    rawTimerOff,
    sizeof(rawTimerOff) / sizeof(rawTimerOff[0]),
    "Daikin TIMER OFF"
  );
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

<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="2">

<title>Main Room AC</title>

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

button {
    width: 100%;
    height: 60px;
    border: none;
    border-radius: 8px;
    font-size: 21px;
    background: #0078d7;
    color: white;
    cursor: pointer;
    margin-bottom: 12px;
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
Main Room AC
</div>

<div class="container">

<div class="card">

<div class="sectionTitle">
Daikin Air Conditioner
</div>
<div class="status">
<span>Power</span>
<span class="value">Power: %%Power%% </span>
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

<div class="card">


<form action="/timerOn">
<button>
Power ON
</button>
</form>

<form action="/timerOff">
<button>
Power OFF
</button>
</form>

<form action="/power">
<button>
Manual Power Toggle
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
html.replace(
      "%%Power%%",
      String(acPower ? "ON" : "OFF"));
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

    html.replace("%%ROOMTEMP%%", "N/A");
    html.replace("%%HUMIDITY%%", "N/A");

  }

  server.send(200, "text/html", html);
}

// ======================
// Web handlers
// ======================

void handlePower() {
  acPower = !acPower;
  sendPowerToggle();

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleTimerOn() {

  sendTimerOn();
  acPower = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleTimerOff() {
  acPower = false;
  sendTimerOff();

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
  server.on("/timerOn", handleTimerOn);
  server.on("/timerOff", handleTimerOff);

  server.begin();
  
  Serial.println("HTTP server started.");
      snmp.setUDP(&udp);

snmp.begin();
snmp.addIntegerHandler(oidRoomTemp,&snmpRoomTemp);
snmp.addIntegerHandler(oidHumidity,&snmpHumidity);
snmp.addIntegerHandler(oidPower,&snmpPower);
snmp.addTimestampHandler(oidUptime,&snmpUptime);
snmp.addIntegerHandler(oidRSSI,&snmpRSSI);
snmp.sortHandlers();
}

// ======================
// Loop
// ======================

void loop() {

  server.handleClient();

  if (millis() - lastSensorRead > 2000) {

    readAHT();

    lastSensorRead = millis();

    Serial.print("Room: ");
    Serial.print(roomTemp, 1);
    Serial.print(" C | Humidity: ");
    Serial.print(roomHumidity, 1);
    Serial.println(" %");

  }
    updateSNMP();

    snmp.loop();
}

void updateSNMP() {

    snmpRoomTemp = roomTemp * 10;
    snmpHumidity = roomHumidity * 10;
    snmpPower     = acPower;
    snmpRSSI = WiFi.RSSI();
    snmpUptime = millis()/10;      // TimeTicks are 1/100 second
}