#include <ArduinoJson.h>

#include <ESP8266WiFi.h>      // Connect to a WiFi
#include <ESP8266WebServer.h> // Establish a webserver
#include <Servo.h>            // Handle PWM signal to control servos
#include <ArduinoJson.h>


/**     Define Pins     **/
#define CUBE_HOLDER_SERVO D6
#define TOP_COVER_SERVO D7


/**     Constants    **/
const char* AP_SSID = "Dark Solver";  
const char* AP_PASS = "darksolver"; 

// Configure the network connection
const IPAddress LOCAL_IP(192, 168, 1, 145);
const IPAddress GATEWAY(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);


/**     Global Variables    **/
// Top Servo (top cover) Angles
int closeAngle = 180;
int flipAngle = 100;
int openAngle = 138;

// Top Servo (top cover) Times
int closeTime = 100;
int flipTime = 100;
int openTime = 100;

// Bottom Servo (cube holder) Angles
int clockwiseAngle = 15;
int homeAngle = 92;
int counterClockwiseAngle = 160;

// Bottom Servo (cube holder) Angles
int clockwiseTime = 100;
int homeTime = 100;
int counterClockwiseTime = 100;

String topCoverState = "open";
String cubeHolderState = "home";


/**     Global Objects    **/
Servo cubeHolder, topCover;

// A server object listen to port 80
ESP8266WebServer server(80); 

DynamicJsonDocument settings(3*JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(3));



void setup() {
  /**   Configure Access Point    **/
  WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
  WiFi.softAP(AP_SSID, AP_PASS);
  
  /**   Configure Servos    **/
  cubeHolder.attach(CUBE_HOLDER_SERVO, 600, 2500);
  cubeHolder.write(90);
  delay(200);
  topCover.attach(TOP_COVER_SERVO, 500, 2740);
  topCover.write(180);
  
  /**   Configure Serial Communication    **/
  Serial.begin(115200);

  /**   Set the server routes   **/
  server.on("/checkConnection", handleCheckConnection);
  server.on("/init", handleInit);
  server.on("/getSettings", handleGetSettings);
  server.on("/updateSettings", handleUpdateSettings);
  server.onNotFound(handleNotFound);

  /**   Establish the server    **/
  server.begin();
}


void loop() {
  // Start the server listening
  server.handleClient();
  
  if (Serial.available()) {
    char userInstruction = Serial.read();

    switch (userInstruction) {
      case 'o':
        openTopCover(); break;
      case 'c':
        closeTopCover(); break;
      case 'f':
        flipTopCover(); break;
      case 'r':
        rotateClockwise(); break;
      case 'l':
        rotateCounterClockwise(); break;
      case 'h':
        homeCubeHolder(); break;
    }
  }
}

/*
 * Handle the client when request "/checkConnection" to check the machine is connected
 */
void handleCheckConnection() {
  server.send(200, "text/plain", "Connected"); 
}


/*
 * Handle the client when request a not found endpoint
 */
void handleNotFound(){
  server.send(404, "text/plain", "Not Found");
}


/*
 * Initalize the settings json and the servos position when gui hits "/init" endpoint
 */
void handleInit () {
  // Parse post request body to settings json
  deserializeJson(settings, server.arg("plain")); 

  // Initalize servos position
  homeCubeHolder();
  openTopCover();
  
  server.send(200, "text/plain", "Initialized");
}


/*
 * Returns the settings json when gui hits "/getSettings" endpoint
 */
void handleGetSettings() {
  // Parse Json to string to send it as response
  String settingsString;
  serializeJson(settings, settingsString);
  
  server.send(200, "text/plain", settingsString);
}


/*
 * Updates the settings json when gui hits "/updateSettings" endpoint
 */
void handleUpdateSettings() {
  // Parse post request body to settings json
  deserializeJson(settings, server.arg("plain"));

   server.send(200, "text/plain", "Updated");
}


/*
 * Flips the complete cube ("F") by "moving" the Front face to Bottom face using the top cover
 */
void flipTopCover() {
  topCover.write(flipAngle);
  delay(flipTime);
}


/*
 * Opens the top cover to free the complete cube
 */
void openTopCover() {
  topCover.write(openAngle);
  delay(openTime);
}


/*
 * Closes the top cover to hold the complete cube
 */
void closeTopCover() {
  topCover.write(closeAngle);
  delay(closeTime);
}


/*
 * Rotates the complete cube clockwise
 */
void rotateClockwise() {
  cubeHolder.write(clockwiseAngle);
  delay(clockwiseTime);
}


/*
 * Rotates the complete cube counter clockwise
 */
void rotateCounterClockwise() {
  cubeHolder.write(counterClockwiseAngle);
  delay(counterClockwiseTime);
}


/*
 * Returns the complete cube to home position
 */
void homeCubeHolder() {
  cubeHolder.write(homeAngle);
  delay(homeTime);
}
