/* UNO R4 WiFi → Red Pitaya (SCPI)
   Two-channel sine with a fixed phase shift
   - OUT1: sine @ FREQ_HZ, PHASE=0°
   - OUT2: sine @ FREQ_HZ, PHASE=PHASE_DEG
   Uses: SOUR<n>:PHAS, OUTPUT:STATE ON, SOUR:TRig:INT
*/

#include <WiFiS3.h>
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// -------- User config --------
IPAddress RP_IP(192, 168, 0, 17);
const uint16_t RP_PORT = 5000;

const float FREQ_HZ   = 10000.0;  // both channels
const float AMP_V     = 1.0;      // one-way amplitude (|AMP|+|OFFS| ≤ 1 V on most models)
const float OFFS_V    = 0.0;      // DC offset
const float PHASE_DEG = 90.0;     // OUT2 phase relative to OUT1 (allowed −360..+360) :contentReference[oaicite:1]{index=1}
/* ---------------------------- */

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
inline void scpiSend(const String& s){
  if (!client.connected()) return;
  client.print(s); client.print("\r\n"); // CRLF per SCPI
}

void startTwoSineWithPhase() {
  scpiSend("GEN:RST");                         // reset generator (safe defaults) :contentReference[oaicite:2]{index=2}

  // Configure OUT1
  scpiSend("SOUR1:FUNC SINE");                 // waveform               :contentReference[oaicite:3]{index=3}
  scpiSend(String("SOUR1:FREQ:FIX ") + String(FREQ_HZ, 6)); // frequency  :contentReference[oaicite:4]{index=4}
  scpiSend(String("SOUR1:VOLT ")     + String(AMP_V, 6));   // amplitude  :contentReference[oaicite:5]{index=5}
  scpiSend(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V, 6));  // offset     :contentReference[oaicite:6]{index=6}
  scpiSend("SOUR1:PHAS 0");                        // 0° reference       :contentReference[oaicite:7]{index=7}

  // Configure OUT2 (same freq/amp, shifted phase)
  scpiSend("SOUR2:FUNC SINE");
  scpiSend(String("SOUR2:FREQ:FIX ") + String(FREQ_HZ, 6));
  scpiSend(String("SOUR2:VOLT ")     + String(AMP_V, 6));
  scpiSend(String("SOUR2:VOLT:OFFS ")+ String(OFFS_V, 6));
  scpiSend(String("SOUR2:PHAS ")     + String(PHASE_DEG, 6)); // phase in degrees

  // Enable both outputs and start generation in-phase reference
  scpiSend("OUTPUT:STATE ON");                 // arm both channels      :contentReference[oaicite:8]{index=8}
  scpiSend("SOUR:TRig:INT");                   // align & start both     :contentReference[oaicite:9]{index=9}

  // (Alternative to the line above: scpiSend("PHAS:ALIGN"); does the same.) :contentReference[oaicite:10]{index=10}
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

  startTwoSineWithPhase();
  Serial.println("Two-channel sine with phase shift is running.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) startTwoSineWithPhase();
  }
  delay(500);
}
