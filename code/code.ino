
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Stream.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

/*
int pin_Out_S0 = 12;
int pin_Out_S1 = 13;
int pin_Out_S2 = 14;
int pin_In_Mux1 = A0;
int Mux1_State[8] = {0};
*/

struct clientData {
  int Temp[8];

};

int a = 0;

int   valget1;






//AWS
#include "sha256.h"
#include "Utils.h"

//WEBSockets
#include <Hash.h>
#include <WebSocketsClient.h>

//MQTT PAHO
#include <SPI.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"

//  --------- Config ---------- //
//AWS IOT config, change these:
char wifi_ssid[]       = "LoRa";
char wifi_password[]   = "12345678s";
char aws_endpoint[]    = "adaexyrkpjo5d-ats.iot.ap-southeast-1.amazonaws.com";
char aws_key[]         = "AKIA5N6LFCOKAAFEDQUY";
char aws_secret[]      = "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";
char aws_region[]      = "ap-southeast-1";
const char* aws_topic  = "$aws/things/iotjuly/shadow/update";
int port = 443;

// If stuff isn't working right, watch the console:
#define DEBUG_PRINT 1



//MQTT config
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;
// ---------- /Config ----------//

//DHT dht(DHTPIN, DHTTYPE);
ESP8266WiFiMulti WiFiMulti;

AWSWebSocketClient awsWSclient(256,5000);

IPStack ipstack(awsWSclient);

MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers> *client = NULL;

//# of connections
long connection = 0;

//generate random mqtt clientID
char* generateClientID () {
  char* cID = new char[23]();
  for (int i=0; i<22; i+=1)
    cID[i]=(char)random(1, 256);
  return cID;
}

//count messages arrived
int arrivedcount = 0;

//callback to handle mqtt messages
void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;

  if (DEBUG_PRINT) {
    Serial.print("Message ");
    Serial.print(++arrivedcount);
    Serial.print(" arrived: qos ");
    Serial.print(message.qos);
    Serial.print(", retained ");
    Serial.print(message.retained);
    Serial.print(", dup ");
    Serial.print(message.dup);
    Serial.print(", packetid ");
    Serial.println(message.id);
    Serial.print("Payload ");
    char* msg = new char[message.payloadlen+1]();
    memcpy (msg,message.payload,message.payloadlen);
    Serial.println(msg);



//--------------------------------------------------------------------------

 // char JSONMessage[] = "{\"state\":{\"reported\":{\"test_value1\":297, \"test_value2\":123}}}";
  Serial.print("Initial string value: ");
  Serial.println(msg);
 
 StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(msg); //Parse message
 
  if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(5000);
    return;
 
  }
 
//  const char * sensorType = parsed["SensorType"]; //Get sensor type value
  ///int value = parsed["Value"];                                         //Get value of sensor measurement
 
//  Serial.println(sensorType);
// Serial.println(value);

  const char* Temp = parsed["state"]["reported"]["test_value1"];

      
  int convTemp=atoi(Temp);

  Serial.print("The Temperature is ");
  Serial.println(convTemp);

 


  //-------------------------------------------------------------------------
  if(convTemp>290)
  {
    digitalWrite(16,HIGH);
    
    }
    else if(convTemp==200)
    {
      digitalWrite(16,LOW);
      
      }


    
    delete msg;
  }
}

//connects to websocket layer and mqtt layer
bool connect () {

    if (client == NULL) {
      client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
    } else {

      if (client->isConnected ()) {    
        client->disconnect ();
      }  
      delete client;
      client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
    }


    //delay is not necessary... it just help us to get a "trustful" heap space value
    delay (1000);
    if (DEBUG_PRINT) {
      Serial.print (millis ());
      Serial.print (" - conn: ");
      Serial.print (++connection);
      Serial.print (" - (");
      Serial.print (ESP.getFreeHeap ());
      Serial.println (")");
    }

   int rc = ipstack.connect(aws_endpoint, port);
    if (rc != 1)
    {
      if (DEBUG_PRINT) {
        Serial.println("error connection to the websocket server");
      }
      return false;
    } else {
      if (DEBUG_PRINT) {
        Serial.println("websocket layer connected");
      }
    }
    
    if (DEBUG_PRINT) {
      Serial.println("MQTT connecting");
    }
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    char* clientID = generateClientID ();
    data.clientID.cstring = clientID;
    rc = client->connect(data);
    delete[] clientID;
    if (rc != 0)
    {
      if (DEBUG_PRINT) {
        Serial.print("error connection to MQTT server");
        Serial.println(rc);
        return false;
      }
    }
    if (DEBUG_PRINT) {
      Serial.println("MQTT connected");
    }
    return true;
}


//subscribe to a mqtt topic
void subscribe () {
   //subscrip to a topic
    int rc = client->subscribe(aws_topic, MQTT::QOS0, messageArrived);
    if (rc != 0) {
      if (DEBUG_PRINT) {
        Serial.print("rc from MQTT subscribe is ");
        Serial.println(rc);
      }
      return;
    }
    if (DEBUG_PRINT) {
      Serial.println("MQTT subscribed");
    }
}


void setup() {
    Serial.begin (115200);   
    pinMode(16,OUTPUT);
    WiFiMulti.addAP(wifi_ssid, wifi_password);
    
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        if (DEBUG_PRINT) {
          Serial.print (". ");
        }
    }
    if (DEBUG_PRINT) {
      Serial.println ("\nconnected to network " + String(wifi_ssid) + "\n");
    }

    //fill AWS parameters    
    awsWSclient.setAWSRegion(aws_region);
    awsWSclient.setAWSDomain(aws_endpoint);
    awsWSclient.setAWSKeyID(aws_key);
    awsWSclient.setAWSSecretKey(aws_secret);
    awsWSclient.setUseSSL(true);


}


void loop() {

 

/*
  int valuee= analogRead(A0);
  if(valuee >=1000)
  {
     a=3500;
    }
    else
    {
     a=0;
      }

  String aa = String(a);    // Read temperature as Fahrenheit (isFahrenheit = true)
  */


  String values = "{\"state\":{\"reported\":{\"test_value1\":200,\"test_value2\":100}}}";
  // http://stackoverflow.com/questions/31614364/arduino-joining-string-and-char
  const char *publish_message = values.c_str();

  //keep the mqtt up and running
  if (awsWSclient.connected ()) {    
    
      client->yield();
      
    //  subscribe (); 
      //publish 
      MQTT::Message message;
      char buf[1000];
      strcpy(buf, publish_message);
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf)+1;
      int rc = client->publish(aws_topic, message);  
  } else {
    //handle reconnection
    connect ();
  }


 
  
}
