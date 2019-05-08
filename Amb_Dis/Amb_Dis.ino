// ********************************************* \\
//                                               \\ 
//                Ambient Display                \\
//                    ITSPAAS                    \\
//                                               \\
// ********************************************* \\

// Project write up is included as a markdown page on github

//Requisite Libraries . . .
#include <ESP8266WiFi.h> 
#include <SPI.h>
#include <Adafruit_Sensor.h>  // the generic Adafruit sensor library used with both sensors
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>y
#include <PubSubClient.h>   
#include <ArduinoJson.h>    
#include <Servo.h>

// variable decelerations
#define speaker 12
#define LED1 15 
#define LED2 14
#define LED3 5 
#define LED4 13
#define LED5 4  

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Wifi
#define wifi_ssid "University of Washington"
#define wifi_password ""

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server


char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

typedef struct { // here we create a new data type definition, a box to hold other data types
  String ishe;

} Trump;            

Trump hmm;            // we have created a Trump type, but not an instance of that type,
                     // so we create the variable 'hmm' of type Trump

WiFiClient espClient;             //blah blah blah, espClient
PubSubClient mqtt(espClient);     //blah blah blah, tie PubSub (mqtt) client to WiFi client

void setup() {
  Serial.begin(115200);
  
  // Set LEDs and speaker as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(speaker, OUTPUT);
  
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // Start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
} 

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("Alaa/Trump"); // we are subscribing to 'Alaa/Trump'
    } else {                        // If we are unable to connnect 
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/////LOOP/////
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'

}

void callback(char* topic, byte* payload, unsigned int length) {

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!
  
  // get whether Trump is still president from JSON and save it as a string 
  hmm.ishe = root["Is_He?"].as<String>();    
  
  if (hmm.ishe == "yes") { // if he is indeed still the president 
    // Change the value of brightness to 5 in order to enter the while loop
    brightness = 5;
    
    while (brightness > 0 ) {
      analogWrite(LED1, brightness);
      analogWrite(LED2, brightness);
      analogWrite(LED3, brightness);
      analogWrite(LED4, brightness);
      analogWrite(LED5, brightness);

      // change the brightness for next time through the loop:
      brightness = brightness + fadeAmount;
      //tone(speaker, brightness*10);

      // reverse the direction of the fading at the ends of the fade:
      if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount;
     }
      // wait for 30 milliseconds to see the dimming effect
      delay(50);
    }
      tone(speaker, 440); // play a 440 Hz tone on the speaker
      delay(10); // wait for 10 ms
      tone(speaker, 880, 30);
      delay(1000);
      noTone(speaker); // turn the speaker off
  }
  
  // Set the brightness of all the LEDs back to zero 
  analogWrite(LED1, 0);
  analogWrite(LED2, 0);
  analogWrite(LED3, 0);
  analogWrite(LED4, 0);
  analogWrite(LED5, 0);
  
  // Handling errors 
  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
}
