/* UNO R4 WiFi → Red Pitaya: generate a sine on OUT1 (SCPI)
   Sends: GEN:RST; SOUR1:FUNC SINE; FREQ/AMP/OFFS; OUTPUT1 ON; TRIG INT
*/

#include "wifiSCPI.h"
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ---------- User config ----------
IPAddress RP_IP(192, 168, 0, 17);  // <-- your Red Pitaya IP
const uint16_t RP_PORT = 5000;     // SCPI server port

// Sine parameters
const float SINE_FREQ_HZ = 2000.0; // frequency
const float SINE_AMPL_V  = 1.0;    // amplitude (volts)
const float SINE_OFFS_V  = 0.0;    // DC offset (volts)
// ---------------------------------

WifiSCPI rp;

void genSineOut1(float freq_hz, float ampl_v, float offs_v) {
  rp.scpi("GEN:RST");
  rp.scpi("SOUR1:FUNC SINE");
  rp.scpi(String("SOUR1:FREQ:FIX ") + String(freq_hz, 6));
  rp.scpi(String("SOUR1:VOLT ")     + String(ampl_v, 6));
  rp.scpi(String("SOUR1:VOLT:OFFS ")+ String(offs_v, 6));
  rp.scpi("SOUR1:TRig:SOUR INT");
  rp.scpi("OUTPUT1:STATE ON");
  rp.scpi("SOUR1:TRig:INT");
}

void setup() {
  Serial.begin(115200);
  delay(150);

  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  // Generate sine on OUT1
  genSineOut1(SINE_FREQ_HZ, SINE_AMPL_V, SINE_OFFS_V);
  Serial.println("Sine started on OUT1.");
}

void loop() {
  // Keep connection alive (optional)
  if (WiFi.status() != WL_CONNECTED) rp.connectWiFi(SECRET_SSID, SECRET_PASS);
  if (!rp.connected()) rp.connectRP(RP_IP, RP_PORT);
  delay(500);
}

// Optional helper to stop output (call once if you want to turn it off)
void stopOutput1() {
  rp.scpi("OUTPUT1:STATE OFF");
  // or: rp.scpi("GEN:RST");
}

