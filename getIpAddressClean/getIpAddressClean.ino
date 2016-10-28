/*
 Name:		getIpAddressClean.ino
 Created:	10/24/2016 12:07:02 PM
 Author:	Rene Moise Kwibuka
*/

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>

//ACCESS POINT DEFINITIONS.
const char WiFiAPPassword[] = "12345678";

//UDP DEFINITIONS
int udpPort = 2390;
WiFiUDP Udp;

//WIFI STATION DEFINITIONS
bool connected = false;		//It will be set to true if the microcontroller succ

// the setup function runs once when you press reset or power the board
void setup() {

	// Open serial communications
	Serial.begin(115200);
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
			WiFi.mode(WIFI_OFF);

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

	//Send microcontroller's ip address.
	sendUDPPacket(toSendData);
	
	
	//Disconnet Access Point wifi after a successful connection.
	//delay(10000);
	//WiFi.softAPdisconnect(true);
	Serial.println(WiFi.localIP());
	
	
}

// the loop function runs over and over again until power down or reset
void loop() {
  
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

	WiFi.softAP(AP_NameChar, WiFiAPPassword);
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

//void sendUDPPacket(char ReplyBuffer[256])
//{
//	int port = 4000;
//	//messageToSend.toCharArray(ReplyBuffer, messageToSend.length() + 1);
//	Udp.beginPacket(Udp.remoteIP(), port);
//	Serial.println("first");
//	Serial.println(ReplyBuffer);
//	Udp.write(ReplyBuffer, sizeof(ReplyBuffer));
//	Udp.endPacket();
//}


void sendUDPPacket(String messageToSend)
{
	char ReplyBuffer[256];
	int port = 4000;
	messageToSend.toCharArray(ReplyBuffer, messageToSend.length()+1);
	//ReplyBuffer[messageToSend.length()] = 0;
	Serial.println(ReplyBuffer);
	Udp.beginPacket(Udp.remoteIP(), port);
	Udp.write(ReplyBuffer);
	Udp.endPacket();
}

////char ReplyBuffer[256] = "ackwnowledgment";
//void sendUDPPacket()
//{
//	String test = "hello";
//	//char ReplyBuffer[256] = test;
//	char ReplyBuffer[256] = "ackwnowledgment";
//	int port = 4000;
//	//messageToSend.toCharArray(ReplyBuffer, messageToSend.length());
//	Udp.beginPacket(Udp.remoteIP(), port);
//	Udp.write(ReplyBuffer, sizeof(ReplyBuffer));
//	Udp.endPacket();
//}