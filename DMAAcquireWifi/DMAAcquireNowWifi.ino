/* UNO R4 WiFi → Red Pitaya (SCPI)
   dma/acquire_dma_trigger_now: set AXI DMA, trigger NOW, read 1024 samples near trig (ASCII)
*/
#include "wifiSCPI.h"
#include "arduino_secrets.h"

IPAddress RP_IP(192,168,0,17);
const uint16_t RP_PORT = 5000;

WifiSCPI rp;

/* helpers */
void printFirst(const String& blk, uint8_t n=12){
  int l=blk.indexOf('{'), r=blk.lastIndexOf('}'); if(l<0||r<=l){ Serial.println(blk); return; }
  String b=blk.substring(l+1,r); int st=0,c=0; Serial.println(F("First DMA samples:"));
  while(c<n){ int k=b.indexOf(',',st); String t=(k==-1)?b.substring(st):b.substring(st,k); t.trim();
    if(t.length()){ Serial.println(t); c++; } if(k==-1) break; st=k+1; }
}

void setup(){
  Serial.begin(115200); delay(200);
  if(!rp.begin(SECRET_SSID, SECRET_PASS, RP_IP, RP_PORT)){
    Serial.println(F("Failed to connect"));
    while(true){}
  }

  uint32_t axiStart = rp.scpiLine("ACQ:AXI:START?").toInt();
  uint32_t axiSize  = rp.scpiLine("ACQ:AXI:SIZE?").toInt();
  (void)axiStart; (void)axiSize;

  rp.scpi("ACQ:RST");
  rp.scpi("ACQ:AXI:DEC 1");
  rp.scpi("ACQ:AXI:DATA:UNITS VOLTS");
  rp.scpi("ACQ:DATA:FORMAT ASCII");
  rp.scpi(String("ACQ:AXI:SOUR1:SET:Buffer ")+axiStart+","+axiSize);
  rp.scpi("ACQ:AXI:SOUR1:ENable ON");
  rp.scpi("ACQ:AXI:SOUR1:Trig:Dly 0");

  rp.scpi("ACQ:START");
  rp.scpi("ACQ:TRig NOW");

  while(rp.scpiLine("ACQ:TRig:STAT?")!="TD") delay(10);
  while(rp.scpiLine("ACQ:AXI:SOUR1:TRIG:FILL?")!="1") delay(10);

  uint32_t pos = rp.scpiLine("ACQ:AXI:SOUR1:Trig:Pos?").toInt();
  String cmd = String("ACQ:AXI:SOUR1:DATA:Start:N? ")+pos+",1024";
  printFirst(rp.scpiBlock(cmd));

  rp.scpi("ACQ:STOP");
  rp.scpi("ACQ:AXI:SOUR1:ENable OFF");
  Serial.println("Done.");
}

void loop(){ }

