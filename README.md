# Daikin-ESP8266-AC-Controller
An ESP8266-based WLAN controller for compatible Daikin air conditioners using an IR transmitter
Features

Local Wi-Fi control through an ESP8266

Daikin AC control using IRremoteESP8266

AHT10 temperature/humidity monitoring

Built-in web dashboard

HTTP API for AC control

Direct temperature setpoint API

Direct fan-speed API

SNMP monitoring

Designed for Grafana, InfluxDB, and local SCADA integration

No cloud service required

Hardware

Required

ESP8266 development board

IR LED / IR transmitter

Compatible Daikin air conditioner

USB power supply for the ESP8266

Optional

AHT10 temperature/humidity sensor

SNMP monitoring system

Wiring

The current firmware uses D5 / GPIO14 for the IR transmitter.

ESP8266 D5 / GPIO14
        |
        v
   IR transmitter

The AHT10 uses the ESP8266 I²C interface.

Software

Libraries used:

ESP8266WiFi

ESP8266WebServer

IRremoteESP8266

Adafruit_AHTX0

Wire

SNMP_Agent

The Daikin implementation uses IRDaikinESP.

Configuration

Configure Wi-Fi and network settings before uploading:

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

IPAddress local_IP(192,168,1,250);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
