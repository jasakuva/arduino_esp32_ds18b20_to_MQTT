#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define ONE_WIRE_BUS D3 //Pin to which is attached a temperature sensor
#define ONE_WIRE_MAX_DEV 5 //The maximum number of devices

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
int numberOfDevices; //Number of temperature devices found
DeviceAddress devAddr[ONE_WIRE_MAX_DEV];  //An array device temperature sensors
float tempDev[ONE_WIRE_MAX_DEV]; //Saving the last measurement of temperature
float tempDevLast[ONE_WIRE_MAX_DEV]; //Previous temperature measurement
long lastTemp; //The last measurement
const int durationTemp = 5000;

const char* ssid = "xxxx";
const char* password = "xxxx";
const char* mqtt_server = "192.168.xxx.xxx";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


/********************************************************************/


//Setting the temperature sensor
void SetupDS18B20(){
  
  delay(2000);
  DS18B20.begin();
  delay(2000);
  Serial.print("Parasite power is: "); 
  if( DS18B20.isParasitePowerMode() ){ 
    Serial.println("ON");
  }else{
    Serial.println("OFF");
  }
  
  numberOfDevices = DS18B20.getDeviceCount();
  Serial.print( "Device count: " );
  Serial.println( numberOfDevices );

  lastTemp = millis();
  DS18B20.requestTemperatures();

  // Loop through each device
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if( DS18B20.getAddress(devAddr[i], i) ){
      //devAddr[i] = tempDeviceAddress;
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.println();
    }else{
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }

  }
}


float getTemperature (int devIndex) {
   float tempC = DS18B20.getTempC( devAddr[devIndex] );
   DS18B20.setWaitForConversion(false); //No waiting for measurement
   DS18B20.requestTemperatures(); //Initiate the temperature measurement
   return tempC;
}


void setup_wifi() {

  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}


void setup(void) 
{ 
 // start serial port 
 Serial.begin(9600); 
 
 // Start up the library 
 SetupDS18B20();

 setup_wifi();
 client.setServer(mqtt_server, 1883);
 //client.setCallback(callback);
 //client.publish("office/sensor2", "setupissa");
 
} 

void loop(void) 
{ 

 float Temperature;

 Temperature = getTemperature ( 0 );
 
  if (!client.connected()) {
    reconnect();
  }
  //client.loop();
  delay(2000);
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
  
  snprintf (msg, 75, "{\"temperature\":%.2f,\"rssi\":%ld}", Temperature,rssi);
  client.publish("office/sensor1", msg, 1);

  Serial.print("message:");
  Serial.println(msg);

  delay(3000);
  
  ESP.deepSleep(120e6);
  
  
} 
