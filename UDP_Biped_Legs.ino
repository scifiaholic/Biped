#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Servo.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wifi connection variables
const char* ssid = "SSID";
const char* password = "password";

// UDP variables
WiFiUDP UDP;
unsigned int localPort = 8888;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE+1]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged"; // a string to send back

String command = ""; //buffer to hold incoming packet,
bool commandEnd = false; //End of command word

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define pinLeftAnkle D4
#define pinLeftHip D3
#define pinwaist D6
#define pinRightHip D7
#define pinRightAnkle D8

#define pinLeftSholder D2
#define pinLeftArm D1
#define pinRightSholder D5
#define pinRightArm D0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Servo servoLeftAnkle;
Servo servoLeftHip;
Servo servowaist;
Servo servoRightHip;
Servo servoRightAnkle;

Servo servoLeftSholder;
Servo servoLeftArm;
Servo servoRightSholder;
Servo servoRightArm;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LAOffset = -2;
int LHOffset =  -8;
int WaOffset =  0;
int RHOffset =  -5;
int RAOffset = -8;

int LSOffset = 0;
int LaOffset = 0;
int RSOffset = 0;
int RaOffset = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // initialise pins
  servoLeftAnkle.attach(pinLeftAnkle);
  servoLeftHip.attach(pinLeftHip);
  servowaist.attach(pinwaist);
  servoRightAnkle.attach(pinRightAnkle);
  servoRightHip.attach(pinRightHip);

  servoLeftSholder.attach(pinLeftSholder);
  servoLeftArm.attach(pinLeftArm);
  servoRightSholder.attach(pinRightSholder);
  servoRightArm.attach(pinRightArm);
  
  servoLeftAnkle.write(90 + LAOffset);
  servoLeftHip.write(90 + LHOffset);
  servowaist.write(90 + WaOffset);
  servoRightAnkle.write(90 + RAOffset);
  servoRightHip.write(90 + RHOffset);

  servoLeftSholder.write(90 + LSOffset);
  servoLeftArm.write(90 + LaOffset);
  servoRightSholder.write(90 + RSOffset);
  servoRightArm.write(90 + RaOffset);
  
  // Initialise Serial connection
  Serial.begin(115200);
  Serial.print("Welcome to the ESP to Serial Biped, wait while I connect to: ");
  Serial.println(ssid);
  
  // Initialise wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  // only proceed if wifi connection successful
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("connected! IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" now lets begin....");
  Serial.println("");

  UDP.begin(localPort);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d ", packetSize, UDP.remoteIP().toString().c_str(), UDP.remotePort());
    int len = UDP.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    if (len > 0)
    {
      packetBuffer[len-1] = '\n';
    }
    
    Serial.printf("UDP packet contents: %s\n", packetBuffer);

    // send back a reply, to the IP address and port we got the packet from
    UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
    UDP.write(ReplyBuffer);
    UDP.endPacket();

    command = (String) packetBuffer;
    int endofcommand;
    endofcommand = command.indexOf('\n');
    command = command.substring(0,endofcommand);
    Serial.print("command: ");
    Serial.println(command);
    
    ////////////////////////
    //Left Joystick X-Axis//
    ////////////////////////
    int LeftXSpacer = command.indexOf('|');
    String LeftXWord = command.substring(0, LeftXSpacer);
    int LeftXInt = LeftXWord.toInt();
    Serial.print(" Left X Value: ");
    Serial.print(LeftXInt);

    LeftXInt = map(LeftXInt, 0, 1024, -14, 14);
    Serial.print(" - leaning: ");
    Serial.print(LeftXInt);

    
    ////////////////////////
    //Left Joystick Y-Axis//
    ////////////////////////
    int LeftYSpacer = command.indexOf('|', LeftXSpacer+1);
    String LeftYWord = command.substring(LeftXSpacer+1, LeftYSpacer);
    int LeftYInt = LeftYWord.toInt();
    Serial.print(" Left Y Value: ");
    Serial.print(LeftYInt);
    
    LeftYInt = map(LeftYInt, 0, 1024, -45, 45);
    Serial.print(" - hips: ");
    Serial.print(LeftYInt);

    ////////////////////////
    //Right Joystick X-Axis//
    ////////////////////////
    int RightXSpacer = command.indexOf('|', LeftYSpacer+1);
    String RightXWord = command.substring(LeftYSpacer+1, RightXSpacer);
    int RightXInt = RightXWord.toInt();
    Serial.print(" Right X Value: ");
    Serial.print(RightXInt);

    RightXInt = map(RightXInt, 0, 1024, -90, 90);
    Serial.print(" - sholders: ");
    Serial.print(RightXInt);

    
    ////////////////////////
    //Right Joystick Y-Axis//
    ////////////////////////
    int RightYSpacer = command.indexOf('|', RightXSpacer+1);
    String RightYWord = command.substring(RightXSpacer+1, RightYSpacer);
    int RightYInt = RightYWord.toInt();
    Serial.print(" Right Y Value: ");
    Serial.print(RightYInt);
    
    RightYInt = map(RightYInt, 0, 1024, -90, 90);
    Serial.print(" - arms: ");
    Serial.println(RightYInt);

    ///////////////
    //Left Button//
    ///////////////
    int LeftBSpacer = command.indexOf('|', RightYSpacer+1);
    String LeftBWord = command.substring(RightYSpacer+1, LeftBSpacer);
    
    ////////////////
    //Right Button//
    ////////////////
    int RightBSpacer = command.indexOf('|', LeftBSpacer+1);
    String RightBWord = command.substring(LeftBSpacer+1, RightBSpacer);

    ///////////////////////////
    //Write command to servos//
    ///////////////////////////
    setleaning(LeftXInt);
    sethips(LeftYInt, LeftBWord);
    setsholders(RightXInt, RightBWord);
    setarms(RightYInt, RightBWord);

    /////////////////
    //Clear command//
    /////////////////
    command = "";
    commandEnd = false;
    
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WAIST/ANKLES
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set the two feet and waist joints to be parallel
void setleaning(int angle){
  if(angle < 5 && angle > -5) {
    angle = 0;
  }
 
  //given an angle from -90 to 90 lean left to right respectively
  //angle = angle + 85;
  servoLeftAnkle.write(90 + LAOffset - angle);
  servowaist.write(angle/2 + 90 + WaOffset);
  servoRightAnkle.write(90 + RAOffset - angle);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HIPS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set the two hip joints parallel
void sethips(int angle, String mirrorOn){
  if(angle < 5 && angle > -5) {
    angle = 0;
  }

  //given an angle from -90 to 90 swivel hips Clock-wise and Counter Clock-wise respectively
  angle = angle + 90;

  servoLeftHip.write(angle + LHOffset);
  if(mirrorOn == "false") {
    servoRightHip.write(angle + RHOffset);
  }
  else{
    servoRightHip.write(180 - angle + RHOffset);    
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHOLDERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set the two arm and sholder joints opposite
void setsholders(int angle, String mirrorOff){
  if(angle < 5 && angle > -5) {
    angle = 0;
  }

  //given an angle from -90 to 90 swivel shoulders and arms Clock-wise and Counter Clock-wise respectively
  angle = angle + 90;

  servoLeftSholder.write(angle + LSOffset);  
  if(mirrorOff == "false") {
    servoRightSholder.write(angle + RSOffset);
  }
  else{
    servoRightSholder.write(180 - angle + RSOffset);    
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ARMS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set the two arm and sholder joints opposite
void setarms(int angle, String mirrorOff){
  if(angle < 5 && angle > -5) {
    angle = 0;
  }

  //given an angle from -90 to 90 swivel shoulders and arms Clock-wise and Counter Clock-wise respectively
  angle = angle + 90;

  servoLeftArm.write(angle + LSOffset);  
  if(mirrorOff == "true") {
    servoRightArm.write(angle + RSOffset);
  }
  else{
    servoRightArm.write(180 - angle + RSOffset);    
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////












