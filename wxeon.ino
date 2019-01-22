#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "SSD1306Wire.h"

const char* wifiSSID = "KANJIKLUB";
const char* wifiPassword = "HANS010!";

const int wxPort = 443;
const char* wxHostName = "api.weather.gov";
const char wxHostFingerprint[] PROGMEM = "72 4C 3E FE 14 F0 30 2F BA 39 3E 1F AD 21 C1 9D 5D A0 70 3E";
const char* wxRequestCurrent = "/stations/KAVL/observations/current";
String wxCurrent = "";
const int wxLoopOffsetMax = 1000;

String httpRequest = "";
String httpRequestLine = "";

WiFiServer server(80);
WiFiClient httpClient;
WiFiClientSecure httpsClient;

void setup() {
  serialInit();
  builtinLEDInit();
  wifiConnect();
  setWxCurrent();
}

void loop() {

  httpClient = server.available();
  int wxLoopOffset = 0;

  builtinBlink();

  if (httpClient.connected()) {

    httpRequestLine = "";
    char inputCharacter;

    Serial.println("Client Connected");
    
    while (httpClient.connected() && httpClient.available()) {

      inputCharacter = httpClient.read();
      httpRequest += inputCharacter;      

      if (inputCharacter == '\n') {
          
        if (httpRequestLine.length() == 0) {

          digitalWrite(LED_BUILTIN, HIGH);
          delay(100); 
          httpConnect(httpClient);
          htmlHead(httpClient);
          htmlBody(httpClient, "I'm too weak to put anything good.");
          htmlBody(httpClient, wxCurrent);
          htmlTail(httpClient);
          digitalWrite(LED_BUILTIN, LOW);
          break;

        } else {
          
          httpRequestLine = "";

        }

        builtinBlink(18);

      } else if (inputCharacter != '\r') {

        httpRequestLine += inputCharacter;

      } else {
        ;
      }
    }   
  }

  Serial.print(httpRequest);
  httpRequest = "";
  httpClient.stop(); 

  if (wxLoopOffset <= wxLoopOffsetMax) {
    wxLoopOffset +=1;
  } else {
    wxLoopOffset = 0;
    setWxCurrent();
  }
    
}

void setWxCurrent() {

  int maxTries = 5;

  httpsClient.setFingerprint(wxHostFingerprint);

  for (int i = 1; i <= maxTries; i++) {

    if (httpsClient.connect(wxHostName, wxPort) == false) {

      Serial.println(String("setWxCurrent(): Failed to connect httpsClient") + httpsClient.status());
            
      if (i == maxTries) {
        Serial.println("setWxCurrent(): Maximum number of tries to connections failed.");
        Serial.println("setWxCurrent(): Aborting.");
        return;
      }

      builtinBlink(1000);
      
    } else {

      httpsClient.print(String("GET ") + wxRequestCurrent + " HTTP/1.1\r\n" +
        "Host: " + wxHostName + "\r\n" +
        "Content-Type: text/html; charset=utf-8\r\n" +
        "Access-Control-Max-Age: 10\r\n" + 
        "User-Agent: WXeon ESP8266\r\n" +
        "Connection: close\r\n\r\n");
        
      builtinBlink();
      
      if (httpsClient.connected()) {
        wxCurrent = httpsClient.readString();
      }
    }
  }



  if (wxCurrent == "") {
    Serial.println(String("FAILED:") + wxHostName + ":" + wxRequestCurrent);
  } else {
    Serial.println(wxCurrent);
  }
}

void htmlBody(WiFiClient c, String content) {
  c.println(content);
}

void htmlBody(WiFiClient c, int content) {
  c.println(content);
}

void htmlHead(WiFiClient c) {
  c.println("<!DOCTYPE html><html><head>");
  c.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  c.println("<link rel='icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAS0lEQVR42s2SMQ4AIAjE+P+ncSYdasgNXMJgcyIIlVKPIKdvioAXyWBeJmVpqRZKWtj9QWAKZyWll50b8IcL9JUeQF50n28ckyb0ADG8RLwp05YBAAAAAElFTkSuQmCC' type='image/x-png' />");
  c.println("</head><body>");
  c.println("<h1>WXeon</h1>");
}

void htmlTail(WiFiClient c) {
  c.println("</body></html>");
  c.println();
}

void httpConnect(WiFiClient c) {
  builtinBlink();
  c.println("HTTP/1.1 200 OK");
  c.println("Content-type:text/html");
  c.println("Connection: close");
  c.println();
}

void wifiConnect() {

  Serial.print(wifiSSID);
  Serial.print(": ");

  WiFi.begin(wifiSSID, wifiPassword);

  builtinBlink();

  while (WiFi.status() != WL_CONNECTED)
  {
    builtinBlink(500);
  }

  server.begin();
  delay(9900);

  Serial.println(WiFi.localIP());
}

void builtinBlink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100); 
  digitalWrite(LED_BUILTIN, LOW);
}

void builtinBlink(int millisecond) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(millisecond); 
  digitalWrite(LED_BUILTIN, LOW);
}

void builtinLEDInit() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void serialInit() {
    Serial.begin(115200);
    Serial.println("\n\n\n\n\n\n\n\n\n\n");
} 