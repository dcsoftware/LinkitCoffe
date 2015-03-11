#include <ArduinoJson.h>
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <LWiFi.h>
#include <LWiFiUdp.h>
#include <LWiFiClient.h>
#include <Time.h>

#define SERIAL_COMMAND_REQUEST "request"
#define SERIAL_COMMAND_STORE "store"
#define TRANSACTION_ID "transaction_id"
#define TIMESTAMP "timestamp"
#define MACHINE_ID "machine_id"
#define MACHINE_NAME "machine_name"
#define MACHINE_LOCATION "machine_location"
#define USER_ID "user_id"
#define AMOUNT "amount"
#define SECRET_KEY "secret_key"
#define DATE_VALID "date_valid"
#define EVENT_TYPE "event_type"
#define WIFI_AP1 "Medical"
#define WIFI_AUTH1 LWIFI_WPA
#define WIFI_PASS "Medical09021972"
#define WIFI_AP2 "Belkin."
#define WIFI_AUTH2 LWIFI_WPA
#define WIFI_PASS2 "CorradiniCelaniStradelliGuelfi103"

#define Drv LFlash          // use Internal 10M Flash
//#define Drv LSD           // use SD card
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

char ssid[] = "Wireless_Touring";
char password[] = "";
String id;
String key;
String lastId;
LFile file;
LWiFiUDP Udp;

boolean last = false, logs = false, keys = false, machine = false;

void checkSecretKey() {
	LWiFiClient client;
	StaticJsonBuffer<300> keyBuffer;
	int mDay, mMonth, mYear;

	Drv.begin();

    if(!Drv.exists("keys.txt")) {
      Serial.println("log: cannot find file keys.txt;");
      file = Drv.open("keys.txt", FILE_WRITE);
      file.print("{\"day\":\"11\",\"month\":\"03\",\"year\":\"2015\",\"secret_key\":\"ABCDEFGHIJ\"}");
      file.close();
      keys = true;
    } else {
      file = Drv.open("keys.txt", FILE_READ);
      char jsonString[100];
      if(file.readBytes(jsonString, 100) == 0) {
        Serial.print("log: No data on file;");
      }

      JsonObject& root = keyBuffer.parseObject(jsonString);
      if(!root.success()) {
        Serial.println("log: error decoding key json;");
      } else {
        mDay = root["day"].as<int>();
        mMonth = root["month"].as<int>();
        mYear = root["year"].as<int>();
        Serial.print("log: key read from file;");
        if((mYear == year()) && (mMonth == month()) && (mDay == day())) {
        	Serial.println("log: secret key is up to date;");
        } else {
        	if(LWiFi.status() == LWIFI_STATUS_CONNECTED) {
        		Serial.println("log: connecting to backend;");
        		client.connect("winged-standard-741.appspot.com", 80);
        		delay(1000);
        		client.println("GET /gettodaykey HTTP/1.1");
        		client.println("Host: winged-standard-741.appspot.com");
        		client.println("Connection: close");
        		client.println();

        		while(!client.available()) {
        			delay(100);
        		}

        		char buff[200];
        		if(client.available()) {

        		client.readBytes(buff, sizeof(buff));
        		Serial.println(buff);
        		}
        	}
        }
      }
      file.close();
    }
}

void initializeData(){
	StaticJsonBuffer<300> jsonBuffer;

    if(!Drv.begin()) {
    Serial.println("Error initializing sd...");
    id = "00005678";
    key = "ABCDEFGHIJ";
    lastId = "20000000";
  } else {
    Serial.println("SD initialized...");
    
    if(!Drv.exists("machine.txt")) {
      Serial.println("log: cannot find file machine.txt;");
      file = Drv.open("machine.txt", FILE_WRITE);
      file.print("{\"machine_id\":\"00005678\",\"machine_name\":\"Macchina Prova 1\",\"machine_location\":\"laboratorio\"}");
      file.close();
      machine = true;
    } else {
      file = Drv.open("machine.txt", FILE_READ);
      char jsonString[100];
      if(file.readBytes(jsonString, 100) == 0) {
        Serial.print("log: No data on file;");
      }
            Serial.println(jsonString);

      JsonObject& root = jsonBuffer.parseObject(jsonString);
      if(!root.success()) {
        Serial.println("log: error decoding machine json;");
      } else {
        id = root[MACHINE_ID].asString();
        Serial.print("log: id read from file ");
        Serial.print(id);
        Serial.println(";");
        machine = true;
      }
      file.close();
    }
    
    if(!Drv.exists("keys.txt")) {
      Serial.println("log: cannot find file keys.txt;");
      file = Drv.open("keys.txt", FILE_WRITE);
      file.print("{\"day\":\"11\",\"month\":\"03\",\"year\":\"2015\",\"secret_key\":\"ABCDEFGHIJ\"}");
      file.close();
      keys = true;
    } else {
      file = Drv.open("keys.txt", FILE_READ);
      char jsonString[70];
      if(file.readBytes(jsonString, 70) == 0) {
        Serial.print("log: No data on file;");
      }
      Serial.println(jsonString);

      
      JsonObject& root = jsonBuffer.parseObject(jsonString);
      if(!root.success()) {
        Serial.println("log: error decoding key json;");
      } else {
        key = root[SECRET_KEY].asString();
        Serial.print("log: key read from file ");
        Serial.print(key);
        Serial.println(";");
        keys = true;
      }
      file.close();
    }
    
    if(!Drv.exists("last.txt")) {
      Serial.println("log: cannot find file last.txt;");
      file = Drv.open("last.txt", FILE_WRITE);
      file.print("{\"transaction_id\":\"10000000\",\"timestamp\":\"0000000000\"}");
      file.close();
      last = true;
    } else {
      file = Drv.open("last.txt", FILE_READ);
      char jsonString[70];
      if(file.readBytes(jsonString, 70) == 0) {
        Serial.print("log: No data on file;");
      }
      Serial.println(jsonString);
      JsonObject& root = jsonBuffer.parseObject(jsonString);
      if(!root.success()) {
        Serial.println("log: error decoding last json;");
      } else {
        lastId = root[TRANSACTION_ID].asString();
        Serial.print("log: last id read from file ");
        Serial.print(lastId);
        Serial.println(";");
        last = true;
      }
      file.close();
    }
    
    if(!Drv.exists("log.txt")) {
      Serial.println("log: cannot find file log.txt;");
      file = Drv.open("log.txt", FILE_WRITE);
      file.close();
      logs = true;
    } else {
      Serial.println("log: log.txt exists;");
      logs = true;
    }
  }
}

time_t setSystemTime() {
	const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets


	delay(1000);

	Serial.println("log: connecting to NTP server;");
	Udp.begin(2390);

	while (Udp.parsePacket() > 0) ; // discard any previously received packets
	Serial.println("Transmit NTP Request");
	sendNTPpacket("it.pool.ntp.org");
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = Udp.parsePacket();
	    if (size >= NTP_PACKET_SIZE) {
			Serial.println("Receive NTP Response");
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
	      // convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL;
	    }
	  }
	  Serial.println("No NTP Response :-(");
	  return 0;
}

void sendNTPpacket(String address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket("it.pool.ntp.org", 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


void setup() {
  // put your setup code here, to run once:
	Serial.begin(9600);
	while(!Serial);
  
	pinMode(10, OUTPUT);

	LTask.begin();
  
	Serial.println("Eccoci");

	LWiFi.begin();
	Serial.println("log: connecting wi-fi;");

	while (!LWiFi.connect(ssid)){
		delay(1000);
	    Serial.println("log: retry WiFi AP;");
	}

	Serial.println("log: connected to wi fi;");
	setSyncProvider(setSystemTime);
	setSyncInterval(600);
	checkSecretKey();
	initializeData();
  



	Serial.println("log: connecting to arduino mega;");
	Serial1.begin(9600);
	while(!Serial1);
	Serial1.println("ready");
	Serial.println("log: connected to arduino mega;");
}
void loop() {
	  Serial.print(hour());
	  Serial.print(":");
	  Serial.print(minute());
	  Serial.print(":");
	  Serial.print(second());
	  Serial.print(" ");
	  Serial.print(day());
	  Serial.print(" ");
	  Serial.print(month());
	  Serial.print(" ");
	  Serial.print(year());
	  Serial.println();
  // put your main code here, to run repeatedly:
  if(Serial1.available()) {    
    char buff[200];
    Serial.println("Reading from serial 1");
    Serial1.readBytesUntil(10, buff, sizeof(buff));

    if(0 == memcmp(buff, SERIAL_COMMAND_REQUEST, 7)) {
      Serial.println("log: request received;");
      Serial1.print(id);
      Serial1.print("/");
      Serial1.print(key);
      Serial1.print("/");
      Serial1.println(String(lastId));
    } else if (0 == memcmp(buff, SERIAL_COMMAND_STORE, 5)) {
      String c = buff;
      char jsonBuff[200];
      memcpy(jsonBuff, buff+6, sizeof(jsonBuff));
      Serial.println(jsonBuff);
      
      StaticJsonBuffer<300> newBuffer;
      JsonObject& object = newBuffer.parseObject(jsonBuff);
      
      if(!object.success()){
        Serial.println("Error decoding data");
      } else {
        if(logs) {
          file = Drv.open("log.txt", FILE_WRITE);
          object.printTo(file);
          file.close();
        }
      }
      
      if(object.containsKey(TRANSACTION_ID)) {  //Aggiunto questo per evitare che in login cancelli e ricrei last.txt perchè non ha il transaction id
        object.remove(MACHINE_ID);
        object.remove(AMOUNT);
        object.remove(USER_ID);
        object.remove(EVENT_TYPE);
      
        object.printTo(Serial);
      
        if(Drv.exists("last.txt")) {
          Serial.println("log: last.txt exists, removing and recreating;");
          Drv.remove("last.txt");
          file = Drv.open("last.txt", FILE_WRITE);
          object.printTo(file);
          file.close();
        } else {
          Serial.println("log: last.txt not exists, creating;");
          file = Drv.open("last.txt", FILE_WRITE);
          object.printTo(file);
          file.close();
        }
      }

    }
  }
  delay(5000);
}
