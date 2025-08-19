/* UNO R4 WiFi → Red Pitaya: generate a sine on OUT1 (SCPI)
   - WiFiS3 networking (UNO R4 WiFi)
   - SCPI TCP port 5000
   - Sends: GEN:RST; SOUR1:FUNC SINE; FREQ/AMP/OFFS; OUTPUT1 ON; TRIG INT
*/

#include <WiFiS3.h>
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ---------- User config ----------
IPAddress RP_IP(192, 168, 0, 17);  // <-- your Red Pitaya IP
const uint16_t RP_PORT = 5000;     // SCPI server port

// Sine parameters
const float SINE_FREQ_HZ = 2000.0; // frequency
const float SINE_AMPL_V  = 1.0;    // amplitude (volts)
const float SINE_OFFS_V  = 0.0;    // DC offset (volts)
// ---------------------------------

WiFiClient client;

bool connectWiFi(unsigned long timeoutMs = 30000) {
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) delay(250);
  if (WiFi.status() != WL_CONNECTED) return false;
  // Ensure not 0.0.0.0
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
  client.print(s);
  client.print("\r\n"); // CRLF is safest
}

void genSineOut1(float freq_hz, float ampl_v, float offs_v) {
  scpiSend("GEN:RST");
  scpiSend("SOUR1:FUNC SINE");
  scpiSend(String("SOUR1:FREQ:FIX ") + String(freq_hz, 6));
  scpiSend(String("SOUR1:VOLT ")     + String(ampl_v, 6));
  scpiSend(String("SOUR1:VOLT:OFFS ")+ String(offs_v, 6));
  scpiSend("SOUR1:TRig:SOUR INT");
  scpiSend("OUTPUT1:STATE ON");
  scpiSend("SOUR1:TRig:INT");
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

  // Generate sine on OUT1
  genSineOut1(SINE_FREQ_HZ, SINE_AMPL_V, SINE_OFFS_V);
  Serial.println("Sine started on OUT1.");
}

void loop() {
  // Keep connection alive (optional)
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) connectRP();
  delay(500);
}

/* Optional helper to stop output (call once if you want to turn it off)
void stopOutput1() {
  scpiSend("OUTPUT1:STATE OFF");
  // or: scpiSend("GEN:RST");
}
*/
