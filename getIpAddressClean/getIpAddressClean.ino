/*
 Name:		getIpAddressClean.ino
 Created:	10/24/2016 12:07:02 PM
 Author:	Rene Moise Kwibuka
*/

//Includes.
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <FS.h>

//ACCESS POINT DEFINITIONS.
const char WIFI_AP_PASSWORD[] = "12345678";

//UDP DEFINITIONS
int udpPort = 2390;
WiFiUDP Udp;

//TIME DEFINITIONS
//int timeZone = -6;
//char line[80] = "";
//unsigned long epoch;
//unsigned long lastEpoch;
//File f;
//bool firstTimeWriting = true;
//String str = "";
//Dir dir;
ulong t = 1478982114L; //using an int for time will trigger an error in 2038.
//int pos;

//WIFI STATION DEFINITIONS
bool connected = false;		//It will be set to true if the microcontroller succ

//PIN DEFINITIONS
// CONTROLLING PINS
int outlet1 = D0, outlet2 = D1, outlet3 = D2, outlet4 = D3;
int outVoltage1 = 2, outVoltage2 = 2, outVoltage3 = 2, outVoltage4 = 2;
int outCurrent1 = 2, outCurrent2 = 2, outCurrent3 = 2, outCurrent4 = 2;
int status1 = 0, status2 = 0, status3 = 0, status4 = 0;
 

//COntroll sending 
int sendingVariable = 0;

//RECEIVED FROM ATMEGA 328P
String receivedFromAtmega;

//REMOTE IP ADDRESS
char remoteIP[15] = "192.168.4.2";
// the setup function runs once when you press reset or power the board
void setup() {

	
	setUpPins();
	digitalWrite(outlet1, HIGH);

	// Open serial communications
	Serial.begin(9600);
	//Serial1.begin()
	//Serial2.begin()
	Serial.println(t);
	Serial.println("Hello");
	
	while (!Serial) {
		; // wait for serial port to connect.
	}
  
	//Set up access point
	setupAPWiFi();

	//start udp.
	Udp.begin(udpPort);
	
	

	bool received = false; //It turns true if any packet is received.
	int noBytes;		//contains a value different from zero when it receives a packet.

	//Wait for packet until received.
	while (!received)
	{
		 noBytes = Udp.parsePacket();

		if (noBytes) 
		{
			//Receive a packet
			String packetReceived = "";
			packetReceived = receiveUDPPacket(noBytes);
			//Serial.println(packetReceived);
			

			//JSON PROCESSING
			StaticJsonBuffer<200> jsonBuffer;

			JsonObject& root = jsonBuffer.parseObject(packetReceived);

			if (!root.success())
			{
				Serial.println("parseObject() failed");
				return;
			}

			String  wifiName = root["wifiName"];
			String  password = root["password"];
			
			//change strings to character arrays so we can pass them to wifi.begin().
			char wifiNameChar[30];
			char passwordChar[30];
			wifiName.toCharArray(wifiNameChar, wifiName.length()+1);
			password.toCharArray(passwordChar, password.length()+1);

			Serial.println(wifiNameChar);
			Serial.println(passwordChar);
			//END JSON PROCESSING


			//Turn the wifi mode off so we can switch to both AP and STA.
			//WiFi.mode(WIFI_OFF);

			received = true;		//This becomes false again if the setup fails.

			//Start STA Connection
			if (!setupSTAMode(wifiNameChar, passwordChar))
			{
				//Set received to true
				received = false;
				WiFi.mode(WIFI_AP);	//Set Access Point Mode On.
			}
		}
	}

	//Send the ip address back to the phone
	
	 //Reserve memory space
	StaticJsonBuffer<200> jsonBufferSend;
	
	//Build object tree in memory
	JsonObject& root = jsonBufferSend.createObject();
	root["connected"] = "1";
	root["ipaddress"] = WiFi.localIP().toString();
	char messageToSend[50];
	root.printTo(messageToSend, sizeof(messageToSend));
	String toSendData = messageToSend;
	Serial.println("Sent: ");
	Serial.println(toSendData);
	delay (15000);
	
	Serial.println("Start sending a udp Packet");
	//Send microcontroller's ip address.
	sendUDPPacket(toSendData, 4000);
	digitalWrite(outlet1, HIGH);
	
	//Disconnet Access Point wifi after a successful connection.
	//delay(10000);
	//WiFi.softAPdisconnect(true);
	//Serial.println(WiFi.localIP());

	Serial.println("Setting pins");
	//SET UP PINS.
	
	Serial.println("Finished Setting pins");
	
}

// the loop function runs over and over again until power down or reset
void loop() {

	if (Serial.available())
	{
		receivedFromAtmega = Serial.readString();
		Serial.println("Receiving data");
		Serial.println(receivedFromAtmega);
		sendUDPPacket("Hello", 4000);
		delay(1000);
		sendUDPPacket(receivedFromAtmega, 4000);
	}
	Serial.println("DOne Receiveing");

	//if (receivedFromAtmega){
	//	Serial.println(str);
	//}
	//str = '\n';

	int noBytes;
	noBytes = Udp.parsePacket();
	
	if (noBytes)
	{
		String packetReceived = "";
		packetReceived = receiveUDPPacket(noBytes);

		Serial.println(packetReceived);

		//JSON PROCESSING
		StaticJsonBuffer<200> jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(packetReceived);

		if (!root.success())
		{
			Serial.println("parseObject() failed");
			return;
		}
		

		String  command = root["command"];
		String remIP = root["remoteIP"];
		Serial.println("Remote IP: ");
		Serial.println(remoteIP);

		if (remIP != "")
		{
			remIP.toCharArray(remoteIP, sizeof(remoteIP));
			
			Serial.println("New IP address: ");
			Serial.println(remoteIP);
		}
		
		//int status = 0;		//Holds an outlet status.

		if (command == "onOff1")
		{
			status1 = digitalRead(outlet1);		
			digitalWrite(outlet1, status1 = !status1);   // switch on to off or vice versa.		
		}
		else if (command == "onOff2")
		{
			status2 = digitalRead(outlet2);			
			digitalWrite(outlet2, status2 = !status2);   // switch on to off or vice versa.
			
		}
		else if (command == "onOff3")
		{
			status3 = digitalRead(outlet3);			
			digitalWrite(outlet3, status3 = !status3);   // switch on to off or vice versa.
			
		}
		else if (command == "onOff4")
		{
			status4 = digitalRead(outlet4);			
			digitalWrite(outlet4, status4 = !status4);   // switch on to off or vice versa.
			
		}
		else
		{
			//Do nothing for now.
		}
	}
	
	//if (sendingVariable < 5)
	{
		//Reserve memory space
		StaticJsonBuffer<250> jsonBufferSend;

		//status1 = 1;

		//Build object tree in memory
		JsonObject& root = jsonBufferSend.createObject();
		root["t"] = String(t);
		root["v"] = String(outVoltage1); 

		root["c1"] = String(outCurrent1); root["c2"] = String(outCurrent2);
		root["c3"] = String(outCurrent3); root["c4"] = String(outCurrent4);
		root["s1"] = String(status1); root["s2"] = String(status2);
		root["s3"] = String(status3); root["s4"] = String(status4);

		char messageToSend[200];
		root.printTo(messageToSend, sizeof(messageToSend));
		String toSendData = messageToSend;
		
		//send UDP Packet
		//delay(4000);
		sendUDPPacket(toSendData, 4000);
		t++;
		outCurrent1++; outCurrent2++; outCurrent3++; outCurrent4++;
		outVoltage1++;

		sendingVariable++;
	}	
	delay(1000);
  
}

String receiveUDPPacket(int maxSize)
{
	char packetBuffer[512]; //buffer to hold incoming and outgoing packets
	// We've received a packet, read the data from it
	int len = Udp.read(packetBuffer, maxSize); // read the packet into the buffer
	if (len > 0) {
		packetBuffer[len] = 0;
	}
	Serial.print("String Received: ");
	//Serial.println(packetBuffer);
	String packet = packetBuffer;
	return packet;
}


//Set up Access Point Wi-FI.
void setupAPWiFi()
{
	WiFi.mode(WIFI_AP);	//Set Access Point Mode On.

	// Do a little work to get a unique-ish name. Append the
	// last two bytes of the MAC (HEX'd) to "Thing-":
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.softAPmacAddress(mac);
	String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
		String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
	macID.toUpperCase();
	String AP_NameString = "OutSmart " + macID;

	char AP_NameChar[AP_NameString.length() + 1];
	memset(AP_NameChar, 0, AP_NameString.length() + 1);

	for (int i = 0; i < AP_NameString.length(); i++)
		AP_NameChar[i] = AP_NameString.charAt(i);

	WiFi.softAP(AP_NameChar, WIFI_AP_PASSWORD);
}

//Connect to the wifi.
bool setupSTAMode(char wifiName[], char password[])
{
	WiFi.mode(WIFI_AP_STA);
	WiFi.begin(wifiName, password);

	bool connected = true;
	int tries = 0;
	while (WiFi.status() != WL_CONNECTED) 
	{
		delay(500);
		Serial.print(".");
		tries++;
		if (tries > 30)
		{
			//Send UDP Packet to the phone app saying that the phone could not be connected.
			//Please try Again.
			//Go Back to listen.
			connected = false;	
			break;
		}
	}
	return connected;
}


void sendUDPPacket(String messageToSend, int port)
{
	char ReplyBuffer[256];	
	messageToSend.toCharArray(ReplyBuffer, messageToSend.length()+1);
	Udp.beginPacket(remoteIP, port);	
	Udp.write(ReplyBuffer);	
	Udp.endPacket();
}


void setUpPins()
{
	// sets controlling pins as outputs
	pinMode(outlet1, OUTPUT);    
	pinMode(outlet2, OUTPUT);
	pinMode(outlet3, OUTPUT);
	pinMode(outlet4, OUTPUT);
}