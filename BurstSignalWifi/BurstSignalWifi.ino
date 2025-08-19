/* UNO R4 WiFi → Red Pitaya (SCPI)
   Generate PWM on OUT1 with 30% duty
   Uses official commands: SOUR1:FUNC PWM, SOUR1:DCYC <ratio>, FREQ/AMP/OFFS
*/

#include <WiFiS3.h>
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ----- User config -----
IPAddress RP_IP(192, 168, 0, 17);  // Red Pitaya IP
const uint16_t RP_PORT = 5000;     // SCPI port

const float PWM_FREQ_HZ = 1000.0;  // PWM frequency (Hz)
const float PWM_DUTY    = 0.30;    // 30% duty (ratio 0..1)
const float AMP_V       = 1.0;     // one-way amplitude (V)
const float OFFS_V      = 0.0;     // DC offset (V)
// -----------------------

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
  client.print(s); client.print("\r\n");  // CRLF per SCPI
}

void startPWM() {
  scpiSend("GEN:RST");
  scpiSend("SOUR1:FUNC PWM");                                 // PWM waveform
  scpiSend(String("SOUR1:FREQ:FIX ") + String(PWM_FREQ_HZ, 6));
  scpiSend(String("SOUR1:VOLT ")     + String(AMP_V, 6));     // amplitude (V)
  scpiSend(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V, 6));    // offset (V)
  scpiSend(String("SOUR1:DCYC ")     + String(PWM_DUTY, 6));  // duty cycle (0..1)
  scpiSend("OUTPUT1:STATE ON");                                // enable OUT1
  scpiSend("SOUR1:TRig:SOUR INT");                             // internal trig
  scpiSend("SOUR1:TRig:INT");                                  // start now
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

  startPWM();
  Serial.println("PWM (30% duty) running on OUT1.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) startPWM();
  }
  delay(500);
}
