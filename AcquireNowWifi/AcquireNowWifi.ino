/* UNO R4 WiFi → Red Pitaya (SCPI)
   acquire_trigger_now: force trigger and read CH1 (ASCII/VOLTS)
*/
#include "wifiSCPI.h"
#include "arduino_secrets.h"

IPAddress RP_IP(192,168,0,17);
const uint16_t RP_PORT = 5000;

WifiSCPI rp;

void printFirst(const String& blk, uint8_t n=12){
  int l=blk.indexOf('{'), r=blk.lastIndexOf('}'); if(l<0||r<=l){ Serial.println(blk); return; }
  String b=blk.substring(l+1,r); int st=0,c=0; Serial.println(F("First samples:"));
  while(c<n){ int k=b.indexOf(',',st); String t=(k==-1)?b.substring(st):b.substring(st,k); t.trim();
    if(t.length()){ Serial.println(t); c++; } if(k==-1) break; st=k+1; }
}

void setup(){
  Serial.begin(115200); delay(200);
  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  rp.scpi("ACQ:RST");
  rp.scpi("ACQ:DEC:Factor 128");
  rp.scpi("ACQ:DATA:FORMAT VOLTS");
  rp.scpi("ACQ:DATA:UNITS ASCII");
  rp.scpi("ACQ:TRig:DLY 0");
  rp.scpi("ACQ:START");
  delay(300);
  rp.scpi("ACQ:TRig NOW");

  while(rp.scpiLine("ACQ:TRig:FILL?")!="1") delay(10);
  printFirst(rp.scpiBlock("ACQ:SOUR1:DATA?"));
  Serial.println("Done.");
}

void loop(){ }

