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

#include "wifiSCPI.h"
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

WifiSCPI rp;

void startBurst() {
  // Reset & basic sine settings
  rp.scpi("GEN:RST");
  rp.scpi("SOUR1:FUNC SINE");
  rp.scpi(String("SOUR1:FREQ:FIX ") + String(FREQ_HZ, 6));
  rp.scpi(String("SOUR1:VOLT ")     + String(AMP_V,   6));
  rp.scpi(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V,  6));

  // --- Burst mode (official commands) ---
  rp.scpi("SOUR1:BURS:STAT BURST");
  rp.scpi(String("SOUR1:BURS:NCYC ") + String(NCYCLES));
  rp.scpi(String("SOUR1:BURS:NOR ")  + String(REPETITIONS));
  rp.scpi(String("SOUR1:BURS:INT:PER ") + String(PERIOD_US));

  // Output + trigger
  rp.scpi("OUTPUT1:STATE ON");
  rp.scpi("SOUR1:TRig:SOUR INT");
  rp.scpi("SOUR1:TRig:INT");
}

void setup() {
  Serial.begin(115200);
  delay(150);

  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  startBurst();
  Serial.println("Burst started on OUT1.");
}

void loop() {
  // Optional keep-alive
  if (WiFi.status() != WL_CONNECTED) rp.connectWiFi(SECRET_SSID, SECRET_PASS);
  if (!rp.connected()) {
    if (rp.connectRP(RP_IP, RP_PORT)) startBurst();
  }
  delay(500);
}

