#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
// ======================
// WiFi Configuration
// ======================
#include <WiFiUdp.h>
#include <SNMP_Agent.h>

WiFiUDP udp;
SNMPAgent snmp("public","private");
const char* oidRoomTemp  = ".1.3.6.1.4.1.53864.2.1.0";
const char* oidHumidity  = ".1.3.6.1.4.1.53864.2.2.0";
const char* oidSetpoint  = ".1.3.6.1.4.1.53864.2.3.0";
const char* oidFan       = ".1.3.6.1.4.1.53864.2.4.0";
const char* oidPower     = ".1.3.6.1.4.1.53864.2.5.0";
const char* oidPowerful  = ".1.3.6.1.4.1.53864.2.6.0";
const char* oidComfort   = ".1.3.6.1.4.1.53864.2.7.0";
const char* oidSwing     = ".1.3.6.1.4.1.53864.2.8.0";
const char* oidRSSI      = ".1.3.6.1.4.1.53864.2.9.0";
const char* oidUptime    = ".1.3.6.1.4.1.53864.2.10.0";
int snmpRoomTemp;
int snmpHumidity;
int snmpSetpoint;
int snmpFan;
int snmpPower;
int snmpPowerful;
int snmpComfort;
int snmpSwing;
int snmpRSSI;
uint32_t snmpUptime = 0;

void toggleSwing();
void toggleComfort();
void handleRoot();
void sendAC();
void fanUp();
void fanDown();
void tempUp();
void tempDown();
void onAuto();
void powerful();
void off();
const char* ssid = "ssid";
const char* password = "password";

IPAddress local_IP(192,168,1,250);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secondaryDNS(1,1,1,1);

// ======================
// Hardware
// ======================
Adafruit_AHTX0 aht;
const uint16_t kIrLed = D5;     // GPIO14

IRDaikinESP ac(kIrLed);

ESP8266WebServer server(80);

// ======================
// AC State
// ======================
float roomTemp = 0.0;
float roomHumidity = 0.0;
bool acPower = false;
bool acPowerful = false;
bool acSwing = true;
bool acComfort = false;
uint8_t acTemp = 25;
bool ahtAvailable = false;
// Daikin fan speeds
// 1 = Quiet
// 2~5 = Fan 1~4
// kDaikinFanAuto = Auto

uint8_t acFan = kDaikinFanAuto;
// ======================
// Helper Functions
// ======================
String stateText(bool s){
    return s ?
    "<span style='color:lime;'>ON</span>" :
    "<span style='color:red;'>OFF</span>";
}

String fanText() {

    if(acFan == kDaikinFanAuto)
        return "AUTO";

    switch(acFan){

    case 1: return "QUIET";
    case 2: return "FAN 1";
    case 3: return "FAN 2";
    case 4: return "FAN 3";
    case 5: return "FAN 4";
    }

    return "?";
}
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

body{
    margin:0;
    background:#1c1f26;
    color:white;
    font-family:Arial,Helvetica,sans-serif;
}

.header{
    background:#11161d;
    color:#00bfff;
    padding:20px;
    text-align:center;
    font-size:30px;
    font-weight:bold;
    border-bottom:2px solid #00bfff;
}

.container{
    width:95%;
    max-width:850px;
    margin:auto;
    padding:20px;
}

.card{
    background:#2b313a;
    border:1px solid #555;
    border-radius:12px;
    padding:20px;
    margin-bottom:20px;
}

.sectionTitle{
    color:#00d0ff;
    font-size:22px;
    margin-bottom:15px;
    font-weight:bold;
}
.powerCard{

    display:flex;
    justify-content:center;
    flex-direction:Row;
    gap:12px;

}

.powerCard form{

    flex:1;

}
.status{
    display:flex;
    justify-content:space-between;
    padding:8px 0;
    border-bottom:1px solid #444;
    font-size:18px;
}

.value{
    color:#00ff80;
    font-weight:bold;
}

.row{
    display:flex;
    gap:10px;
    margin-top:10px;
}

.row form{
    flex:1;
}

button{
    width:100%;
    height:48px;
    border:none;
    border-radius:8px;
    font-size:18px;
    background:#0078d7;
    color:white;
    cursor:pointer;
}

button:hover{
    background:#1090ff;
}

</style>

</head>
<body>

<div class="header">
Stefan AC
</div>

<div class="container">

<div class="dashboard">

<!-- ================= STATUS ================= -->

<div class="card statusCard">

<div class="sectionTitle">
Daikin Air Conditioner
</div>

<div class="status">
<span>Power</span>
<span class="value">%%STATUS%%</span>
</div>

<div class="status">
<span>Powerful</span>
<span class="value">%%POWERFUL%%</span>
</div>

<div class="status">
<span>Comfort</span>
<span class="value">%%COMFORT%%</span>
</div>

<div class="status">
<span>Vertical Swing</span>
<span class="value">%%SWING%%</span>
</div>

<div class="status">
<span>Room Temp</span>
<span class="value">%%ROOMTEMP%% C</span>
</div>

<div class="status">
<span>Room Humidity</span>
<span class="value">%%HUMIDITY%% %</span>
</div>

<div class="status">
<span>Fan</span>
<span class="value">%%FAN%%</span>
</div>

</div>


<!-- ================= POWER ================= -->

<div class="card powerCard">




<form action="/on">
<button>ON</button>
</form>



<form action="/off">
<button>OFF</button>
</form>

</div>


<!-- ================= TEMP ================= -->

<div class="card tempCard">

<div class="sectionTitle">
Temperature
</div>

<div class="bigValue">

%%TEMP%% C

</div>

<div class="row">

<form action="/tempDown">
<button>-</button>
</form>

<form action="/tempUp">
<button>+</button>
</form>

</div>

</div>

</div>


<!-- ================= FAN ================= -->

<div class="card">

<div class="sectionTitle">
Fan Speed

</div>

<div class="bigValue">

%%FAN%%

</div>

<div class="row">

<form action="/fanDown">
<button>-</button>
</form>

<form action="/fanUp">
<button>+</button>
</form>

</div>

</div>


<!-- ================= FEATURES ================= -->

<div class="card">

<div class="sectionTitle">
Features
</div>

<div class="row">

<form action="/swing">
<button>Swing</button>
</form>
<form action="/powerful">
<button>POWERFUL</button>
</form>
<form action="/comfort">
<button>Comfort</button>
</form>

</div>

</div>

</div>

</body></html>

)rawliteral";

    html.replace("%%STATUS%%", stateText(acPower));
    html.replace("%%POWERFUL%%", stateText(acPowerful));
    html.replace("%%COMFORT%%", stateText(acComfort));
    html.replace("%%SWING%%", stateText(acSwing));
    html.replace("%%TEMP%%", String(acTemp));
    html.replace("%%FAN%%", fanText());
    if(ahtAvailable){
    html.replace("%%ROOMTEMP%%", String(roomTemp,1));
    html.replace("%%HUMIDITY%%", String(roomHumidity,1));
}
    else{
    html.replace("%%ROOMTEMP%%", "N/A");
    html.replace("%%HUMIDITY%%", "N/A");
}
    server.send(200, "text/html", html);
}
void setup() {

    Serial.begin(115200);

    ac.begin();
Wire.begin();

if (aht.begin()) {
    Serial.println("AHT10 initialized.");
    ahtAvailable = true;
} else {
    Serial.println("AHT10 not found!");
    ahtAvailable = false;
}
    WiFi.mode(WIFI_STA);
    WiFi.config(
        local_IP,
        gateway,
        subnet,
        primaryDNS,
        secondaryDNS
    );

    WiFi.begin(ssid,password);

    Serial.print("Connecting");

    while(WiFi.status()!=WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    server.on("/setTemp", setTemp);
    server.on("/", handleRoot);
    server.on("/on", onAuto);
    server.on("/powerful", powerful);
    server.on("/off", off);
    server.on("/tempUp", tempUp);
    server.on("/tempDown", tempDown);
    server.on("/swing", toggleSwing);
server.on("/comfort", toggleComfort);
    server.on("/fanUp", fanUp);
    server.on("/fanDown", fanDown);
    server.on("/setFan", setFan);
    server.begin();

    Serial.println("HTTP Server Started");
    snmp.setUDP(&udp);

snmp.begin();
snmp.addIntegerHandler(oidRoomTemp,&snmpRoomTemp);
snmp.addIntegerHandler(oidHumidity,&snmpHumidity);
snmp.addIntegerHandler(oidSetpoint,&snmpSetpoint);
snmp.addIntegerHandler(oidFan,&snmpFan);
snmp.addIntegerHandler(oidPower,&snmpPower);
snmp.addIntegerHandler(oidPowerful,&snmpPowerful);
snmp.addIntegerHandler(oidComfort,&snmpComfort);
snmp.addIntegerHandler(oidSwing,&snmpSwing);
snmp.addIntegerHandler(oidRSSI,&snmpRSSI);

snmp.addTimestampHandler(oidUptime,&snmpUptime);

snmp.sortHandlers();
}



unsigned long lastRead = 0;

void loop() {

    server.handleClient();

    if (millis() - lastRead > 2000) {

        readAHT();

        lastRead = millis();

    }

    updateSNMP();

    snmp.loop();
}

void sendAC() {

    if (acPower)
        ac.on();
    else
        ac.off();

    ac.setMode(kDaikinCool);

    ac.setTemp(acTemp);

    ac.setFan(acFan);

ac.setPowerful(acPowerful);

ac.setComfort(acComfort);

ac.setSwingVertical(acSwing);

    Serial.println(ac.toString());

    for(int i=0;i<3;i++){
        ac.send();
        delay(120);
    }
}
void powerful(){
    if (acPowerful==true){
        acPowerful = false;
    }
    else{
    acPowerful = true;
    acComfort = false; }

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}
void off() {

    acPower = false;
    acPowerful = false;

    sendAC();

    server.sendHeader("Location", "/");
    server.send(303);
}
void onAuto() {

    acPower = true;
    acPowerful = false;

    sendAC();

    server.sendHeader("Location", "/");
    server.send(303);
}

void tempUp(){

    if(acTemp < 30)
        acTemp++;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}

void tempDown(){

    if(acTemp > 18)
        acTemp--;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}

void fanUp(){

    if(acFan == kDaikinFanAuto)
        acFan = 2;

    else if(acFan < 5)
        acFan++;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}

void fanDown(){

    if(acFan > 2)
        acFan--;

    else
        acFan = kDaikinFanAuto;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}
void setFan() {
    if (!server.hasArg("value")) {
        server.send(400, "text/plain", "Missing value");
        return;
    }

    int fan = server.arg("value").toInt();

    if (fan < 0 || fan > 5) {
        server.send(400, "text/plain", "Fan must be 0-5");
        return;
    }

    acFan = fan;

    sendAC();

    server.send(200, "text/plain",
                "Fan set to " + fanText());
}
void toggleSwing(){

    acSwing = !acSwing;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}


void toggleComfort(){

    acComfort = !acComfort;

    if(acComfort)
        acPowerful = false;

    sendAC();

    server.sendHeader("Location","/");
    server.send(303);
}
void setTemp() {
    if (!server.hasArg("value")) {
        server.send(400, "text/plain", "Missing value");
        return;
    }

    int temp = server.arg("value").toInt();

    if (temp < 18 || temp > 30) {
        server.send(400, "text/plain", "Temperature must be 18-30 C");
        return;
    }

    acTemp = temp;

    sendAC();

    server.send(200, "text/plain",
                "Temperature set to " + String(acTemp) + " C");
}
void readAHT() {

    if (!ahtAvailable)
        return;

    sensors_event_t humidity, temp;

    aht.getEvent(&humidity, &temp);

    roomTemp = temp.temperature;
    roomHumidity = humidity.relative_humidity;
}
void updateSNMP() {

    snmpRoomTemp = roomTemp * 10;
    snmpHumidity = roomHumidity * 10;

    snmpSetpoint = acTemp;
    if(acFan == kDaikinFanAuto)
    snmpFan = 0;
    else
    snmpFan = acFan;

    snmpPower     = acPower;
    snmpPowerful  = acPowerful;
    snmpComfort   = acComfort;
    snmpSwing     = acSwing;

    snmpRSSI = WiFi.RSSI();

    snmpUptime = millis()/10;      // TimeTicks are 1/100 second
}