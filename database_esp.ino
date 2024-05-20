#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "12345689"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAWRKe57h-udVoLiQOFE6yy750e_Aa8V0A"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "rvardhan9984@gmail.com"
#define USER_PASSWORD "ecn351"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://temp-monitor-2ec14-default-rtdb.asia-southeast1.firebasedatabase.app"

//temperature calculation parameter
#define vRef 3.30
#define ADC_Resolution 4095
#define LM35_Per_Degree_Volt 0.01
//#define Zero_Deg_ADC_Value 550

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String timePath = "/timestamp";

//parentpath is the  Parent Node (to be updated in every loop with the current timestamp)
String parentPath;

int timestamp;   // used to save time. 
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";



const int lm35_pin = A0;                /* Connect LM35 out pin to A0 of ESP32*/
float temp_val, ADC_Per_Degree_Val;
//float temp_adc_val;
float temperature;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 1800;


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}


void setup(){
  Serial.begin(115200);

  ADC_Per_Degree_Val = (ADC_Resolution/vRef)*LM35_Per_Degree_Volt;


  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    
    json.set(tempPath.c_str(), String(findTemperature ())); //*****temp data ****//
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}

float findTemperature (){
  for (int i = 0; i < 10; i++) {
    temperature += analogRead(lm35_pin);  /* Read ADC value */
    delay(100);
  }
  temperature = temperature/20.0;
  temperature=(temperature/ADC_Per_Degree_Val);
  Serial.print("LM35 Temperature = ");
  Serial.print(temperature);           /* Print Temperature on the serial window */
  Serial.print("°C\n");
  delay(100);
  return temperature;
}
