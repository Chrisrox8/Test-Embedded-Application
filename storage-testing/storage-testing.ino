// Very basic Spiffs example, writing 10 strings to SPIFFS filesystem, and then read them back
// For SPIFFS doc see : https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <FS.h>



//-- Preferences --
char ssid[] = "eaglesnet";  //  your network SSID (name)
char pass[] = "";       // your network password
int timeZone = -6;
//unsigned long getTimeEveryXmillis = 15 * 1000; // every 15 seconds

//unsigned long timeToGetTime
unsigned long epoch;
unsigned long lastEpoch;
File f;
bool firstTimeWriting = true;
String str = "";
Dir dir;
int t = 123456;
int pos; 
//unsigned long mostRecentFetch;

unsigned int localPort = 2391;      // local port to listen for UDP packets

									/* Don't hardwire the IP address or we won't get the benefits of the pool.
									*  Lookup the IP address for the host name instead */
									//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

String epochString;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

									// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

SoftwareSerial mySerial(D3, D4);

void setup() {
	Serial.begin(9600);
	Serial.setTimeout(900);
	mySerial.begin(9600);
	while (!Serial) {} //Wait for serial communication to start
	mySerial.println("\nVery basic Spiffs example, writing 10 lines to SPIFFS filesystem, and then read them back");
	SPIFFS.begin();
	// Next lines have to be done ONLY ONCE!!!!!When SPIFFS is formatted ONCE you can comment these lines out!!
	//Serial.println("Please wait 30 secs for SPIFFS to be formatted");
	//SPIFFS.format();
	//Serial.println("Spiffs formatted");

	//-- Open Serial port to Arduino --
	mySerial.println("Hi");

	//-- Connect to WiFi network --
	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	//-- Start UDP --
	udp.begin(localPort);
	mySerial.println("Ready!");
	dir = SPIFFS.openDir("/");
}

void loop() {

	writeRecordsToDevice("/f.txt");

		// open file for reading
		f = SPIFFS.open("/f.txt", "r");
		if (!f) {
			mySerial.println("file open failed");
		}  mySerial.println("======= Reading from SPIFFS file =======");
		int i = 0;
		// read strings from file
		String s;
		while((s = f.readStringUntil('\n')) != NULL) {
			//Serial.print(i);
			mySerial.print(":");
			mySerial.println(s);
			//i++;
		}
		
		f.close();
		while (dir.next()) {
			str += dir.fileName();
			str += " / ";
			str += dir.fileSize();
			str += "\r\n";
		}
		mySerial.print(str);
}

void writeRecordsToDevice(String path) {
	// get the time
	GetTimeFromInternet();
	if (lastEpoch == epoch) {
		epoch += 1;
		lastEpoch += 1;
		epochString = (String)epoch;
	}
	if (firstTimeWriting) {
		f = SPIFFS.open(path, "w");
		firstTimeWriting = false;
	}
	else {
		// open file for writing
		f = SPIFFS.open(path, "a");
	}

	if (!f) {
		mySerial.println("file open failed");
	}
	mySerial.println("====== Writing to SPIFFS file =========");
	// write 10 strings to file

	//Reserve memory space
	StaticJsonBuffer<200> jsonBuffer;
	//char temp[100]; //= "{\"t\":\"-7616\",\"v\":\"0.00\",\"c1\":\"0.05\",\"c2\":\"4.01\",\"c3\":\"8.66\",\"c4\":\"12.95\"}";
	int incomingSerialDataIndex = 0;
	
		//temp[incomingSerialDataIndex] = Serial.read(); // Add the incoming byte to the array
		//incomingSerialDataIndex++; // Ensure the next byte is added in the next position
	
		//Serial.readBytesUntil('\n',temp,73);
	//while (Serial.available() && pos < sizeof temp - 1) {

	//	// Read incoming byte.
	//	char c = Serial.read();
	//	temp[pos++] = c;

	//	// Echo received message.
	//	if (c == '\n') {            // \n means "end of message"
	//		temp[pos] = '\0';     // terminate the buffer
	//		mySerial.print(temp);   // send echo
	//		pos = 0;                // reset to start of buffer
	//	}
	//}

	//char buffer[75];
	bool read = false;
	String temp;
	while ( Serial.available()) {
		mySerial.println("Got data");
		if ((temp=Serial.readStringUntil('\n')) != 0) {
			
			//Serial.print(i);
			mySerial.print(":");

			mySerial.println(temp);
			mySerial.println("Done printing");
			read = true;
			//i++;
		}
		else
			mySerial.println("No string");
	}

	mySerial.println("Message received");
	mySerial.println(temp);
		//Parse object received from 328P
	JsonObject& root = jsonBuffer.parseObject(temp);

	if (!root.success())
	{
		mySerial.println("parseObject() failed");
		return;
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

	f.println(";V:" + (String)voltage + ",C1:" + (String)current1 + ",C2:" + (String)current2
		+ ",C3:" + (String)current3 + ",C4:" + (String)current4 + ",T:" + epochString);

	/*StaticJsonBuffer<200> jsonBuffer1;
	JsonObject& root1 = jsonBuffer1.createObject();
	root1["v"] = voltage + .01;
	root1["c1"] = current1 + .01;
	root1["c2"] = current2 + .01;
	root1["c3"] = current3 + .01;
	root1["c4"] = current4 + .01;
	root1["t"] = epochString;

	root1.printTo(temp);*/
	mySerial.println(temp);
	f.close();

}

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

	int cb = udp.parsePacket();
	if (!cb) {
		mySerial.println("Fail");
	}
	else {
		//Serial.print("OK ");
		mySerial.println(cb);
		// We've received a packet, read the data from it
		udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

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
		
		epochString = String(epoch);
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
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}