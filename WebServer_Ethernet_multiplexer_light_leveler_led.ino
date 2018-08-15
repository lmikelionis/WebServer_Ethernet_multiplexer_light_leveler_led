//#include <RGB_Driver.h>
#include <I2C.h>
#include <EtherCard.h>
#include <Get_params_parser.h>


// LIGHTING
String ledStatus = "1";
String mode = "1";

String coldWhite = "0";
String warmWhite = "0";

String red = "150";
String green = "0";
String blue = "0";


int whiteLevel = 1;
int whiteLevelLock = 5;
int whiteLevelLockMax = 45;
int whiteLevelLockMin = 2;
int coldWhiteLevelMax = 250;
int coldWhiteLevelLockMin = 5;
int span = 1500;

int testTimeOffset = 30;

String rgbDriverMode = "";

// ETHERNET
static byte myip[] = { 192,168,1,46 };

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[400]; // tcp/ip send and receive buffer
BufferFiller bfill;

int timeOffset = 2500;
int sensVal = 0;


void setup() {
  Serial.begin(115200);
  Serial.println(F("====== SETUP HAD BEGAN ========"));

  // Scan only to get address ONCE !!!
  //  I2c.scan();

  
  // ETHERCARD SETUP
  if (ether.begin(sizeof Ethernet::buffer, mymac, 53) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }
  ether.staticSetup(myip);
  ether.printIp("IP:  ", ether.myip);

  // Setup sensor for light level monitoring
  I2c.begin();
  
  I2c.write(0x4A, 0x01, 0x0);
  I2c.write(0x4A, 0x02, 0x00);

  // resetLedOutputs();

  Serial.println(F("====== SETUP COMPLETED ========"));
  delay(1000);
}

void loop() {
  word len = ether.packetReceive();
  
  // resetLedOutputs(); // !!! IMPORTANT TO AVOID WHITE FLAS, AS MISO PIN IS USED FOR WHITE
  execute_RGB_program(ledStatus.toInt(), mode.toInt(), coldWhite.toInt(), warmWhite.toInt(), red.toInt(), green.toInt(), blue.toInt());
  
  word pos = ether.packetLoop(len);
    
  // check if valid tcp data is received
  if (pos) {

   Serial.print(F("HAR RECEIVED SOME DATA !!! "));
    
    bfill = ether.tcpOffset();

    char* data = (char *) Ethernet::buffer + pos;

    // Get your params.
    Get_params_parser *parmParser  = new Get_params_parser(data);

    String ledStatusTmp = parmParser->getParamValue("&1");
    
   if(isDigit(ledStatusTmp.charAt(0))) {

//    Serial.println(F("The ledStatusTmp was a number"));

    String ledStatusOld = ledStatus;

    if (ledStatusTmp.toInt() == 3) {
      Serial.println(F("The ledStatusTmp was a number 3"));
      ledStatus = ledStatusOld;
    } else {
      ledStatus = ledStatusTmp;
    }
    
    if (ledStatus.toInt() == 1) {

      String modeTmp = parmParser->getParamValue("&2");     
      String coldWhiteTmp = parmParser->getParamValue("&3");
      String warmWhiteTmp = parmParser->getParamValue("&4");
      String redTmp = parmParser->getParamValue("&5");
      String greenTmp = parmParser->getParamValue("&6");
      String blueTmp = parmParser->getParamValue("&7");
  
      delete parmParser;
    
    
        if (isDigit(modeTmp.charAt(0))) {
//          Serial.println(F("The modeTmp was a number"));
//          Serial.println(modeTmp);
            mode = modeTmp;
        }
    
        if (isDigit(coldWhiteTmp.charAt(0))) {
          coldWhite = coldWhiteTmp;
        }
    
        if (isDigit(warmWhiteTmp.charAt(0))) {
          warmWhite = warmWhiteTmp;
        }
    
        if (isDigit(redTmp.charAt(0))) {
          red = redTmp;
        }
    
        if (isDigit(greenTmp.charAt(0))) {
          green = greenTmp;
        }
    
        if (isDigit(blueTmp.charAt(0))) {
          blue = blueTmp;
        }
    } 
    
    if (ledStatus.toInt() == 0) {
      ledStatus = "0";
    }

  }
    word rspns = httpResponse(sensVal, ledStatus.toInt(), mode.toInt(), coldWhite.toInt(), warmWhite.toInt(), red.toInt(), green.toInt(), blue.toInt());
    ether.httpServerReply(rspns);
  }
  
  I2c.read(0x4A, 0x3, 6);
  sensVal = I2c.receive();
  
//  Serial.print(F("---- CURRENT LIGHT LEVEL SENSOR = "));
//  Serial.println(sensVal);
}

void execute_RGB_program(int state, int mode, int ww, int cw, int red, int green, int blue) {
  Serial.print(F("====== execute_RGB_program STATE = "));
  Serial.println(state);
  Serial.print(F("====== execute_RGB_program MODE = "));
  Serial.println(mode);
  
  if(state == 1) {
    if (mode == 0) {
      // ADAPTIVE
      Serial.println(F("====== WILL SET AJUST ========"));
//      adjustLight();
    } else if(mode == 1) {
      // MANUAL
      Serial.println(F("====== WILL SET MANUAL MODE 1 ========"));
      setManualMode(ww, cw, red, green, blue);
    } else if(mode == 2) {
      // TIMED
    } else if(mode == 3) {
      Serial.println(F("====== WILL SET DEMO ========"));
      testRgbCCTCycle();
      delay(100);
    }
  } else {
    Serial.println(F("====== LEDS ARE OFF ========"));
    turnLedsOff();
  }

    Serial.println(F("Current settings"));
//    Serial.println(ledStatus);
//    Serial.println(mode);
    Serial.println(coldWhite);
    Serial.println(warmWhite);
    Serial.println(red);
    Serial.println(green);
    Serial.println(blue);
    Serial.println(F("..."));
//    delay(500);/
}

void turnLedsOff() {
  setColdWhite(0);
  setWarmWhite(0);
  setRed(0);
  setGreen(0);
  setBlue(0); 
}

void setManualMode(int ww, int cw, int red, int green, int blue) {
  Serial.println(F("====== WILL SET MANUAL MODE 2 ========"));
//  delay(500);
  setColdWhite(ww);
  setWarmWhite(cw);
  setRed(red);
  setGreen(green);
  setBlue(blue);
  delay(10);
}

void setColor(uint8_t pin, int value ) {
  analogWrite(pin, value);
}

void setColdWhite(int value) {
  uint8_t whitePinn = 2;
  setColor(whitePinn, value);
}

void setWarmWhite(int value) {
  uint8_t warmWhite = 6;
  setColor(warmWhite, value);
}

void setRed(int value) {
  uint8_t redPin = 4;
  setColor(redPin, value);
}

void setGreen(int value) {
  uint8_t greenPin = 7;
  setColor(greenPin, value);
}

void setBlue(int value) {
  uint8_t bluePin = 9;
  setColor(bluePin, value);
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
      "\"version\": \"0.0.2\","
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
    Serial.println("---- RESPONSE WAS SENT  ---- ");
    return bfill.position();
}

void resetLedOutputs() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  analogWrite(2, 0);
  analogWrite(3, 0);
  analogWrite(4, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
}

void testRgbCCTCycle() {
  
  Serial.println("---- ENTERED INTO LED DEMO MODE  ---- ");
  
  setColdWhite(0);
  delay(testTimeOffset);
  setColdWhite(50);
  delay(testTimeOffset);
  setColdWhite(155);
  delay(testTimeOffset);
  setColdWhite(255);
  delay(testTimeOffset);
  setColdWhite(0);


  setWarmWhite(0);
  delay(testTimeOffset);
  setWarmWhite(50);
  delay(testTimeOffset);
  setWarmWhite(155);
  delay(testTimeOffset);
  setWarmWhite(255);
  delay(testTimeOffset);
  setWarmWhite(0);

  
  setRed(0);
  delay(testTimeOffset);
  setRed(50);
  delay(testTimeOffset);
  setRed(155);
  delay(testTimeOffset);
  setRed(255);
  delay(testTimeOffset);
  setRed(0);

  setGreen(0);
  delay(testTimeOffset);
  setGreen(50);
  delay(testTimeOffset);
  setGreen(155);
  delay(testTimeOffset);
  setGreen(255);
  delay(testTimeOffset);
  setGreen(0);

  setBlue(0);
  delay(testTimeOffset);
  setBlue(1);
  delay(testTimeOffset);
  setBlue(50);
  delay(testTimeOffset);
  setBlue(155);
  delay(testTimeOffset);
  setBlue(255);
  delay(testTimeOffset);
  setBlue(0);
}

