
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "FastLED.h"


/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) ******************/
const char* ssid = "YOURSSID"; //type your WIFI information inside the quotes
const char* password = "YOURSSIDPASSWORD";
const char* mqtt_server = "192.168.0.100"; //Local MQTT Server Address
const char* mqtt_username = "yourMQTTusername";
const char* mqtt_password = "yourMQTTpassword";
const int mqtt_port = 1883;



/**************************** FOR OTA **************************************************/
#define SENSORNAME "StrangerThings" //change this to whatever you want to call your device

/************* MQTT TOPICS (change these topics as you wish)  **************************/
const char* light_state_topic = "lights/strangerthings";
const char* light_set_topic = "lights/strangerthings/set";

const char* wordChar = "I Am Here";
String wordString = "I Am Here";

int current_position = 0;
int wait_time = 0;
int animation_speed = 50;

/****************************************FOR JSON***************************************/
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
#define MQTT_MAX_PACKET_SIZE 512
const char* on_cmd = "ON";
const char* off_cmd = "OFF";


/*********************************** FastLED Defintions ********************************/
#define NUM_LEDS    186
#define DATA_PIN    5
//#define CLOCK_PIN 5
#define CHIPSET     WS2811
#define COLOR_ORDER RBG

WiFiClient espClient;
PubSubClient client(espClient);
struct CRGB leds[NUM_LEDS];
struct CRGB leds_rgb[NUM_LEDS];
bool stateOn = false;

/********************************** START SETUP*****************************************/
void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  setColor(0,0,0);

}




/********************************** START SETUP WIFI*****************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}




/********************************** START CALLBACK*****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }

  sendState();
}



/********************************** START PROCESS JSON*****************************************/
bool processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }
 
  if (root.containsKey("state")) {
    
    if (strcmp(root["state"], on_cmd) == 0) {
      Serial.println("Recieved new state command, turn ON.");
      stateOn = true;
      current_position = 0;
      wait_time = 0;
    }
    else if (strcmp(root["state"], off_cmd) == 0) {
      Serial.println("Recieved new state command, turn OFF.");
      stateOn = false;
      setColor(0,0,0);
    }
  }
  
  if (root.containsKey("strangerthingslight")) {
    wordChar = root["strangerthingslight"];
    wordString = String(wordChar);
    Serial.print("Received new stranger things light word: ");
    Serial.println(wordString);
    setColor(0,0,0);
    current_position = 0;
    wait_time = 0;
  }
  
  if (root.containsKey("strangerthingsspeed")) {
    animation_speed = root["strangerthingsspeed"];
    Serial.print("Received new stranger things light speed: ");
    Serial.println(animation_speed);
    setColor(0,0,0);
    current_position = 0;
    wait_time = 0;
  }
  
  return true;
}



/********************************** START SEND STATE*****************************************/
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (stateOn) ? on_cmd : off_cmd;
  root["speed"] = animation_speed;
  root["strangerthingslight"] = wordString;

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(light_state_topic, buffer, true);
}



/********************************** START RECONNECT*****************************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic);
      sendState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



/********************************** START Set Color*****************************************/
void setColor(int inR, int inG, int inB) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].red   = inR;
    leds[i].green = inG;
    leds[i].blue  = inB;
  }

  FastLED.show();

  Serial.print("Setting LEDs: ");
  Serial.print("r: ");
  Serial.print(inR);
  Serial.print(", g: ");
  Serial.print(inG);
  Serial.print(", b: ");
  Serial.println(inB);

//setup leds_rgb

    int color_temp = 0;
    for ( int i = 0; i < NUM_LEDS; i++){
      if (color_temp==0) 
      {
         leds_rgb[i].red   = 255;
         leds_rgb[i].green = 0;
         leds_rgb[i].blue  = 0;
      }
      else if (color_temp==1) 
      {
         leds_rgb[i].red   = 0;
         leds_rgb[i].green = 255;
         leds_rgb[i].blue  = 0;
      }
      else if (color_temp==2) 
      {
         leds_rgb[i].red   = 0;
         leds_rgb[i].green = 0;
         leds_rgb[i].blue  = 255;
      }
      color_temp++;
      if (color_temp==3) color_temp=0;
    }
  
}



/********************************** START MAIN LOOP*****************************************/
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }
  client.loop();


  int temp;
  int temp_r;
  int temp_g;
  int temp_b;
  

  if (current_position > wordString.length()) current_position = 0;
  wait_time++;
  
  if (wait_time == (1500*animation_speed) && stateOn) {
    temp=returnIndexForChar(wordString.charAt(current_position));
    Serial.print("Char #");
    Serial.print(current_position);
    Serial.print(":");
    Serial.print(wordString.charAt(current_position));
    Serial.print("=");
    Serial.print(temp);
    if (temp>=0) {
      temp_r=leds_rgb[temp].red;
      temp_g=leds_rgb[temp].green;
      temp_b=leds_rgb[temp].blue;
      leds[temp].setRGB(temp_r,temp_g,temp_b);
      Serial.print("... r=");
      Serial.print(temp_r);
      Serial.print(",g=");
      Serial.print(temp_g);
      Serial.print(",b=");
      Serial.println(temp_b);
      FastLED.show();
      current_position++;
    }
    else{
      Serial.println(" .. Invalid Char, No lights.");
      current_position++;
    }
  }
  if (wait_time == (3000*animation_speed)) {
      temp=returnIndexForChar(wordString.charAt(current_position-1));
      Serial.print("Turning off bulb#:");
      Serial.println(temp);
      leds[temp].setRGB(0,0,0);
      FastLED.show();
      wait_time = 0;
    }
}


//mapping letters to light position, assuming a 50 light string
int returnIndexForChar(char temp) {
  if (temp == 'A' || temp == 'a') return 49;
  if (temp == 'B' || temp == 'b') return 48;
  if (temp == 'C' || temp == 'c') return 47;
  if (temp == 'D' || temp == 'd') return 45;
  if (temp == 'E' || temp == 'e') return 44;
  if (temp == 'F' || temp == 'f') return 42;
  if (temp == 'G' || temp == 'g') return 40;
  if (temp == 'H' || temp == 'h') return 38;
  if (temp == 'I' || temp == 'i') return 21;
  if (temp == 'J' || temp == 'j') return 22;
  if (temp == 'K' || temp == 'k') return 24;
  if (temp == 'L' || temp == 'l') return 25;
  if (temp == 'M' || temp == 'm') return 27;
  if (temp == 'N' || temp == 'n') return 29;
  if (temp == 'O' || temp == 'o') return 31;
  if (temp == 'P' || temp == 'p') return 33;
  if (temp == 'Q' || temp == 'q') return 35;
  if (temp == 'R' || temp == 'r') return 17;
  if (temp == 'S' || temp == 's') return 16;
  if (temp == 'T' || temp == 't') return 14;
  if (temp == 'U' || temp == 'u') return 12;
  if (temp == 'V' || temp == 'v') return 10;
  if (temp == 'W' || temp == 'w') return 7;
  if (temp == 'X' || temp == 'x') return 5;
  if (temp == 'Y' || temp == 'y') return 2;
  if (temp == 'Z' || temp == 'z') return 0;
  return -1;
}
