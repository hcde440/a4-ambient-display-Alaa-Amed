// ********************************************* \\
//                                               \\ 
//                Ambient Display                \\
//                    ITSPAAS                    \\
//                                               \\
// ********************************************* \\

// Project write up is included as a markdown page on github

//Requisite Libraries . . .
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>  // Include this library, which provides methods to send HTTP requests.
#include <ArduinoJson.h>        //provides the ability to parse and construct JSON objects   
#include "Wire.h"           
#include <PubSubClient.h>   

//////////
//So to clarify, we are connecting to and MQTT server
//that has a login and password authentication
//////////

#define BUTTON_PIN 5          // Push button pin 
#define  inputPin 4           // choose the input pin (for PIR sensor)
#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

              
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

// booleans for button state -- button can be pressed as an alternative to the motion sensor 
bool current = false;
bool last = false;

//////////
//We also need to publish and subscribe to topics, for this sketch are going
//to adopt a topic/subtopic addressing scheme: topic/subtopic
//////////

WiFiClient espClient;             
PubSubClient mqtt(espClient);   

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201];

// Wifi

#define wifi_ssid "University of Washington"
#define wifi_password ""

// boolean to check if trump is president  
bool yesOrNo = false;


typedef struct {    // here we create a new data type definition, a box to hold other data types
  String check;
} Trump;            //then we give our new data structure a name so we can use it in our code

Trump ishe;        //we have created a Trump type, but not an instance of that type,
                   //so we create the variable 'ishe' of type Trump


                   
void setup() { // Code runs only once or whenever we restart the board

  Serial.begin(115200);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  // Set push button and PIR sensor as input
  pinMode(BUTTON_PIN, INPUT);
  pinMode(inputPin, INPUT);     // declare sensor as input

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
      mqtt.subscribe("Alaa/+");     //we are subscribing to 'Alaa' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
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

  // Check to see if push button is pressed 
  if(digitalRead(BUTTON_PIN) == LOW)
    current = true;
  else
    current = false;
 
  // return if the value hasn't changed
  if(current == last)
    return;
  
  last = current;
  if (current) {
    Serial.println("pressed");
    
    // Make an API call if button is pressed
    checkIfStillPresident();
    delay(1000);
  }

  // PIR sensor tester 
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // Make an API call if motion is detected 
      checkIfStillPresident();
      delay(1000);
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  } 


  // If he is still president
  if (yesOrNo) {
    sprintf(message, "{\"Is_He?\":\"yes\"}"); 
    //publish message  
    mqtt.publish("Alaa/Trump", message);
   yesOrNo = false; 
  }
}//end Loop

void checkIfStillPresident() {   // function called checkIfStillPresiden that checks if Donald Trump is still President of the US.
  HTTPClient theClient;  // Use HttpClient object to send requests
  theClient.begin("http://istrumpstillpresident.io/itsp/Tom/Bob"); // API call with search query attached
  theClient.addHeader("Accept", "application/json");
  int httpCode = theClient.GET();
  if (httpCode > 0) { // if we get something back
    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer; //  Dynamic Json buffer is allocated on the heap and grows automaticallyis 
      // it is also the entry point for using the library: it handles the memory management and calls the parser
      JsonObject& root = jsonBuffer.parseObject(payload);
      
      if (!root.success()) { // Test if parsing succeeds.
        Serial.println("parseObject() failed in getMet()."); // if parsing doesn't successed, print that to serial monitor 
        return;
      }
       ishe.check = root["message"].as<String>(); 
       if (ishe.check == "Sadly Yes, Tom, Donald Trump is still the US President.") {
          yesOrNo = true;                                       
       }
    }
  }else {
    Serial.println("Something went wrong with connecting to the endpoint in getMet()."); // if we were not, for some reason, able to receive responses, then print this tp dserial monitor 
  }
}


 
