/* UNO R4 WiFi → Red Pitaya (SCPI)
   Two-channel sine with a fixed phase shift
   - OUT1: sine @ FREQ_HZ, PHASE=0°
   - OUT2: sine @ FREQ_HZ, PHASE=PHASE_DEG
   Uses: SOUR<n>:PHAS, OUTPUT:STATE ON, SOUR:TRig:INT
*/

#include "wifiSCPI.h"
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// -------- User config --------
IPAddress RP_IP(192, 168, 0, 17);
const uint16_t RP_PORT = 5000;

const float FREQ_HZ   = 10000.0;  // both channels
const float AMP_V     = 1.0;      // one-way amplitude
const float OFFS_V    = 0.0;      // DC offset
const float PHASE_DEG = 90.0;     // OUT2 phase relative to OUT1
/* ---------------------------- */

WifiSCPI rp;

void startTwoSineWithPhase() {
  rp.scpi("GEN:RST");

  // Configure OUT1
  rp.scpi("SOUR1:FUNC SINE");
  rp.scpi(String("SOUR1:FREQ:FIX ") + String(FREQ_HZ, 6));
  rp.scpi(String("SOUR1:VOLT ")     + String(AMP_V, 6));
  rp.scpi(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V, 6));
  rp.scpi("SOUR1:PHAS 0");

  // Configure OUT2 (same freq/amp, shifted phase)
  rp.scpi("SOUR2:FUNC SINE");
  rp.scpi(String("SOUR2:FREQ:FIX ") + String(FREQ_HZ, 6));
  rp.scpi(String("SOUR2:VOLT ")     + String(AMP_V, 6));
  rp.scpi(String("SOUR2:VOLT:OFFS ")+ String(OFFS_V, 6));
  rp.scpi(String("SOUR2:PHAS ")     + String(PHASE_DEG, 6));

  rp.scpi("OUTPUT:STATE ON");
  rp.scpi("SOUR:TRig:INT");
}

void setup() {
  Serial.begin(115200);
  delay(150);

  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  startTwoSineWithPhase();
  Serial.println("Two-channel sine with phase shift is running.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) rp.connectWiFi(SECRET_SSID, SECRET_PASS);
  if (!rp.connected()) {
    if (rp.connectRP(RP_IP, RP_PORT)) startTwoSineWithPhase();
  }
  delay(500);
}

