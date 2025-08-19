#include <WiFiS3.h>
#include "SCPI_RP.h"
#include "SCPI_WiFi.h"
#include "arduino_secrets.h"

// ------- User config -------
#define USE_STATIC_IP  1
IPAddress RP_IP(192, 168, 0, 17);
const uint16_t RP_PORT = 5000;

const float GEN_FREQ_HZ   = 2000.0;
const float GEN_AMPL_V    = 1.0;
const float GEN_OFFSET_V  = 0.0;

#if USE_STATIC_IP
  IPAddress LOCAL_IP(192,168,0,50);
  IPAddress GATEWAY (192,168,0,1);
  IPAddress SUBNET  (255,255,255,0);
  IPAddress DNS     (8,8,8,8);
#endif
// ---------------------------

scpi_rp::SCPI_WiFi net;          // << new
scpi_rp::SCPIRedPitaya rp;
bool rpReady = false;

// (optional) keep these if you like:
void scpiSend(const String& s) { net.send(s); }
String scpiQueryLine(const String& s, unsigned long t=2000) { return net.queryLine(s,t); }
String scpiQueryAsciiBlock(const String& s, unsigned long t=8000) { return net.queryAsciiBlock(s,t); }

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.print("WiFi connecting to "); Serial.println(SECRET_SSID);

  scpi_rp::WiFiConfig cfg;
#if USE_STATIC_IP
  cfg.useStaticIP = true;
  cfg.localIP = LOCAL_IP; cfg.gateway = GATEWAY; cfg.subnet = SUBNET; cfg.dns = DNS;
#endif

  if (!net.connectWiFi(SECRET_SSID, SECRET_PASS, &cfg)) {
    Serial.println("❌ WiFi failed");
    while (true) delay(1000);
  }
  Serial.print("✅ WiFi OK, IP = "); Serial.println(net.localIP());

  if (!net.connectRP(RP_IP, RP_PORT)) {
    Serial.println("❌ TCP connect failed (will retry in loop)");
  } else {
    Serial.println("✅ TCP connected");
    net.bind(rp);         // << bind SCPI to this socket (Stream)
    rpReady = true;
  }

  // ... your LED init, genSineOut1(), and first acquire() as before
}

void loop() {
  // Keep connections alive
  if (!net.isWiFiUp() || !net.isRPUp()) {
    scpi_rp::WiFiConfig cfg;
#if USE_STATIC_IP
    cfg.useStaticIP = true; cfg.localIP = LOCAL_IP; cfg.gateway = GATEWAY; cfg.subnet = SUBNET; cfg.dns = DNS;
#endif
    if (net.ensureConnections(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT, &cfg)) {
      net.bind(rp);                 // re-bind after reconnect
      rpReady = true;
      // optionally re-start generator
      // genSineOut1(GEN_FREQ_HZ, GEN_AMPL_V, GEN_OFFSET_V);
    } else {
      rpReady = false;
    }
  }

  // ... keep your heartbeat + periodic acquisition exactly as before,
  // but use net.send/query* helpers (already wired above).
}
