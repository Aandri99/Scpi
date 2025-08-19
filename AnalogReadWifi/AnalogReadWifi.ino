/*  UNO R4 WiFi → Red Pitaya (SCPI)
    - OUT1 arbitrary waveform = sum of 3 sines
    - Fast, chunked upload (default 2048 points)
    - No acquisition, no LEDs — just ARB gen
*/

#include <WiFiS3.h>
#include <math.h>
#include "arduino_secrets.h"   // SECRET_SSID, SECRET_PASS

/************ User Config ************/
// Red Pitaya SCPI server
IPAddress RP_IP(192, 168, 0, 17);
const uint16_t RP_PORT = 5000;

// ARB buffer size (use 2048 for speed; 16384 is the full period size)
int   ARB_NPTS     = 2048;

// Base output frequency (Hz). The three tones will be
// k1*f0, k2*f0, k3*f0 to make a seamless periodic waveform.
float F0_HZ        = 1000.0;   // "fundamental" frequency

// Amplitudes of the 3 sines (renamed to avoid conflict with A1/A2/A3 pin names)
float AMP1 = 1.0f,  AMP2 = 0.6f,  AMP3 = 0.3f;

// Harmonic bins (integers). Output tones = k*F0_HZ.
int   K1 = 1,       K2 = 2,       K3 = 5;

// Optional phases (radians)
float P1 = 0.0f,    P2 = 0.0f,    P3 = 0.0f;

// Analog output settings
float ARB_AMPL_V   = 1.0f;     // amplitude (Volts)
float ARB_OFFS_V   = 0.0f;     // DC offset (Volts)
/************************************/

WiFiClient client;

/* ---------- SCPI helpers ---------- */
void scpiFlush() { while (client.available()) (void)client.read(); }

void scpiSend(const String& s) {
  if (!client.connected()) return;
  client.print(s);
  client.print("\r\n");   // CRLF is safest
}

/* ---------- WiFi / RP connect ---------- */
bool connectWiFi(unsigned long timeoutMs = 30000) {
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) delay(250);
  if (WiFi.status() != WL_CONNECTED) return false;
  // ensure not 0.0.0.0
  unsigned long t1 = millis();
  while (WiFi.localIP() == IPAddress(0,0,0,0) && millis() - t1 < 3000) delay(50);
  return true;
}

bool connectRP() {
  if (client.connected()) return true;
  client.stop();
  return client.connect(RP_IP, RP_PORT);
}

/* ---------- Waveform: sum of 3 sines, normalized to [-1..+1] ---------- */
float sum3sines_norm(int i, int N) {
  // angle step = 2π * i / N (one period fits exactly in N samples)
  float t = 2.0f * (float)M_PI * (float)i / (float)N;
  float x = 0.0f;
  x += AMP1 * sinf(K1 * t + P1);
  x += AMP2 * sinf(K2 * t + P2);
  x += AMP3 * sinf(K3 * t + P3);

  float S = fabsf(AMP1) + fabsf(AMP2) + fabsf(AMP3);
  if (S < 1e-9f) S = 1.0f;   // avoid divide-by-zero
  x /= S;                    // keep within [-1..+1] conservatively
  if (x > 1.0f) x = 1.0f;    // hard clamp (rare)
  if (x < -1.0f) x = -1.0f;
  return x;
}

/* ---------- Upload ARB (chunked), configure, and run ---------- */
bool uploadSum3SinesOut1(int N) {
  if (!client.connected()) return false;

  Serial.print("Uploading ARB (N="); Serial.print(N); Serial.print(") ... ");

  scpiSend("GEN:RST");
  scpiSend("OUTPUT1:STATE OFF");
  scpiSend("SOUR1:FUNC ARBITRARY");

  // Stream values: SOUR1:TRAC:DATA:DATA v0,v1,...,vN-1
  client.print("SOUR1:TRAC:DATA:DATA ");
  const int chunk = 256;
  for (int i = 0; i < N; ++i) {
    if (i) client.print(",");
    client.print(sum3sines_norm(i, N), 6);
    if ((i % chunk) == 0) Serial.print(".");
  }
  client.print("\r\n");
  Serial.println(" done");

  // Frequency scaling: when N < 16384, the device repeats the pattern in the 16k buffer.
  // Scale so the base tone is exactly F0_HZ.
  float freq_set = F0_HZ * (16384.0f / (float)N);

  client.print("SOUR1:FREQ:FIX ");  client.print(freq_set, 6);     client.print("\r\n");
  client.print("SOUR1:VOLT ");      client.print(ARB_AMPL_V, 6);   client.print("\r\n");
  client.print("SOUR1:VOLT:OFFS "); client.print(ARB_OFFS_V, 6);   client.print("\r\n");
  scpiSend("SOUR1:TRig:SOUR INT");
  scpiSend("OUTPUT1:STATE ON");
  scpiSend("SOUR1:TRig:INT");
  return true;
}

/* ---------- Arduino ---------- */
void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.print("WiFi → ");
  if (!connectWiFi()) { Serial.println("FAIL"); while (true) delay(1000); }
  Serial.print("OK, IP="); Serial.println(WiFi.localIP());

  Serial.print("Connecting to RP "); Serial.print(RP_IP); Serial.print(":"); Serial.println(RP_PORT);
  if (!connectRP()) { Serial.println("TCP connect failed"); while (true) delay(1000); }
  Serial.println("TCP connected");

  // Upload and start the 3-sine sum on OUT1
  uploadSum3SinesOut1(ARB_NPTS);

  Serial.println("Sum-of-3-sines running on OUT1.");
}

void loop() {
  // Keep the link healthy (optional)
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) {
    if (connectRP()) uploadSum3SinesOut1(ARB_NPTS); // re-arm after reconnect
  }
  delay(500);
}
