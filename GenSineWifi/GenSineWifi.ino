/* UNO R4 WiFi → Red Pitaya (SCPI)
   OUT1 sweep using official sweep commands (no manual stepping)
*/

#include "wifiSCPI.h"
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ------- User config -------
IPAddress RP_IP(192, 168, 0, 17);   // your Red Pitaya IP
const uint16_t RP_PORT = 5000;      // SCPI server port

// Output amplitude/offset
const float AMP_V  = 1.0;           // one-way amplitude (V)
const float OFFS_V = 0.0;           // DC offset (V)

// Sweep config
const float F_START_HZ = 1e3;       // start frequency (Hz)
const float F_STOP_HZ  = 20e3;      // stop frequency (Hz)
const unsigned long SWEEP_TIME_US = 3e6; // sweep time from start→stop, in microseconds
const bool  LOG_SWEEP   = false;    // false=LINEAR, true=LOG
const bool  PINGPONG    = true;     // true=UP_DOWN, false=NORMAL (up only)
const bool  REPEAT_INF  = true;     // infinite repetitions (else one-shot)
// ---------------------------

WifiSCPI rp;

void startSweep() {
  rp.scpi("GEN:RST");
  rp.scpi("SOUR1:FUNC SWEEP");
  rp.scpi(String("SOUR1:VOLT ") + String(AMP_V, 6));
  rp.scpi(String("SOUR1:VOLT:OFFS ") + String(OFFS_V, 6));
  rp.scpi(String("SOUR1:SWeep:FREQ:START ") + String(F_START_HZ, 6));
  rp.scpi(String("SOUR1:SWeep:FREQ:STOP ")  + String(F_STOP_HZ, 6));
  rp.scpi(String("SOUR1:SWeep:TIME ")       + String((long)SWEEP_TIME_US));
  rp.scpi(String("SOUR1:SWeep:MODE ") + (LOG_SWEEP ? "LOG" : "LINEAR"));
  rp.scpi(String("SOUR1:SWeep:DIR ")  + (PINGPONG ? "UP_DOWN" : "NORMAL"));
  if (REPEAT_INF) rp.scpi("SOUR1:SWeep:REP:INF ON");
  else            rp.scpi("SOUR1:SWeep:REP:INF OFF");
  rp.scpi("OUTPUT1:STATE ON");
  rp.scpi("SOUR1:TRig:SOUR INT");
  rp.scpi("SOUR1:TRig:INT");
  rp.scpi("SOUR1:SWeep:STATE ON");
}

void setup() {
  Serial.begin(115200);
  delay(150);

  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  startSweep();
  Serial.println("Sweep running on OUT1.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) rp.connectWiFi(SECRET_SSID, SECRET_PASS);
  if (!rp.connected()) {
    if (rp.connectRP(RP_IP, RP_PORT)) startSweep();
  }
  delay(500);
}

