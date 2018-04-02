//#include <RGB_Driver.h>
#include <I2C.h>
#include <EtherCard.h>
#include <Vtech_multiplexer.h>
#include <Get_params_parser.h>


// LIGHTING
int whiteLevel = 1;
int whiteLevelLock = 5;
int whiteLevelLockMax = 45;
int whiteLevelLockMin = 2;
int coldWhiteLevelMax = 250;
int coldWhiteLevelLockMin = 5;
int span = 1500;

String rgbDriverMode = "";

// ETHERNET
static byte myip[] = { 192,168,1,46 };

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[400]; // tcp/ip send and receive buffer
BufferFiller bfill;

int timeOffset = 100;
int sensVal = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("====== SETUP HAD BEGAN ========"));

  // Scan only to get address ONCE !!!
  //  I2c.scan();
  
  // ETHERCARD SETUP
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }
  ether.staticSetup(myip);
  ether.printIp("IP:  ", ether.myip);

  // Setup sensor for light level monitoring
  I2c.begin();
  
  I2c.write(0x4A, 0x01, 0x0);
  I2c.write(0x4A, 0x02, 0x00);

  resetLedOutputs();
  
  Serial.println(F("====== SETUP COMPLETED ========"));
}

void execute_RGB_program(int mode) {
//  resetLedOutputs();
  if (mode == 0) {
    // ADAPTIVE
    adjustLight();
  } else if(mode == "TIMED") {
    // TIMED
  } else if(mode == "MANUAL") {
    // MANUAL
  } else if(mode == 3) {
    testRgbCCTCycle();
  }
}

void loop() {
  String mode = "3";
  execute_RGB_program(mode.toInt());

  resetLedOutputs();
  word len = ether.packetReceive();
  resetLedOutputs(); // !!! IMPORTANT TO AVOID WHITE FLAS, AS MISO PIN IS USED FOR WHITE
  word pos = ether.packetLoop(len);
    
  // check if valid tcp data is received
  if (pos) {
    bfill = ether.tcpOffset();

    char* data = (char *) Ethernet::buffer + pos;

    // Get your params.
    Get_params_parser parmParser(data);
    String rgbDriverState = parmParser.getParamValue("PARAM01");
    String mode = parmParser.getParamValue("PARAM02");     
    String coldWhite = parmParser.getParamValue("PARAM03");

    word rspns = httpResponse(sensVal, rgbDriverState.toInt(), mode.toInt(), coldWhite.toInt(), 0, 0, 0, 0);
    ether.httpServerReply(rspns);
  } else {
    resetLedOutputs();
    I2c.read(0x4A, 0x3, 6);
    sensVal = I2c.receive();
    resetLedOutputs();
    
    Serial.print(F("---- CURRENT LIGHT LEVEL SENSOR = "));
    Serial.println(sensVal);
    Serial.println(F("NOT RECEIVED A REQUEST"));
  }
}


void setColor(uint8_t pin, int value ) {
  analogWrite(pin, value);
  Serial.print(pin);
  Serial.print(" - ");
  Serial.println(value);
}

void setColdWhite(int value) {
  uint8_t whitePinn = 3;
  setColor(whitePinn, value);
}

void setWarmWhite(int value) {
  uint8_t warmWhite = 5;
  setColor(warmWhite, value);
}

void setRed(int value) {
  uint8_t redPin = 6;
  setColor(redPin, value);
}

void setGreen(int value) {
  uint8_t greenPin = 9;
  setColor(greenPin, value);
}

void setBlue(int value) {
  uint8_t redBlue = 10;
  setColor(redBlue, value);
}

int adjustLight() {
  setRed(whiteLevel);
    
  if (sensVal < whiteLevelLockMax && whiteLevel <= 240){
    whiteLevel++;
    setColdWhite(whiteLevel);
    setWarmWhite(whiteLevel);
    setRed(whiteLevel);

    if (whiteLevel > 140 && whiteLevel < 240) {
      setGreen(whiteLevel - 120);
    }
    
    delay(timeOffset);
  } else if (sensVal > whiteLevelLockMin && whiteLevel > 0){
    whiteLevel--;
    setColdWhite(whiteLevel);
    setWarmWhite(whiteLevel);
    setRed(whiteLevel);
    
    delay(timeOffset);
  }

//  Serial.print("---- CURRENT LIGHT POWER LEVEL  = ");
//  Serial.println(whiteLevel);
//  Serial.println(" ---- ---- ---- ---- ---- ---- ---- ");  
}

static word httpResponse(int sensorValue, int state, int mode, int coldWhite, int warmWhite, int red, int green, int blue) {
  bfill = ether.tcpOffset();
  bfill.emit_p(
  PSTR(
    "HTTP/1.0 200 You got the DATA\r\n"
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n"
      "{"
      "\"version\": \"0.0.1\","
      "\"reading\": \"$D\","
      "\"state\": \"$D\","
      "\"mode\": \"$D\","
      "\"coldWhite\": \"$D\","
      "\"warmWhite\": \"$D\","
      "\"red\": \"$D\","
      "\"green\": \"$D\","
      "\"blue\": \"$D\""
      "}"
    ), 
    sensorValue, state, mode, coldWhite, warmWhite, red, green, blue);
    Serial.println("---- RESPONSE WAS SENT  = ");
    return bfill.position();
}

void resetLedOutputs() {
//  pinMode(3, OUTPUT);
//  pinMode(5, OUTPUT);
//  pinMode(6, OUTPUT);
//  pinMode(9, OUTPUT);
//  pinMode(10, OUTPUT);

  analogWrite(3, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
  analogWrite(9, 0);
  analogWrite(10, 0);
}

void testRgbCCTCycle() {
  delay(2000);
//  setColdWhite(0);
//  delay(timeOffset);
//  setColdWhite(1);
//  delay(timeOffset);
//  setColdWhite(150);
//  delay(timeOffset);
//  setColdWhite(255);
//  delay(timeOffset);
//  setColdWhite(0);
//
//  setWarmWhite(0);
//  delay(timeOffset);
//  setWarmWhite(50);
//  delay(timeOffset);
//  setWarmWhite(100);
//  delay(timeOffset);
//  setWarmWhite(200);
//  delay(timeOffset);
//  setWarmWhite(255);
//  delay(timeOffset);
//  setWarmWhite(0);
//
  setRed(0);
  delay(timeOffset);
  setRed(50);
  delay(timeOffset);
  setRed(155);
  delay(timeOffset);
  setRed(255);
  delay(timeOffset);
  setRed(0);
//
  setGreen(0);
  delay(timeOffset);
  setGreen(50);
  delay(timeOffset);
  setGreen(155);
  delay(timeOffset);
  setGreen(255);
  delay(timeOffset);
  setGreen(0);

  setBlue(0);
  delay(timeOffset);
  setBlue(1);
  delay(timeOffset);
  setBlue(50);
  delay(timeOffset);
  setBlue(155);
  delay(timeOffset);
  setBlue(255);
  delay(timeOffset);
  setBlue(0);
}

