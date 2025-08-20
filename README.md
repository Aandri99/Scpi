# Scpi

## WifiSCPI helper

`wifiSCPI.h` / `wifiSCPI.cpp` provide a small class that joins a Wi-Fi network, opens a TCP connection to the Red Pitaya SCPI server and exposes helpers for sending commands.

### Using the helper in your sketch

1. Copy `wifiSCPI.h` and `wifiSCPI.cpp` into your sketch folder (or install them as a library).
2. Create an `arduino_secrets.h` with your network credentials:

   ```cpp
   #define SECRET_SSID "your-ssid"
   #define SECRET_PASS "your-password"
   ```
3. Include the headers and call `begin` with the Red Pitaya IP address and SCPI port:

   ```cpp
   #include "wifiSCPI.h"
   #include "arduino_secrets.h"

   IPAddress rpIp(192, 168, 0, 17);   // update with your RP address
   const uint16_t rpPort = 5000;      // SCPI port
   WifiSCPI rp;

   void setup() {
     Serial.begin(115200);
     rp.begin(SECRET_SSID, SECRET_PASS, rpIp, rpPort);

     rp.scpi("ACQ:RST");
     Serial.println(rp.scpiLine("*IDN?"));
   }

   void loop() {}
   ```

`begin` retries until both Wi-Fi and SCPI connections succeed. Use `scpi` to send a command without reading a response, `scpiLine` for single line replies, and `scpiBlock` for block data between `{}` braces. The underlying `WiFiClient` can be accessed via `client()` for streaming operations.

Each example folder demonstrates a different Red Pitaya feature while leveraging this helper for network setup and SCPI communication.
