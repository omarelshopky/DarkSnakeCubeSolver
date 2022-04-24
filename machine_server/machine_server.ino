#include <ArduinoJson.h>

#include <ESP8266WiFi.h>      // Connect to a WiFi
#include <ESP8266WebServer.h> // Establish a webserver
#include <Servo.h>            // Handle PWM signal to control servos
#include <ArduinoJson.h>


/**     Define Pins     **/
#define CUBE_HOLDER_SERVO D6
#define TOP_COVER_SERVO D7
#define BUILTIN_LED D4

/**     Constants    **/
const char* AP_SSID = "Dark Solver";  
const char* AP_PASS = "darksolver"; 

// Configure the network connection
const IPAddress LOCAL_IP(192, 168, 1, 145);
const IPAddress GATEWAY(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);


/**     Global Variables    **/
String topCoverState = "open";
String cubeHolderState = "home";


/**     Global Objects    **/
Servo cubeHolder, topCover;

// A server object listen to port 80
ESP8266WebServer server(80); 

DynamicJsonDocument settings(3*JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(4) + 3*JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(3));



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

  /**   Configure the builtin led   **/
  pinMode(BUILTIN_LED, OUTPUT); 
  digitalWrite(BUILTIN_LED, !LOW);
  
  /**   Configure Serial Communication    **/
  Serial.begin(115200);

  /**   Set the server routes   **/
  server.on("/checkConnection", handleCheckConnection);
  server.on("/init", handleInit);
  server.on("/disconnect", handleDisconnect);
  server.on("/getSettings", handleGetSettings);
  server.on("/updateSettings", handleUpdateSettings);
  server.on("/flipTopCover", flipTopCover);
  server.on("/closeTopCover", closeTopCover);
  server.on("/openTopCover", openTopCover);
  server.on("/rotateClockwise", rotateClockwise);
  server.on("/rotateCounterClockwise", rotateCounterClockwise);
  server.on("/homeCubeHolder", homeCubeHolder);
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

  // Turn on the builtin led
  digitalWrite(BUILTIN_LED, !HIGH); 
  
  server.send(200, "text/plain", "Initialized");
}


/*
 * Turn off the builtin led when gui hit "/disconnect" endpoint
 */
void handleDisconnect() {
  // Turn off the builtin led
  digitalWrite(BUILTIN_LED, !LOW); 

  closeTopCover();
  
  server.send(200, "text/plain", "Disconnected");
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
  Serial.println(server.arg("plain"));

  server.send(200, "text/plain", "Updated");
}


/*
 * Flips the complete cube ("F") by "moving" the Front face to Bottom face using the top cover
 */
void flipTopCover() {
  topCover.write(settings["TOP_COVER"]["ANGLE"]["FLIP"]);
  delay(settings["TOP_COVER"]["TIME"]["FLIP_TO_CLOSE"]);
}


/*
 * Opens the top cover to free the complete cube
 */
void openTopCover() {
  topCover.write(settings["TOP_COVER"]["ANGLE"]["OPEN"]);
  delay(settings["TOP_COVER"]["TIME"]["FLIP_OPEN"]);
}


/*
 * Closes the top cover to hold the complete cube
 */
void closeTopCover() {
  topCover.write(settings["TOP_COVER"]["ANGLE"]["CLOSE"]);
  delay(settings["TOP_COVER"]["TIME"]["FLIP_OPEN"]);
}


/*
 * Rotates the complete cube clockwise
 */
void rotateClockwise() {
  String ang = settings["CUBE_HOLDER"];
  Serial.println(ang);
  cubeHolder.write(settings["CUBE_HOLDER"]["ANGLE"]["CW"]);
  delay(settings["CUBE_HOLDER"]["TIME"]["ROTATE"]);
}


/*
 * Rotates the complete cube counter clockwise
 */
void rotateCounterClockwise() {
  cubeHolder.write(settings["CUBE_HOLDER"]["ANGLE"]["CCW"]);
  delay(settings["CUBE_HOLDER"]["TIME"]["ROTATE"]);
}


/*
 * Returns the complete cube to home position
 */
void homeCubeHolder() {
  cubeHolder.write(settings["CUBE_HOLDER"]["ANGLE"]["HOME"]);
  delay(settings["CUBE_HOLDER"]["TIME"]["RELEASE"]);
}
