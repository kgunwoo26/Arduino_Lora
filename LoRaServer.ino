#include <SoftwareSerial.h>
#include "SNIPE.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include <ArduinoJson.h>


ESP8266WebServer server(80);
#define HOME_SSID "Geonu"
#define HOME_PWD "11331133"

#define TXpin 11
#define RXpin 10
#define ATSerial Serial
#define MAX_SIZE 100

struct device {
  String id;
  String distance;
  String weight;
};

//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00"; 

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);
String load_recv;

device devices[MAX_SIZE];
int size = 0;
String recv_data[3];

void setup() {


 Serial.begin(115200);
  WiFi.begin(HOME_SSID, HOME_PWD);

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendJson);
  server.begin();
  Serial.println("Server listening");
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());

  ATSerial.begin(115200);
  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);
  DebugSerial.begin(115200);
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }
  if (!SNIPE.lora_setFreq(LORA_CH_1)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }
  if (!SNIPE.lora_setSf(LORA_SF_7)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }
  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }  
  DebugSerial.println("SNIPE LoRa PingPong Test");
}

void split(){
  String temp = load_recv;
  int count = 0;
  int start = 0;
  int end;
  while (1){
    end = temp.indexOf('/');
    if(end == -1){
      recv_data[count++] = temp;
      break;
    }
    recv_data[count++] = temp.substring(start,end);
    temp = temp.substring(end+1);
  }
  
}

String StateToJson() {
  String jsondata ;
  StaticJsonDocument<200> doc;
  Serial.println(size);
  for(int i=0; i<size; i++){
    Serial.println(devices[i].id+" "+devices[i].distance+ " "+devices[i].weight);  
    JsonObject obj = doc.createNestedObject();
    obj["deviceName"] = devices[i].id;
    obj["distance"]= devices[i].distance;
    obj["weight"] = devices[i].weight;
  }
  
  // A["id"] = 'A';
  // A["distance"] = A.distance;
  // A["weight"] = A.weight;

  // B["id"] = 'B';
  // B["distance"] = B.distance;
  // B["weight"] = B.weight;

  serializeJson(doc,jsondata);
  return jsondata;
}

void sendJson() {
  server.send(200,"application/json", StateToJson());
}

void update(){
    int index = -1;
    split();
    for(int i=0; i<size; i++){
      if(devices[i].id == recv_data[0]) index = i;
    }
    if(index == -1){ index = size++;}
    if(!recv_data[0].equals("AT_RX_TIMEOUT"))
      devices[index] ={recv_data[0],recv_data[1],recv_data[2]};
}

void loop() {
  server.handleClient();
  load_recv = SNIPE.lora_recv();
  if(load_recv != NULL) update();
  DebugSerial.println(load_recv);
  delay(300);
}