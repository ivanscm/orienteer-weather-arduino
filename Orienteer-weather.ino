#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <SPI.h>
#include <Ethernet.h>

#define DHTPIN 2
#define DHTTYPE DHT22

// How frequently to send information?
#define DELAY_MILLIS 30000UL // 30sec

#define REST "/orientdb/document/db"

#define SERVER_NAME "weather.orienteer.org"
#define SERVER_PORT 80
#define AUTH_BASE64 "YWRtaW46YWRtaW4="
#define DEVICE_ID "ivan_cabinet"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED //de:ad:be:ef:fe:ed
};

DHT_Unified dht(DHTPIN, DHTTYPE);
EthernetClient client;

char params[128];

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[64];

  Serial.print(F("connecting..."));

  if(client.connect(domainBuffer,thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Authorization: Basic %s", AUTH_BASE64);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    // send the body
    client.print(thisData);
  } 
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}

void setup() {
  Serial.begin(9600); 
  dht.begin();
  delay(1000);
  Ethernet.begin(mac);
}

void loop() {
  // uncomment for debug
  /*
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  */

  thisMillis = millis();

  if(thisMillis - lastMillis > DELAY_MILLIS)
  {
    lastMillis = thisMillis;

    sensors_event_t event;  
    dht.temperature().getEvent(&event);
    int temperature = event.temperature;
    dht.humidity().getEvent(&event);
    int humidity = event.relative_humidity;
    
    sprintf(params,"{'@class':'Weather', 'temperature':%i, 'humidity':%i, 'device':'%s'}", temperature, humidity, DEVICE_ID);
    
    if(!postPage(SERVER_NAME, SERVER_PORT, REST, params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
  }

}
