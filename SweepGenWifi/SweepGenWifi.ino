/* UNO R4 WiFi → Red Pitaya (SCPI)
   Sine burst on OUT1 using official burst commands

   Flow:
     GEN:RST
     SOUR1:FUNC SINE
     SOUR1:FREQ:FIX <Hz>
     SOUR1:VOLT <V>; SOUR1:VOLT:OFFS <V>
     SOUR1:BURS:STAT BURST
     SOUR1:BURS:NCYC <Ncycles>
     SOUR1:BURS:NOR  <Repetitions>   (65536 == infinite)
     SOUR1:BURS:INT:PER <Period_us>
     OUTPUT1:STATE ON
     SOUR1:TRig:SOUR INT
     SOUR1:TRig:INT
*/

#include <WiFiS3.h>
#include "arduino_secrets.h"  // SECRET_SSID, SECRET_PASS

// ---------- User config ----------
IPAddress RP_IP(192, 168, 0, 17);   // your Red Pitaya IP
const uint16_t RP_PORT = 5000;      // SCPI port

// Signal settings
const float FREQ_HZ   = 10e3;       // sine frequency
const float AMP_V     = 0.8;        // one-way amplitude (V). |AMP|+|OFFS| ≤ 1 V on most RP models
const float OFFS_V    = 0.0;        // DC offset (V)

// Burst settings
const uint32_t NCYCLES      = 5;        // number of sine periods in one burst
const uint32_t REPETITIONS  = 10;       // number of bursts (65536 == infinite)
const uint32_t PERIOD_US    = 100000;   // time from start-of-burst to start-of-next (µs)
// ----------------------------------

WiFiClient client;

bool connectWiFi(unsigned long timeoutMs = 30000) {
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeoutMs) delay(250);
  if (WiFi.status() != WL_CONNECTED) return false;
  unsigned long t1 = millis();
  while (WiFi.localIP() == IPAddress(0,0,0,0) && millis() - t1 < 3000) delay(50);
  return true;
}
bool connectRP() {
  if (client.connected()) return true;
  client.stop();
  return client.connect(RP_IP, RP_PORT);
}
inline void scpiSend(const String& s) {
  if (!client.connected()) return;
  client.print(s); client.print("\r\n");   // CRLF per SCPI
}

void startBurst() {
  // Reset & basic sine settings
  scpiSend("GEN:RST");
  scpiSend("SOUR1:FUNC SINE");
  scpiSend(String("SOUR1:FREQ:FIX ") + String(FREQ_HZ, 6));
  scpiSend(String("SOUR1:VOLT ")     + String(AMP_V,   6));
  scpiSend(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V,  6));

  // --- Burst mode (official commands) ---
  scpiSend("SOUR1:BURS:STAT BURST");                     // enable burst mode
  scpiSend(String("SOUR1:BURS:NCYC ") + String(NCYCLES));        // cycles per burst
  scpiSend(String("SOUR1:BURS:NOR ")  + String(REPETITIONS));    // number of bursts (65536 == INF)
  scpiSend(String("SOUR1:BURS:INT:PER ") + String(PERIOD_US));   // period between burst starts, in µs

  // Output + trigger
  scpiSend("OUTPUT1:STATE ON");        // enable output driver
  scpiSend("SOUR1:TRig:SOUR INT");     // internal trigger
  scpiSend("SOUR1:TRig:INT");          // start now
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.print("WiFi → ");
  if (!connectWiFi()) { Serial.println("FAIL"); while (true) delay(1000); }
  Serial.print("OK, IP="); Serial.println(WiFi.localIP());

  Serial.print("Connecting to RP "); Serial.print(RP_IP); Serial.print(":"); Serial.println(RP_PORT);
  if (!connectRP()) { Serial.println("TCP connect failed"); while (true) delay(1000); }
  Serial.println("TCP connected");

  startBurst();
  Serial.println("Burst started on OUT1.");
}

void loop() {
  // Optional keep-alive
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) startBurst();
  }
  delay(500);
}
