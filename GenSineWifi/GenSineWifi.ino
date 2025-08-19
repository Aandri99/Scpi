/* UNO R4 WiFi → Red Pitaya (SCPI)
   OUT1 sweep using official sweep commands (no manual stepping)
   Docs: SOUR1:FUNC SWEEP; SOUR1:SWEep:FREQ:START/STOP; ...:TIME; ...:MODE; ...:DIR; ...:STATE
*/

#include <WiFiS3.h>
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ------- User config -------
IPAddress RP_IP(192, 168, 0, 17);   // your Red Pitaya IP
const uint16_t RP_PORT = 5000;      // SCPI server port

// Output amplitude/offset
const float AMP_V  = 1.0;           // one-way amplitude (V), keep |AMP_V|+|OFFS_V| ≤ 1 V
const float OFFS_V = 0.0;           // DC offset (V)

// Sweep config
const float F_START_HZ = 1e3;       // start frequency (Hz)
const float F_STOP_HZ  = 20e3;      // stop frequency (Hz)
const unsigned long SWEEP_TIME_US = 3e6; // sweep time from start→stop, in microseconds
const bool  LOG_SWEEP   = false;    // false=LINEAR, true=LOG
const bool  PINGPONG    = true;     // true=UP_DOWN, false=NORMAL (up only)
const bool  REPEAT_INF  = true;     // infinite repetitions (else one-shot)
// ---------------------------

WiFiClient client;

bool connectWiFi(unsigned long timeoutMs = 30000) {
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) delay(250);
  if (WiFi.status() != WL_CONNECTED) return false;
  unsigned long t1 = millis();
  while (WiFi.localIP() == IPAddress(0,0,0,0) && (millis() - t1) < 3000) delay(50);
  return true;
}
bool connectRP() {
  if (client.connected()) return true;
  client.stop();
  return client.connect(RP_IP, RP_PORT);
}
inline void scpiSend(const String& s) {
  if (!client.connected()) return;
  client.print(s); client.print("\r\n");  // CRLF per SCPI
}

void startSweep() {
  // Reset generator and set SWEEP waveform
  scpiSend("GEN:RST");                                 // stop & defaults
  scpiSend("SOUR1:FUNC SWEEP");                        // enable sweep waveform  (docs)
  scpiSend(String("SOUR1:VOLT ") + String(AMP_V, 6));  // amplitude (V)
  scpiSend(String("SOUR1:VOLT:OFFS ") + String(OFFS_V, 6)); // offset (V)

  // Configure sweep
  scpiSend(String("SOUR1:SWeep:FREQ:START ") + String(F_START_HZ, 6)); // Hz
  scpiSend(String("SOUR1:SWeep:FREQ:STOP ")  + String(F_STOP_HZ, 6));  // Hz
  scpiSend(String("SOUR1:SWeep:TIME ")       + String((long)SWEEP_TIME_US)); // µs
  scpiSend(String("SOUR1:SWeep:MODE ") + (LOG_SWEEP ? "LOG" : "LINEAR"));     // LINEAR/LOG
  scpiSend(String("SOUR1:SWeep:DIR ")  + (PINGPONG ? "UP_DOWN" : "NORMAL"));  // direction

  // Repetition: infinite or one-shot
  if (REPEAT_INF) scpiSend("SOUR1:SWeep:REP:INF ON");  // infinite repeats
  else            scpiSend("SOUR1:SWeep:REP:INF OFF"); // one pass

  // Enable output & run
  scpiSend("OUTPUT1:STATE ON");        // enable output driver
  scpiSend("SOUR1:TRig:SOUR INT");     // internal trigger source
  scpiSend("SOUR1:TRig:INT");          // start from the beginning
  scpiSend("SOUR1:SWeep:STATE ON");    // start sweep engine
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

  startSweep();
  Serial.println("Sweep running on OUT1.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) startSweep();
  }
  delay(500);
}
