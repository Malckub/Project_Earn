#include "DHT.h"
#include <WiFi.h>
#include <Arduino.h>
#include <Wire.h>
#include "PubSubClient.h"
//wifi
#define ssid "HOME" //ชื่อ wifi
#define password "0819444161" //รหัส
//DHT
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//flame
#define flamePin 27
//BUZZER
#define buzzerPin 18
String buzzer = "off";
//relay
#define relayPin 19
String relays = "off";
//NETPIE
const char* mqtt_server = "broker.netpie.io";
const char* client_id = "8d916673-6675-4056-8044-eb5de72f6ee0";//ต้องแก้ไข
const char* token     = "ryzcozTKgqQrgaxn2jGRH4HWdoacGUnZ";    //ต้องแก้ไข
const char* secret    = "72gKmVdUPNrSSeax9nnez4wxxzd3eecq";   //ต้องแก้ไข
WiFiClient espClient;
PubSubClient client(espClient);
String war = "off";
String check = "on";
char msg[100];

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}
// ฟังก์ชั่น ตรวจสอบการเชื่อมต่อ และเชื่อมต่อ NETPIE2020
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect(client_id, token, secret)) {
      Serial.println(F("connected"));
      client.subscribe("@msg/#");
    // ... and resubscribe from server
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String mess;
  String tpc;
  for (int i = 0; i < length; i++) {
    mess = mess+(char)payload[i];
  }
  Serial.println(mess);
  getMsg(topic,mess);
}

void setup() {
  Serial.begin(9600);
  pinMode(buzzerPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  // เชื่อมต่อ WiFi
  setup_wifi();
  // ตั่งค่า Broker Netpie และพอร์ต 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int flame = digitalRead(flamePin);
  int relay = digitalRead(relayPin);
  //warning
  if(war == "on"){
    if(flame == 1){
      digitalWrite(relayPin,1);
      while(check == "on"){
        digitalWrite(buzzerPin,1);
        delay(500);
        digitalWrite(buzzerPin,0);
        delay(500);
        client.loop();
      }
      check = "on";
      digitalWrite(relayPin,0);
    }
  }
  // Wait a few seconds between measurements.
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String data = "{\"data\":{\"Humidity\":"+String(h)+",\"Temp\":"+String(t)+",\"relay\":"+String(relay)+"}}";
  Serial.println(data);
  data.toCharArray(msg,(data.length()+1));
  client.publish("@shadow/data/update",msg);
  client.loop();
  delay(1000);
}
String getMsg(String topic_,String mess_){
  if(topic_ == "@msg/war"){
    if(mess_ == "on"){
      war = "on";
    }
    else{
      digitalWrite(buzzerPin,0);
      digitalWrite(relayPin,0);
      war = "off";
    }
    return war;
  }
  else if(topic_ == "@msg/relay"){
    if(mess_ == "on"){
      digitalWrite(relayPin,1);
      relays = "on";
    }
    else{
      digitalWrite(relayPin,0);
      relays = "off";
    }
    return "";
  }
  else if(topic_ == "@msg/buzzer"){
    if(mess_ == "on"){
      buzzer = "on";
      while(buzzer == "on"){
        digitalWrite(buzzerPin,1);
        delay(500);
        digitalWrite(buzzerPin,0);
        delay(500);
        client.loop();
      }
    }
    else{
      digitalWrite(relayPin,0);
      buzzer = "off";
    }
    return buzzer;
  }
  else if(topic_ == "@msg/check"){
    check = mess_;
    return check;
  }
}