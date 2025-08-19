/* UNO R4 WiFi → Red Pitaya (SCPI)
   acquire_trigger_from_generator:
   - start a sine on OUT1
   - arm acquisition on AWG trigger (AWG_PE)
   - fire generator INT trigger to produce the edge
*/
#include <WiFiS3.h>
#include "arduino_secrets.h"

IPAddress RP_IP(192,168,0,17);
const uint16_t RP_PORT = 5000;

WiFiClient client;

/* helpers */
void scpiFlush(){ while(client.available()) (void)client.read(); }
inline void scpi(const String& s){ client.print(s); client.print("\r\n"); }
String scpiLine(const String& s, unsigned long to=3000){
  scpiFlush(); scpi(s); String r; unsigned long t0=millis();
  while(millis()-t0<to) while(client.available()){
    char c=client.read(); if(c=='\n'){ if(r.endsWith("\r")) r.remove(r.length()-1); return r; } r+=c; }
  return r;
}
String scpiBlock(const String& s, unsigned long to=8000){
  scpiFlush(); scpi(s); String r; bool in=false; unsigned long t0=millis();
  while(millis()-t0<to) while(client.available()){
    char c=client.read(); if(!in){ if(c=='{'){ in=true; r+=c; } continue; } r+=c; if(c=='}') return r; }
  return r;
}
void printFirst(const String& blk, uint8_t n=12){
  int l=blk.indexOf('{'), r=blk.lastIndexOf('}'); if(l<0||r<=l){ Serial.println(blk); return; }
  String b=blk.substring(l+1,r); int st=0,c=0; Serial.println(F("First samples:"));
  while(c<n){ int k=b.indexOf(',',st); String t=(k==-1)?b.substring(st):b.substring(st,k); t.trim();
    if(t.length()){ Serial.println(t); c++; } if(k==-1) break; st=k+1; }
}
bool connectWiFi(unsigned long to=30000){
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0=millis(); while(WiFi.status()!=WL_CONNECTED && millis()-t0<to) delay(250);
  if(WiFi.status()!=WL_CONNECTED) return false;
  unsigned long t1=millis(); while(WiFi.localIP()==IPAddress(0,0,0,0)&&millis()-t1<3000) delay(50);
  return true;
}
bool connectRP(){ if(client.connected()) return true; client.stop(); return client.connect(RP_IP, RP_PORT); }

void setup(){
  Serial.begin(115200); delay(200);
  connectWiFi(); connectRP();

  // Generator: small sine on OUT1
  scpi("GEN:RST");
  scpi("SOUR1:FUNC SINE");
  scpi("SOUR1:FREQ:FIX 10000");
  scpi("SOUR1:VOLT 0.5");
  scpi("SOUR1:VOLT:OFFS 0");
  scpi("OUTPUT1:STATE ON");

  // Acquisition waiting for AWG trigger
  scpi("ACQ:RST");
  scpi("ACQ:DEC:Factor 1");
  scpi("ACQ:DATA:FORMAT VOLTS");
  scpi("ACQ:DATA:UNITS ASCII");
  scpi("ACQ:TRig:DLY 0");
  scpi("ACQ:START");
  scpi("ACQ:TRig AWG_PE");

  // Fire AWG INT trigger
  scpi("SOUR1:TRig:SOUR INT");
  scpi("SOUR1:TRig:INT");

  while(scpiLine("ACQ:TRig:FILL?")!="1") delay(10);
  printFirst(scpiBlock("ACQ:SOUR1:DATA?"));
  Serial.println("Done.");
}
void loop(){ }
