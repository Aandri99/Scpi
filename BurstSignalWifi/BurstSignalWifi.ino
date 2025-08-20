/* UNO R4 WiFi → Red Pitaya (SCPI)
   Generate PWM on OUT1 with 30% duty
   Uses official commands: SOUR1:FUNC PWM, SOUR1:DCYC <ratio>, FREQ/AMP/OFFS
*/

#include "wifiSCPI.h"
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

// ----- User config -----
IPAddress RP_IP(192, 168, 0, 17);  // Red Pitaya IP
const uint16_t RP_PORT = 5000;     // SCPI port

const float PWM_FREQ_HZ = 1000.0;  // PWM frequency (Hz)
const float PWM_DUTY    = 0.30;    // 30% duty (ratio 0..1)
const float AMP_V       = 1.0;     // one-way amplitude (V)
const float OFFS_V      = 0.0;     // DC offset (V)
// -----------------------

WifiSCPI rp;

void startPWM() {
  rp.scpi("GEN:RST");
  rp.scpi("SOUR1:FUNC PWM");
  rp.scpi(String("SOUR1:FREQ:FIX ") + String(PWM_FREQ_HZ, 6));
  rp.scpi(String("SOUR1:VOLT ")     + String(AMP_V, 6));
  rp.scpi(String("SOUR1:VOLT:OFFS ")+ String(OFFS_V, 6));
  rp.scpi(String("SOUR1:DCYC ")     + String(PWM_DUTY, 6));
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

  startPWM();
  Serial.println("PWM (30% duty) running on OUT1.");
}

void loop() {
  // Optional keep-alive / auto-restart
  if (WiFi.status() != WL_CONNECTED) rp.connectWiFi(SECRET_SSID, SECRET_PASS);
  if (!rp.connected()) {
    if (rp.connectRP(RP_IP, RP_PORT)) startPWM();
  }
  delay(500);
}

