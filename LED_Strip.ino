#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>

int pixelCount = 150;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(pixelCount, pixelCount);

MDNSResponder mdns;
ESP8266WebServer server(80);
void handleRoot() {
  String out = "<html><head><meta http-equiv='refresh' content='0; url=http://din-egen-hjemmeside.dk/' /></head></html>";
  server.send(200, "text/html", out);
}

void setup(void) {
  WiFiManager wifiManager;
  wifiManager.autoConnect("LED Strip");
  EEPROM.begin(512);

  strip.Begin();
  strip.Show();

  server.on("/", []() {
    handleRoot();
  });

  server.on("/color", handleGenericArgs);

  server.on("/getColor", []() {
    String color = getColor();
    server.send(200, "text/html", color);
  });

  server.begin();

  delay(1000);
  String color = getColor();
  setColorFromHex(color);
}

void handleGenericArgs() {
  if (server.args() > 0) {
    String hexColor = server.arg(0);
    setColorFromHex(hexColor);

    for (int i = 0; i < hexColor.length(); i++) {
      EEPROM.write(30 + i, hexColor[i]);
    }
    EEPROM.commit();

    server.send(200, "text/html", "");
  }
}

void setColorFromHex(String hexColor) {
  int number = (int)strtol(&hexColor[0], NULL, 16);

  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;

  setColor(RgbColor(r, g, b));
}

String getColor() {
  String color;
  for (int i = 0; i < 6; i++)
  {
    color += char(EEPROM.read(30 + i));
  }
  return color;
}

void loop(void) {
  mdns.update();
  server.handleClient();
}

void setColor(RgbColor color) {
  for (int i = 0; i < pixelCount; i++) {
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}
