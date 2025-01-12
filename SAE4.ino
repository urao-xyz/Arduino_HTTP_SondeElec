//
//
//


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

#define ledVie 0

#ifndef STASSID
#define STASSID    "XXX" //Réseau local
#define STAPSK     "XXX"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
int incomingByte = 3 ;
unsigned long t, t1;
LiquidCrystal_I2C lcd(0x3F, 16, 2);

WiFiClient client;  // or WiFiClientSecure for HTTPS
HTTPClient http;

//////////////////////////////// setup ////////////////////////////////////////
void setup(void) {
  //--------------------------- GPIO LCD SERIAL ------------------------------
  pinMode(ledVie, OUTPUT);
  lcd.begin();
  lcd.backlight();
  Serial.begin(115200);
  lcd.setCursor(0, 0);
  lcd.print("Try to connect");
  lcd.setCursor(0, 1);
  lcd.print(STASSID);

  //----------------------- WIFI connexion -----------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  int i = 0;
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(10, 1);

    switch (i % 3) {
      case 0: lcd.print("|"); break;
      case 1: lcd.print("/"); break;
      case 2: lcd.print("-");
    }
    i++;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print("Attente!!  ");
}

//////////////////////////// loop ///////////////////////////////////////
void loop(void) {
  digitalWrite(ledVie, HIGH);
  delay(50);

  MDNS.update();
  digitalWrite(ledVie, LOW);
  delay(50);

  t = millis();
  if ((t - t1) > 500) {
    t1 = t;
    if (Serial.available() > 0) { // si données disponibles sur le port série
      // lit l'octet entrant
      incomingByte = Serial.read();
    }

    // Send request
    http.useHTTP10(true);
    http.begin(client, "http://192.168.0.7/emeter/0");
    http.GET();

    // Parse response
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());

    // Read values
    Serial.println(doc["voltage"].as<long>());
    Serial.println(doc["power"].as<long>());
    lcd.setCursor(12, 0);
    lcd.print("   ");
    lcd.setCursor(13, 0);
    lcd.print(doc["voltage"].as<long>());
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(doc["power"].as<long>());

    //http.useHTTP10(true);
    http.begin(client, "http://192.168.0.7/emeter/1");
    http.GET();
    deserializeJson(doc, http.getStream());
    Serial.println(doc["power"].as<long>());
    lcd.setCursor(5, 1);
    lcd.print(doc["power"].as<long>());

    if (incomingByte == '0') {
      http.useHTTP10(true);
      http.begin(client, "http://192.168.0.7/relay/0?turn=off");
      http.GET();
      deserializeJson(doc, http.getStream());
    }
    
    if (incomingByte == '1') {
      http.useHTTP10(true);
      http.begin(client, "http://192.168.0.7/relay/0?turn=on");
      http.GET();

    }
    http.begin(client, "http://192.168.0.7/relay/0");
    http.GET();
    deserializeJson(doc, http.getStream());
    Serial.println(doc["ison"].as<long>());
    lcd.setCursor(10, 1);
    lcd.print(doc["ison"].as<long>());

    http.end();
  }
}
