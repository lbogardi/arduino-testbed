/* Copyright (c) 2015, Majenko Technologies All rights reserved. */
#include <stdlib.h>
#include <WEMOS_DHT12.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/* location related settings */
#define ESP8266_NAME "home"
#define FLOOR        "groundfloor"
#define ROOM         "wc"

#define STASSID      "---------"
#define STAPSK       "---------"

// leave this one GITHUB
//#define STASSID      "ssid"
//#define STAPSK       "pass"

// below is the guest wifi
//#define STASSID     "-----------------------"
//#define STAPSK      "-----------------------"

// the following lines should be commented out, 
// if the element is not used
#define UPTIME          1
#define SS_RELAY        1
#define RELAY           1
#define PIR_SENSOR      1
#define DHT12_SENSOR    1
#define BUILTIN_LED     1
#define LIGHTS          1
#define DEBUG           1          // all serial prints are supressed
// some of the code is not compiled into the binary
// if some of the lines above are commented out


// digital pin definitions
#define RELAY_PIN    D1  // GPIO0  relay connected to  GPIO0
#define PIR_PIN      D2  // GPIO4  passive infra receiver
#define BUILTIN_LED  D4  // GPIO2  led to turn on and off
#define SSR_PIN      D5  // GPIO12 solid state relay on GPIO2

// analog pin definitions
#define DHT12_PIN    11          // time/humidity sensor

//when to turn off light
#define timeSeconds 3

         int          PIR_state = 0;
unsigned long    previousMillis = 0;            // When the sensor was last read
unsigned long previousPIRMillis = 0;
unsigned long previousLEDMillis = 0;
const long         dht_interval = 1000;         // sensor reading frequency
const long         pir_interval = 1000;         // pir reading frequency
const long         led_interval = 1000;
const long       light_interval = 1000;
const long       relay_interval = 10000;
const char                *ssid = STASSID;
const char            *password = STAPSK;
volatile byte  interruptCounter = 0;
int          numberOfInterrupts = 0;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTriggerLED   = 0;
unsigned long lastTriggerRELAY = 0;

String  base_url        = "";

boolean startTimerLED   = false;
boolean startTimerRELAY = false;

// dht and esp web server objects, respectively
DHT12            dht12;
ESP8266WebServer server(80);

// ICACHE_RAM_ATTR ensures that ISR is in IRAM!
void ICACHE_RAM_ATTR handleInterrupt(void){
  interruptCounter++;
  startTimerLED    = true;
  startTimerRELAY  = true;
  
  lastTriggerLED   = millis();
  lastTriggerRELAY = millis();
}

void loop(void) {
  
  // lets do some house keeping
  autoLedOff();
  autoRelayOff();
  //autoLightOff();
  
  if(interruptCounter>0){
      interruptCounter--;
      numberOfInterrupts++;
      pirUpdate();
      //ledUpdate();
  }
  server.handleClient();
  MDNS.update();
}
    
void setup(void) {
  
  Serial.begin(115200);
  debug("WeMos DHT Server");
  //String myHostname = "esp8266";
  
  // /ground/kitchen/lights/toggle
  // /ground/kitchen/lights/state
  char baseURL[70];
  sprintf( baseURL, "/%s/%s", FLOOR, ROOM );
  
  // the IP address for the shield:
  //IPAddress  ip(192, 168, 86, 27);
  //IPAddress  gw(192, 168, 86,  1);
  //IPAddress  sn(255, 255,  0,  0);
  //IPAddress dns(192, 168, 86, 20);

  /* --------------------------- initialization ------------------------  */
  // how about DHT 
#ifdef PIR_SENSOR 
  pinMode(PIR_PIN, INPUT_PULLUP);
#endif

#ifdef BUILTIN_LED
  pinMode(LED_BUILTIN, OUTPUT);
#endif

#ifdef RELAY
  pinMode(RELAY_PIN, OUTPUT);
#endif

#ifdef SS_RELAY
  pinMode(SSR_PIN, OUTPUT);
#endif

  /* attach interrupt to the PIR, so it reports whenever there is detection */
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), handleInterrupt, RISING);
  
  /* --------------------------- wifi connect ------------------------  */
  wifi_station_set_hostname(ESP8266_NAME);
  WiFi.mode(WIFI_STA);
  //WiFi.enableInsecureWEP();
  WiFi.begin(ssid, password);
  //WiFi.config();
  
  while (WiFi.status() != WL_CONNECTED) {
    debug("."); 
	delay(500);
  }

  //String hostName = "home.lan";
  //String 

  if (!MDNS.begin(ESP8266_NAME)) {
    debug("Error setting up MDNS responder!"); 
	while (1) { delay(1000); }
  }
  
  debug("mDNS responder started");
  
  MDNS.addService("http", "tcp", 80);
  debug("mDNS http tcp services have been added.");
  
  /* --------------------------- handle different operations ------------------------  */
  server.onNotFound(handleNotFound);
  
  server.on("/",               handleRoot ); 
  server.on("/uptime",         readUptime );
  server.on("/state/json",     returnAllStateJson );
  
#ifdef PIR_SENSOR
  server.on("/pir/state",      pirState );
  server.on("/pir/delay",      pirDelay );
#endif  
  
#ifdef DHT12_SENSOR
  server.on("/temp",           readTemp );
  server.on("/temp/fahrenheit",readTempFahrenheit );
  server.on("/temp/celsius",   readTemp );
  server.on("/humidity",       readHumidity );
#endif

#ifdef BUILTIN_LED
  server.on("/led/on",     ledOn );
  server.on("/led/off",    ledOff );
  server.on("/led/state",  ledState );
  server.on("/led/toggle", ledToggle );
#endif

#ifdef LIGHTS
  server.on("/light/on",     ledOn );
  server.on("/light/off",    ledOff );
  server.on("/light/state",  ledState );
  server.on("/light/toggle", ledToggle );
#endif

#ifdef RELAY
  server.on("/relay/on",     relayOn );
  server.on("/relay/off",    relayOff );
  server.on("/relay/toggle", relayToggle );
  server.on("/relay/state",  relayState );
#endif

#ifdef SS_RELAY
  server.on("/ssrelay/on",     ssRelayOn );
  server.on("/ssrelay/off",    ssRelayOff );
  server.on("/ssrelay/toggle", ssRelayToggle );
  server.on("/ssrelay/state",  ssRelayState );
#endif

  server.begin();
  
  debug("HTTP server started\n");
}

void autoLedOff(void){
    now = millis();
    if(startTimerLED && (now - lastTriggerLED > (led_interval))) {
      turnLedOff();
      startTimerLED = false;
    }
}

void autoRelayOff(void){
    now = millis();
    if(startTimerRELAY && (now - lastTriggerRELAY > (relay_interval))) {
	  turnRelayOff();
      startTimerRELAY = false;
    }
}


// only reads every 2 seconds or so -> pir_interval
void readPIRSensor(void) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousPIRMillis >= pir_interval) {
    previousPIRMillis = currentMillis;
    int state = digitalRead(PIR_PIN);
    PIR_state = state;
  }
}

void pirUpdate(void){  
  int state = digitalRead(PIR_PIN);
  PIR_state = state;
  PIR_state == 1
             ? turnRelayOn()
			 : turnRelayOff();
}

void ledUpdate(void){  
  //PIR_state == 1 
  //           ? turnLedOn()
  //			 : turnLedOff();
}

void ledState(void){  
  int state = digitalRead(LED_BUILTIN);
  server.send(200, "text/html", String(!state) );
}


void pirState(void){
  readPIRSensor();
  server.send(200, "text/html", String(PIR_state) );
}

/* 
    ----- built-in LED related procedures -----
                                                     */

void turnLedOn(void){
  digitalWrite(LED_BUILTIN, 0);
  
}
  
void turnLedOff(void){
  digitalWrite(LED_BUILTIN, 1);
}

void ledOn(void){
  turnLedOn();
  server.sendHeader("Location","/");              
  server.send(303);
  }

void ledOff(void){
  turnLedOff();
  server.sendHeader("Location","/");              
  server.send(303);
  }

void ledToggle(void){
  int state = digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, not state);
  server.sendHeader("Location","/");              
  server.send(303);
  }

/* ----- RELAY related routines ----- */

void turnRelayOn(void){
  digitalWrite(RELAY_PIN, 1);
}
  
void turnRelayOff(void){
  digitalWrite(RELAY_PIN, 0);
}
  
void relayOn(void){
  turnRelayOn();
  server.sendHeader("Location","/");             
  server.send(303);
}
  
void relayOff(void){
  turnRelayOff();
  server.sendHeader("Location","/");              
  server.send(303);
}

void relayState(void){
    int state = digitalRead(RELAY_PIN);
    server.send(200, "text/html", String(state) );
  }

void relayToggle(void){
  int state = digitalRead(RELAY_PIN);
  state = not state;
  digitalWrite(RELAY_PIN, state);
  server.sendHeader("Location","/");              
  server.send(303);
  }

/* 
    ----- Solid State Relay related routines --- 
                                                  */

void turnSSRelayOn(void){
  digitalWrite(SSR_PIN, 0);
}
  
void turnSSRelayOff(void){
  digitalWrite(SSR_PIN, 1);
}
  
void ssRelayOn(void){
  turnSSRelayOn();
  server.sendHeader("Location","/");             
  server.send(303);
}
  
void ssRelayOff(void){
  turnSSRelayOff();
  server.sendHeader("Location","/");              
  server.send(303);
}

void ssRelayState(void){
    int state = digitalRead(SSR_PIN);
    server.send(200, "text/html", String(state) );
  }

void ssRelayToggle(void){
  int state = digitalRead(SSR_PIN);
  state = not state;
  digitalWrite(SSR_PIN, state);
  server.sendHeader("Location","/");              
  server.send(303);
  }

/* 
    PIR - passive infrared sensor related routines 
	                                                */

void pirOn(void){
  server.send(200, "text/html", "on" );
}

void pirOff(void){
  server.send(200, "text/html", "off" );
}

void pirDelay(void){
  server.send(200, "text/html", String(pir_interval) );
}

void returnAllStateJson(void){
  //loop through all the switches etc. and put everything in JSON
  String mystr;
  mystr += "{}";
  server.send(200, "text/json", mystr);
  }

void readUptime(void){
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr  = min / 60;
  char uptime[100];
  
  snprintf( uptime, 100, "<p>Uptime: %02d: %02d: %02d</p>\n", hr, min%60, sec%60);
                  
  server.send(200, "text/html", uptime);
  }

void readTemp(void){
    String mystr;
    read_sensor_DHT12();
    mystr = String(dht12.cTemp);
    server.send(200, "text/plain", mystr);
  }

void readTempFahrenheit(void){
    String mystr;
    read_sensor_DHT12();
    mystr = String(dht12.fTemp);
    server.send(200, "text/plain", mystr);
  }

void readHumidity(void){
    String mystr;
    read_sensor_DHT12();
    mystr = String(dht12.humidity);
    server.send(200, "text/plain", mystr);
}

/* 
    ----- timing related routines -----
                                                     */

void read_sensor_DHT12() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= dht_interval) {
    // Save the last time you read the sensor
    previousMillis = currentMillis;

    if(dht12.get()==0){
        debug("Celsius : "       + String(dht12.cTemp));
        debug("Fahrenheit : "    + String(dht12.fTemp));
        debug("Rel. Humidity : " + String(dht12.humidity));
      }
    else { 
	debug("Error while reading dht");
	}
  }
}

//void generalTimerFunc(){
//  unsigned long currentMillis = millis();
//  if (currentMillis - previousLEDMillis >= led_interval) {
//    previousLEDMillis = currentMillis;
//
//    if (PIR_state==1){
//      turnLedOn();
//      }else{
//      turnLedOff();
//    }
//  }
//}

/* 
    ----- PAGES served by the webserver -----
                                                     */

void handleRoot(){
	
  String baseURL = "/" + String(FLOOR) + "/" + String(ROOM);
  String temp;
  String message = "<html><head>\n";              //content='55'
  //message.reserve(2600); 
  
  message += "<meta http-equiv='refresh'                 />\n";
  message += "<title>ESP8266 Demo</title>\n";
  message +=   "<style>\n";
  message +=     "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\n";
  message +=     ".btn-group a {";
  message +=     "padding: 10px 24px; /* Some padding */";
  message +=     "border: 1px 1px 1px 1px;";
  message +=     "cursor: pointer; /* Pointer/hand icon */";
  message +=     "float: left; /* Float the buttons side by side */}";
  message +=     ".btn-group:after { content: ''; clear: both; display: table; }";
  message +=   "</style>\n";
  message += "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3.css\">";
  message += "</head>\n";
  message += "<body>\n";
  
  message +=   "<h1>" + String(ESP8266_NAME) + "</h1>\n";

#ifdef UPTIME
  message +=   "<div class=\"w3-container\">";
  message +=   "<div class=\"title\">uptime:</div>";
  message +=   "<div class='btn-group'>";
  message +=   "<a href='"+baseURL+"/uptime' class='w3-btn w3-white w3-border w3-border-blue w3-round'>uptime</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef LIGHTS
  message +=   "<div class=\"title\">lights</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/light/on' class='w3-btn w3-white w3-border w3-border-blue w3-round'>light on</a>\n";
  message +=     "<a href='/light/off' class='w3-btn w3-white w3-border w3-border-blue w3-round'>light off</a>\n";
  message +=     "<a href='/light/state' class='w3-btn w3-white w3-border w3-border-blue w3-round'>light state</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef BUILTIN_LED
  message +=   "<div class=\"title\">built-in led</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/led/on' class='w3-btn w3-white w3-border w3-border-blue w3-round'>led on</a>\n";
  message +=     "<a href='/led/off' class='w3-btn w3-white w3-border w3-border-blue w3-round'>led off</a>\n";
  message +=     "<a href='/led/state' class='w3-btn w3-white w3-border w3-border-blue w3-round'>led state</a>\n";
  message +=     "<a href='/led/toggle' class='w3-btn w3-white w3-border w3-border-blue w3-round'>led toggle</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef PIR_SENSOR
  message +=   "<div class=\"title\">pir - passive infrared sensor</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/pir/state' class='w3-btn w3-white w3-border w3-border-blue w3-round'>pir state</a>\n";
  message +=     "<a href='/pir/delay' class='w3-btn w3-white w3-border w3-border-blue w3-round'>pir delay</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef DHT12_SENSOR
  message +=   "<div class=\"title\">temperature / humidity</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/temp/celsius' class='w3-btn w3-white w3-border w3-border-blue w3-round'>celsius</a>\n";
  message +=     "<a href='/temp/fahrenheit' class='w3-btn w3-white w3-border w3-border-blue w3-round'>fahrenheit</a>\n";
  message +=     "<a href='/humidity' class='w3-btn w3-white w3-border w3-border-blue w3-round'>humidity</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef RELAY
  message +=   "<div class=\"title\">relay</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/relay/on' class='w3-btn w3-white w3-border w3-border-blue w3-round'>relay on</a>\n";
  message +=     "<a href='/relay/off' class='w3-btn w3-white w3-border w3-border-blue w3-round'>relay off</a>\n";
  message +=     "<a href='/relay/toggle' class='w3-btn w3-white w3-border w3-border-blue w3-round'>relay toggle</a>\n";
  message +=     "<a href='/relay/state' class='w3-btn w3-white w3-border w3-border-blue w3-round'>relay state</a>\n";
  message +=   "</div><br><br>";
#endif

#ifdef SS_RELAY
  message +=   "<div class=\"title\">ssr - solid state relay</div>";
  message +=   "<div class='btn-group'>";
  message +=     "<a href='/ssrelay/on' class='w3-btn w3-white w3-border w3-border-blue w3-round'>ssrelay on</a>\n";
  message +=     "<a href='/ssrelay/off' class='w3-btn w3-white w3-border w3-border-blue w3-round'>ssrelay off</a>\n";
  message +=     "<a href='/ssrelay/toggle' class='w3-btn w3-white w3-border w3-border-blue w3-round'>ssrelay toggle</a>\n";
  message +=     "<a href='/ssrelay/state' class='w3-btn w3-white w3-border w3-border-blue w3-round'>ssrelay state</a>\n";
  message +=   "</div><br><br>";
#endif

  message +=   "</div>";
//message +=   "<img src=\"/test.svg\" />\n";
  message += "</body>\n";
  message += "</html>\n";

  server.send(200, "text/html", message);
  }

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  //message += "\nArguments: " + server.args() + "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

void debug(String text){
  if (DEBUG){
	  Serial.println( text );
  }
}
