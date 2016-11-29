/*
 Name:		ProcessingSpeed.ino
 Created:	11/28/2016 3:00 pm
 Author:	Christian Wagner
*/

//Includes.
#include <eRCaGuy_Timer2_Counter.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <FS.h>

//CONSTANTS
const int NUMBER_OF_TIME_RECORDS = 50; //This is the amount of records we want to record for the experiment

//Variables to record time statistics for Experiment
unsigned long timeAtReceivingRecord[NUMBER_OF_TIME_RECORDS]; //Array to hold timestamps (in .5 microseond periods of when we receive records from the ATMega 328P)
unsigned long timeAtSendingRecord[NUMBER_OF_TIME_RECORDS]; //Array to hold timestamps (in .5 microseond periods of when we send records to the Android Mobile APP)
unsigned long timeTranspired[NUMBER_OF_TIME_RECORDS]; //Array to hold time transpired in microseconds betwen when we received the data and sent the data
unsigned long minimumTimeTranspired; //Minimum time transpired between receiving the data and sending the data
unsigned long maximumTimeTranspired; //Maximum time transpired between receiving the data and sending the data

//ACCESS POINT DEFINITIONS.
const char WIFI_AP_PASSWORD[] = "12345678";

//UDP DEFINITIONS
int udpPort = 2390;
WiFiUDP Udp;

//TIME AND STORAGE DEFINITIONS
int timeZone = -6; //The time zone we are in
unsigned long epoch; //The date and time in UNIX format
unsigned long lastEpoch; //The previous value of data and time in UNIX format
File f; //The file that has the power records on the ESP8266
unsigned int udpTimePort = 2391;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
String epochString;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

ulong t = 1478982114L; //using an int for time will trigger an error in 2038

					   //WIFI STATION DEFINITIONS
bool connected = false;		//It will be set to true if the microcontroller succeeds to connect

							//PIN DEFINITIONS
							// CONTROLLING PINS
int outlet1 = D0, outlet2 = D1, outlet3 = D2, outlet4 = D5;
int outVoltage1 = 2, outVoltage2 = 2, outVoltage3 = 2, outVoltage4 = 2;
int outCurrent1 = 2, outCurrent2 = 2, outCurrent3 = 2, outCurrent4 = 2;
int status1 = 0, status2 = 0, status3 = 0, status4 = 0;


//COntroll sending 
int sendingVariable = 0;

//RECEIVED FROM ATMEGA 328P
//String receivedFromAtmega;


//REMOTE IP ADDRESS
char remoteIP[15] = "192.168.4.2";

//mySerial COMMUNICATION DEFINITIONS
SoftwareSerial mySerial(D3, D4);

// the setup function runs once when you press reset or power the board
void setup() {

	timer2.setup();
	setUpPins();
	digitalWrite(outlet1, HIGH);

	// Initializations.
	mySerial.begin(9600);		//for serial debugging purposes.
	Serial.begin(9600);			//for ATMEGA328P serial communication	
	SPIFFS.begin();				//For file system.

	mySerial.println("STARTING POINT....");

	//start udp.
	Udp.begin(udpPort);


	bool received = false; //It turns true if any packet is received.
	int noBytes;		//contains a value different from zero when it receives a packet.

						//Wait for packet until received.
	while (!received)
	{
		noBytes = Udp.parsePacket();
		mySerial.println("Listening");
		if (noBytes)
		{
			//Receive a packet
			String packetReceived = "";
			packetReceived = receiveUDPPacket(noBytes);
			mySerial.println("Packet Received");
		    mySerial.println(packetReceived);


			//JSON PROCESSING
			StaticJsonBuffer<200> jsonBuffer;

			JsonObject& root = jsonBuffer.parseObject(packetReceived);

			if (!root.success())
			{
				mySerial.println("parseObject() failed");
				return;
			}

			String  wifiName = root["wifiName"];
			String  password = root["password"];

			//change strings to character arrays so we can pass them to wifi.begin().
			char wifiNameChar[30];
			char passwordChar[30];
			wifiName.toCharArray(wifiNameChar, wifiName.length() + 1);
			password.toCharArray(passwordChar, password.length() + 1);

			mySerial.println(wifiNameChar);
			mySerial.println(passwordChar);
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
		delay(1000);
	}

	//SEND THE IP ADDRESS BACK TO THE PHONE

	//Reserve memory space
	StaticJsonBuffer<200> jsonBufferSend;

	//Build object tree in memory
	JsonObject& root = jsonBufferSend.createObject();
	root["connected"] = "1";
	root["ipaddress"] = WiFi.localIP().toString();
	char messageToSend[50];
	root.printTo(messageToSend, sizeof(messageToSend));
	String toSendData = messageToSend;
	mySerial.println("Sent: ");
	mySerial.println(toSendData);
	delay(15000);

	mySerial.println("Start sending a udp Packet");
	//Send microcontroller's ip address.
	sendUDPPacket(toSendData, 4000);
	digitalWrite(outlet1, HIGH);

	//Disconnet Access Point wifi after a successful connection.
	//delay(10000);
	//WiFi.softAPdisconnect(true);
	//mySerial.println(WiFi.localIP());

	mySerial.println("Setting pins");
	//SET UP PINS.

	mySerial.println("Finished Setting pins");

	for (int i = 0; i < NUMBER_OF_TIME_RECORDS; i++) {

		//Get time
		GetTimeFromInternet();

		//Get Data
		String readData = getDataFromATMEGA();
		timeAtReceivingRecord[i] = timer2.get_count(); //Record time when data received

		//Record Time

		if (readData != " ")
		{
			//STORE DATA

			// open file for writing
			f = SPIFFS.open("f.txt", "a");
			f.println(readData);
			f.close();

			//delay(1000);
			timeAtSendingRecord[i] = timer2.get_count(); //Record time when data sent
			//mySerial.println("Before Sending");
			//SEND DATA
			//sendUDPPacket(readData, 4000);

			//mySerial.println("Done Sending");
		}
	}
	
	unsigned long numberToAverage = 0;
	unsigned long Lowest = timeTranspired[0];
	unsigned long Largest = timeTranspired[0];
	//Calculate time statistics for Experiment Report
	for (int i = 0; i < NUMBER_OF_TIME_RECORDS; i++) {

		timeTranspired[i] = (timeAtSendingRecord[i] - timeAtReceivingRecord[i])/2;
		numberToAverage += timeTranspired[i];
		if (timeTranspired[i] < Lowest) {
			Lowest = timeTranspired[i];
		}
		else if (timeTranspired[i] > Largest) {
			Largest = timeTranspired[i];
		}
	}
	unsigned long average = numberToAverage / NUMBER_OF_TIME_RECORDS;
	mySerial.println("The average was: " + average);
	mySerial.println("The maximum was: " + Largest);
	mySerial.println("The minimum was: " + Lowest);
}

//GET DATA FUNCTION
String getDataFromATMEGA()
{
	String temp;
	while (Serial.available()) {
		mySerial.println("Got data");
		if ((temp = Serial.readStringUntil('\n')) != 0) {

			//Serial.print(i);
			mySerial.print(":");

			mySerial.println(temp);
			mySerial.println("Done printing");
		}
	}

	StaticJsonBuffer<200> jsonBuffer; //Buffer to hold JSON object received from ATMega 328P
	//Parse object received from 328P
	JsonObject& root = jsonBuffer.parseObject(temp);

	if (!root.success())
	{
		mySerial.println("parseObject() failed");
		temp = " ";
		return temp;
	}

	//
	// Step 3: Retrieve the values
	//
	// Received in this form: {"t":"-7616","v":"0.00","c1":"0.05","c2":"4.01","c3":"8.66","c4":"12.95"}
	float        current1 = root["c1"];
	float        current2 = root["c2"];
	float        current3 = root["c3"];
	float        current4 = root["c4"];
	float        voltage = root["v"];

	StaticJsonBuffer<200> jsonBuffer1;
	JsonObject& root1 = jsonBuffer1.createObject();
	root1["v"] = voltage;
	root1["c1"] = current1;
	root1["c2"] = current2;
	root1["c3"] = current3;
	root1["c4"] = current4;
	root1["t"] = epochString;

    root1["s1"] = String(status1); root1["s2"] = String(status2);
   root1["s3"] = String(status3); root1["s4"] = String(status4);
   String dataToSend;
   root1.printTo(dataToSend);
   mySerial.println(dataToSend);
   return dataToSend;
}

//TIME FUNCTIONS
void GetTimeFromInternet()
{
	//get a random server from the pool
	WiFi.hostByName(ntpServerName, timeServerIP);

	sendNTPpacket(timeServerIP); // send an NTP packet to a time server

	// wait to see if a reply is available
	for (int w = 0; w<10; w++)
	{
		delay(100);
	}

	int cb = Udp.parsePacket();
	if (!cb) {
		mySerial.println("Fail");
	}
	else {
		//Serial.print("OK ");
		mySerial.println(cb);
		// We've received a packet, read the data from it
		Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

		//the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		// now convert NTP time into everyday time:
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;
		lastEpoch = epoch;
		epoch = secsSince1900 - seventyYears;
		//mostRecentFetch = millis();

		epochString = String(epoch); //String version of epoch to
		mySerial.println(epoch);
	}
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
	///Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}

String receiveUDPPacket(int maxSize)
{
	char packetBuffer[512]; //buffer to hold incoming and outgoing packets
							// We've received a packet, read the data from it
	int len = Udp.read(packetBuffer, maxSize); // read the packet into the buffer
	if (len > 0) {
		packetBuffer[len] = 0;
	}
	mySerial.print("String Received: ");
	//mySerial.println(packetBuffer);
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
		mySerial.print(".");
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
	messageToSend.toCharArray(ReplyBuffer, messageToSend.length() + 1);
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