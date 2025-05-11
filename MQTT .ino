#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Adafruit IO credentials
#define IO_USERNAME  ""
#define IO_KEY       ""

// Adafruit IO server and port
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

// Wi-Fi credentials
const char* WLAN_SSID = "";
const char* WLAN_PASS = "";

// Create an instance of the WiFi client
WiFiClient client;

// Create an instance of the Adafruit MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

// Create Adafruit MQTT Subscribe objects
Adafruit_MQTT_Subscribe Loc = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/location");
Adafruit_MQTT_Subscribe GA = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/google-assistance");

// Create Adafruit MQTT Publish objects
Adafruit_MQTT_Publish locReset = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/location");
Adafruit_MQTT_Publish gaReset = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/google-assistance");

// Define the GPIO pin for output
const int ToCont = 4;  // GPIO 4 (D2)

// Function to connect to MQTT server
void MQTT_connect();

void setup() {
  Serial.begin(9600);

  pinMode(ToCont, OUTPUT);
 
  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  mqtt.subscribe(&Loc);
  mqtt.subscribe(&GA);
}

int Loc_State = 0, GA_State = 0;

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Loc) {
      Serial.println(F("Got_Loc: "));
      Serial.println((char *)Loc.lastread);
      Loc_State = atoi((char *)Loc.lastread);
    }  
    if (subscription == &GA) {
      Serial.print(F("Got_GA: "));
      Serial.println((char *)GA.lastread);
      GA_State = atoi((char *)GA.lastread);
      Serial.println("Received from GA");
    }
    
    // Check the condition and perform actions
    if (GA_State == 1 && Loc_State == 1) {
      // Send a pulse to the output pin
      digitalWrite(ToCont, HIGH);
      delay(100);  // Pulse duration
      digitalWrite(ToCont, LOW);
      Serial.println("ToCont pulsed");

      // Reset both feeds
      if (locReset.publish(0) && gaReset.publish(0)) {
        Serial.println("Both feeds reset to 0");
      } else {
        Serial.println("Failed to reset feeds");
      }
    }

    Send_Out();
  }
  
  Send_Out();
}

void Send_Out() {
  if(GA_State && Loc_State == 1) {
    digitalWrite(ToCont, HIGH);
    Serial.println("ToCont HIGH");
  } else {
    digitalWrite(ToCont, LOW);
    Serial.println("ToCont LOW");
  }
  
  Serial.print("Loca State: ");
  Serial.println(Loc_State);
  Serial.print("GA State: ");
  Serial.println(GA_State);
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
 
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);  // Infinite loop to indicate failure
    }
  }
  Serial.println("MQTT Connected!");
}
