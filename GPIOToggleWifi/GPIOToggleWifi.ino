#include <WiFiS3.h>
#include "SCPI_RP.h"
#include "arduino_secrets.h"  // SECRET_SSID, SECRET_PASS

// ---------- User config ----------
#define USE_STATIC_IP  1
IPAddress RP_IP(192, 168, 0, 17);    // Red Pitaya IP
const uint16_t RP_PORT = 5000;       // SCPI TCP port

#if USE_STATIC_IP
IPAddress LOCAL_IP(192,168,0,50);
IPAddress GATEWAY (192,168,0,1);
IPAddress SUBNET  (255,255,255,0);
IPAddress DNS     (8,8,8,8);
#endif
// ---------------------------------

WiFiClient client;
scpi_rp::SCPIRedPitaya rp;  // kept to match your pattern
bool rpReady = false;

// --- connection (same style as yours) ---
bool connectWiFi(unsigned long timeoutMs = 30000) {
#if USE_STATIC_IP
  WiFi.config(LOCAL_IP, GATEWAY, SUBNET, DNS);
#endif
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) delay(250);
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long t1 = millis();
    while (WiFi.localIP() == IPAddress(0,0,0,0) && millis() - t1 < 3000) delay(50);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool connectRP() {
  if (client.connected()) return true;
  client.stop();
  if (!client.connect(RP_IP, RP_PORT)) return false;
  rp.initSocket(&client);        // bind SCPI to this TCP socket (pattern preserved)
  rpReady = true;
  return true;
}

// --- tiny SCPI helpers on your client socket ---
void scpiSend(const String& s) { if (client.connected()) { client.print(s); client.print("\r\n"); } }
String scpiQueryLine(const String& s, unsigned long to=2000) {
  if (!client.connected()) return "";
  while (client.available()) (void)client.read();       // flush
  client.print(s); client.print("\r\n");
  String r; unsigned long t0=millis();
  while (millis()-t0<to) {
    while (client.available()) {
      char c=client.read();
      if (c=='\n') { if (r.endsWith("\r")) r.remove(r.length()-1); return r; }
      r+=c;
    }
  }
  return r;
}

// --- GPIO pins ---
const char* OUT_PIN = "DIO0_P";
const char* IN_PIN  = "DIO1_P";

void setup() {
  Serial.begin(115200);
  delay(150);

  connectWiFi();
  connectRP();

  if (client.connected()) {
    scpiSend(String("DIG:PIN:DIR OUT,") + OUT_PIN);
    scpiSend(String("DIG:PIN:DIR IN,")  + IN_PIN);

    Serial.print(OUT_PIN); Serial.print(" dir="); Serial.println(scpiQueryLine(String("DIG:PIN:DIR? ")+OUT_PIN));
    Serial.print(IN_PIN);  Serial.print(" dir="); Serial.println(scpiQueryLine(String("DIG:PIN:DIR? ")+IN_PIN));
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) {
      scpiSend(String("DIG:PIN:DIR OUT,") + OUT_PIN);
      scpiSend(String("DIG:PIN:DIR IN,")  + IN_PIN);
    }
  }
  if (!client.connected()) { delay(200); return; }

  static bool level = false;
  scpiSend(String("DIG:PIN ") + OUT_PIN + "," + (level ? "1" : "0"));
  level = !level;

  String v = scpiQueryLine(String("DIG:PIN? ") + IN_PIN);
  Serial.print(IN_PIN); Serial.print(" = "); Serial.println(v);

  delay(200);
}
