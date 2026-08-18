# Daikin ESP8266 AC Controller

ESP8266-based local Wi-Fi controllers for Daikin air conditioners.

I originally built this for a Daikin FTK inverter-series AC using
IRremoteESP8266. Later, I made a separate controller for a Daikin FTV
unit where some functions had to be implemented using captured raw IR
frames from the original remote.

## Firmware

### FTK Inverter Series

`espserverdaikinAC.ino`

The original controller for the Daikin FTK inverter series.

- Wi-Fi control
- Daikin IR control using IRremoteESP8266
- Temperature and fan control
- AHT10 temperature/humidity monitoring
- Web interface
- HTTP API
- SNMP monitoring

### FTV Series

`espserverdaikinFTKVToggleOnly.ino`
`espserverdaikinFTKV_TimerONOFF.ino`

A separate controller for the Daikin FTV series.

This version uses captured raw IR frames from the original FTV remote
for functions that aren't handled by the normal Daikin implementation.

It currently includes the captured timer-related frames used to control
the AC's ON/OFF timer.

The timer frames are tied to the captured remote state and are not yet
dynamically generated.

## Hardware

- ESP8266
- IR transmitter
- Compatible Daikin air conditioner
- AHT10 sensor (optional)

## Local monitoring

The controller can expose sensor and status values through SNMP, making
it possible to feed the AC into a local Grafana / InfluxDB setup.

There is no cloud service involved.
