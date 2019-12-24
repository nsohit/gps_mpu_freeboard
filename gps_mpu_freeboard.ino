/*
  GENERIC SDK FOR ESP
  Add ESP8266 and PubSubClient libraries into your arduino libraries forlder
  You can find those library files inside the zip file.
  update the send_event function and do_actions function with respect your requirements.
*/

#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>
#define PUB_INTERVAL 30
//#include <MPU6050_tockn.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
unsigned long lastPublish = 0;
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

static const int RXPin = D3, TXPin = D4;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

const char* ssid = "RAHARJA";
const char* password = " ";

//Backend credentials
const char* mqtt_server = "iot-2/type/+/id/DEVICEID/evt/+/fmt/json";
String DEVICE_SERIAL = " " ; //update the device serial according to the serial given by the consumer portal


const char* EVENT_TOPIC = "quickstart.messaging.internetofthings.ibmcloud.com";
String SUB_TOPIC_STRING = "+/" + DEVICE_SERIAL + " ";


WiFiClient espClient;
PubSubClient client(espClient);
char msg[300];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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

//receiving a message
void callback(char* topic, byte* payload, long length) {
  Serial.print("Message arrived [");
  Serial.print(SUB_TOPIC_STRING);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  do_actions(msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "rabbit", "rabbit")) {
      Serial.println("connected");
      //subscribe to the topic
      const char* SUB_TOPIC = SUB_TOPIC_STRING.c_str();
      client.subscribe(SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  ss.begin(GPSBaud);



}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnect();
  }

  client.loop();

  if (millis() - lastPublish >= PUB_INTERVAL * 1000UL) {

    readSensor();
    
    lastPublish = millis();

  }

}

void publish_message(const char* message) {
  client.publish(EVENT_TOPIC, message);
}

//====================================================================================================================================================================
void send_event() {
  //create this function to send some events to the backend. You should create a message format like this
  /* Eg :{
        "mac":"6655591876092787",
        "eventName":"eventOne",
        "state":"none",
        "eventOne":{
          "ev1Value1":30
        }
      }

  */
  //Should call publish_message() function to send events. Pass your message as parameter
  // Eg : publish_message(message);
}
//====================================================================================================================================================================
//====================================================================================================================================================================
void do_actions(const char* message) {
  //Create this function according to your actions. you will receive a message something like this
  /* Eg : {
        "action":"actionOne",
        "param":{
          "ac1Value1":"1110" ,
          "parentMac":"6655591876092787",
          "ac1Value4":"port:03",
          "ac1Value5":"on",
          "mac":"6655591876092787",
          "ac1Value2":"2220",
          "ac1Value3":"567776"
        }
      } */


}
void readSensor() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
 
    gps.encode(ss.read());
    gps.location.isUpdated();
  /*
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  */
  String message = "{\"eventName\":\"HIMASIKOM\",\"status\":\"none\",\"line\":" + String (AcX) + ",\"lot\":" + String(gps.location.lng(), 6) + ",\"lat\":" + String(gps.location.lat(), 6) + ",\"MAC\":\"" + String(DEVICE_SERIAL) + "\"}";
  Serial.println(message);
  char* msg = (char*)message.c_str();
  publish_message(msg); // send the event to backend
}

   
 
