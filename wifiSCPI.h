#ifndef WIFI_SCPI_H
#define WIFI_SCPI_H

#include <WiFiS3.h>

class WifiSCPI {
public:
  WifiSCPI();
  bool begin(const char* ssid, const char* pass, const IPAddress& ip, uint16_t port = 5000);
  bool connectWiFi(const char* ssid, const char* pass, unsigned long timeout = 30000);
  bool connectRP(const IPAddress& ip, uint16_t port = 5000);
  void scpi(const String& s);
  String scpiLine(const String& s, unsigned long timeout = 3000);
  String scpiBlock(const String& s, unsigned long timeout = 10000);
  void scpiFlush();
  bool connected() { return _client.connected(); }

private:
  WiFiClient _client;
};

#endif
