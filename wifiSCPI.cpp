#include "wifiSCPI.h"

WifiSCPI::WifiSCPI() : _client() {}

bool WifiSCPI::begin(const char* ssid, const char* pass, const IPAddress& ip, uint16_t port){
  while(!connectWiFi(ssid, pass)) {
    Serial.println(F("WiFi not ready, retrying..."));
    delay(1000);
  }
  while(!connectRP(ip, port)) {
    Serial.println(F("SCPI connection failed, retrying..."));
    delay(1000);
  }
  return true;
}

bool WifiSCPI::connectWiFi(const char* ssid, const char* pass, unsigned long timeout){
  WiFi.begin(ssid, pass);
  unsigned long t0 = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - t0 < timeout) delay(250);
  if(WiFi.status() != WL_CONNECTED) return false;
  unsigned long t1 = millis();
  while(WiFi.localIP() == IPAddress(0,0,0,0) && millis() - t1 < 3000) delay(50);
  return true;
}

bool WifiSCPI::connectRP(const IPAddress& ip, uint16_t port){
  if(_client.connected()) return true;
  _client.stop();
  return _client.connect(ip, port);
}

void WifiSCPI::scpiFlush(){
  while(_client.available()) (void)_client.read();
}

void WifiSCPI::scpi(const String& s){
  _client.print(s);
  _client.print("\r\n");
}

String WifiSCPI::scpiLine(const String& s, unsigned long timeout){
  scpiFlush();
  scpi(s);
  String r;
  unsigned long t0 = millis();
  while(millis() - t0 < timeout) {
    while(_client.available()) {
      char c = _client.read();
      if(c == '\n') {
        if(r.endsWith("\r")) r.remove(r.length()-1);
        return r;
      }
      r += c;
    }
  }
  return r;
}

String WifiSCPI::scpiBlock(const String& s, unsigned long timeout){
  scpiFlush();
  scpi(s);
  String r;
  bool in = false;
  unsigned long t0 = millis();
  while(millis() - t0 < timeout) {
    while(_client.available()) {
      char c = _client.read();
      if(!in) {
        if(c == '{') {
          in = true;
          r += c;
        }
        continue;
      }
      r += c;
      if(c == '}') return r;
    }
  }
  return r;
}

