/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "eaglesnet";
const char* password = "";

ESP8266WebServer server(80);

String webPage = "";
int out;

int gpio0_pin = 0;
int gpio2_pin = 2;

void setup(void){
  webPage += "<h1>ESP8266 Web Server</h1><p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";
  webPage += "<p>Socket #2 <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";
  
  // preparing GPIOs
  pinMode(gpio0_pin, OUTPUT); //set it as output
  digitalWrite(gpio0_pin, LOW); //turn it off initially.
  pinMode(gpio2_pin, OUTPUT);   //set this pin as an output
  digitalWrite(gpio2_pin, LOW); //turn it off initially

  pinMode(A0, INPUT);
  out = (unsigned int) analogRead(A0);
 

        
  delay(1000);
  Serial.begin(115200);         //for serial port sake
  WiFi.begin(ssid, password);   //begin connection.
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  //print dots while connecting.
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());   //print the ip address to the serial.
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }

  
   
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  
  server.on("/socket1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio0_pin, HIGH);
    delay(1000);
  });
  
  server.on("/socket1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio0_pin, LOW);
    delay(1000); 
  });
  
  server.on("/socket2On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio2_pin, HIGH);
    delay(1000);
  });
  
  server.on("/socket2Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio2_pin, LOW);
    delay(1000); 
  });
  
  server.begin();
  Serial.println("HTTP server started");
}
 int count = 5;
void loop(void){
  server.handleClient();
  while(count > 0){
    Serial.println("The analog value is: "+ out);
    count--;
  }
} 
