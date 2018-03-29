// SIMPLE REQUEST
// http://192.168.1.46/?&PAR01=255&PAR02=200|

//GET /?&PAR01=100&PAR02=200| HTTP/1.1
//Host: 192.168.1.46
//Connection: keep-alive
//Accept: */*
//Origin: null
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.90 Safari/537.36
//Accept-Encoding: gzip, deflate
//Accept-Language: lt,en-US;q=0.8,en;q=0.6,ru;q=0.4,pl;q=0.2


//#include <RGB_Driver.h>
#include <I2C.h>
#include <EtherCard.h>
#include <Vtech_multiplexer.h>
#include <Get_params_parser.h>


// LIGHTING
int redPin = 3;
int greenPin = 6;
int bluePin = 5;
int whiteWarmPin = 9;
int whiteColdPin = 10;

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
static byte gwip[] = { 192,168,1,254 };

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[400]; // tcp/ip send and receive buffer
BufferFiller bfill;

int timeOffset = 620;
int sensVal = 0;

Vtech_multiplexer multiplexer(0);

void setup() {
  Serial.begin(115200);
  Serial.println(F("====== SETUP HAD BEGAN ========"));

  // Scan only to get address ONCE !!!
  //  I2c.scan();
  
  // ETHERCARD SETUP
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }
    
  ether.staticSetup(myip);

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
  ether.printIp("NetMask: ", ether.netmask);

  // Setup sensor for light level monitoring
  I2c.begin();
  
  I2c.write(0x4A, 0x01, 0x0);
  I2c.write(0x4A, 0x02, 0x00);
    
  Serial.println(F("====== SETUP COMPLETED ========"));
}

void loop() {
    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);
    
    // check if valid tcp data is received
    if (pos) {
        bfill = ether.tcpOffset();

        char* data = (char *) Ethernet::buffer + pos;

        //String strrr((char *) Ethernet::buffer + pos);
    
        //      strcpy((char *)data,"0123");
        //      String myString = String((char *) Ethernet::buffer + pos);
        //      Serial.println(myString);

        // Get your params.
        Get_params_parser parmParser(data);
        String rgbDriverState = parmParser.getParamValue("PARAM01");
        String mode = parmParser.getParamValue("PARAM02");
        
        String coldWhite = parmParser.getParamValue("PARAM03");
        Serial.print(F("coldWhite: "));
        Serial.println(coldWhite);

//        Serial.println(p2.toInt());

        word rspns = httpResponse(sensVal, rgbDriverState.toInt(), mode.toInt(), 0, 0, 0, 0, 0);
        ether.httpServerReply(rspns);
    } else {
      I2c.read(0x4A, 0x3, 6);
      sensVal = I2c.receive();
//      Serial.print(F("---- CURRENT LIGHT LEVEL SENSOR = "));
//      Serial.println(sensVal);
//      Serial.println(F("NOT RECEIVED A REQUEST"));
    }

    multiplexer.writeTo(0, 255);
    multiplexer.writeTo(1, 255);

    multiplexer.writeTo(2, 255);
    multiplexer.writeTo(3, 255);
    multiplexer.writeTo(4, 255);
    delay(5000);

    multiplexer.writeTo(0, 0);
    multiplexer.writeTo(1, 0);

    multiplexer.writeTo(2, 0);
    multiplexer.writeTo(3, 0);
    multiplexer.writeTo(4, 0);
    delay(5000);
}

int adjustLight(uint8_t coldWhitePin, int warmWhitePin, int redPin) {
  
  analogWrite(redPin, whiteLevel); 
    
  if (sensVal < whiteLevelLockMax && whiteLevel <= 240){
    whiteLevel++;
    analogWrite(whiteWarmPin, whiteLevel); 
    analogWrite(redPin, whiteLevel);

    if (whiteLevel > 140 && whiteLevel < 240) {
      analogWrite(greenPin - 120, whiteLevel);
    }
    
    delay(timeOffset);
  } else if (sensVal > whiteLevelLockMin && whiteLevel > 0){
    whiteLevel--;
    analogWrite(whiteWarmPin, whiteLevel); 

    analogWrite(redPin, whiteLevel); 
    delay(timeOffset);
  }

    Serial.print("---- CURRENT LIGHT POWER LEVEL  = ");
    Serial.println(whiteLevel);
    Serial.println(" ---- ---- ---- ---- ---- ---- ---- ");  
}

void execute_RGB_program(String mode) {
  if (mode == 0) {
//    adjustLight(C0);/
  } else if(mode == "TIMED") {
    
  } else if(mode == "MANUAL") {
    
  } else if(mode == "EFFECTS") {
    
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

    return bfill.position();
}

