
/*D.Bailey
  Science Museum of MN
  12-18-2020

  The code below is a proof of concept test for the TSCK crisper component. The purpose
  of this test is to detect valid combinations of RFID tag UIDS, and color spots,
  and upon finding a valid combination, send a message via SerialController indicating 
  the combiation name and its value as being true, or "1".
 
  This test uses an MFRC522 RFID card reader to read MIFARE 1K classic RIFD tag UIDs
  with the option to store the UIDs into the MCU's EEPROM.  To sense the color spots,
  an Adafruit TSC34725 color sensor is used.  When an RFID tag is sensed, it is checked 
  against the tag UIDs that are stored in EEPROM on the Arduino.
  
  To store a tag UID into EEPROM, an input pin assigned to a specific EEPROM address must be set "LOW".
  The next tag that is scanned after the input pin is set "LOW" will have its UID
  stored into the corresponding EEPROM location.  The UID requires 4 bytes of space.
  The stored tag UIDs are referenced inside conditional statements in the main loop.
*/

//included libraries
#include <Wire.h> //I2C
#include <SPI.h>
#include <EEPROM.h>
#include "SerialController.hpp" //SMM json parser
#include "MFRC522.h" //Card reader library
#include "Adafruit_TCS34725.h" //RGB color sensor library

//definitions
#define rstPin 9 //card reader reset pin
#define ssPin 10 //card reader serial data pin
#define tag1ProgramSwitch 14  //pin for program mode to store tag #1 to EEPROM
#define tag2ProgramSwitch 15  //pin for program mode to store tag #2 to EEPROM
#define baudrate 115200 //baudrate for serial com


//variables
String storedUID[2] = {"", ""};  //stored tag UID array
String tagRead = "";
bool storeUID = false;
uint8_t addressOffset = 0; //EEPROM address offset, each tag UID takes-up 4 bytes
uint8_t greenThreshold = 110;
uint8_t redThreshold = 150;
uint8_t blueThreshold = 110;
float red, green, blue;

//class instances
MFRC522 mfrc522(ssPin, rstPin);  //Create MFRC522 instance
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X); //Create TCS34725 instance
SerialController serialController; //create instance of SerialController

void setup() {
  boolean arduinoJsonDebug = false;
  serialController.setup(baudrate, &onParse);
  SPI.begin();  // init SPI bus
  Wire.begin(); //init I2C
  pinMode(tag1ProgramSwitch, INPUT_PULLUP);
  pinMode(tag2ProgramSwitch, INPUT_PULLUP);
  mfrc522.PCD_Init(); //init MFRC522

  //Load stored UID values from EEPROM into stored_UID array
  for (byte j = 0; j < 2; j++) { //iterate for the number of tags to be read
    for (byte i = 0 + addressOffset; i < 4 + addressOffset; i++) { //MIFARE classic 1K UID is 4 bytes in length
      if (EEPROM.read(i) < 0x10) storedUID[j].concat("0"); //if current byte is less than 16, add a leading "0" before hex value
      storedUID[j].concat(String(EEPROM.read(i), HEX));
      storedUID[j].toUpperCase();
    }
    //Serial.print("Tag UID stored at EEPROM address ");
    //Serial.print(addressOffset);
    //Serial.print(": ");
    //Serial.println(storedUID[j]);
    addressOffset += 4; //shift to the next 4 bytes in EEPROM to read the next tag UID
  }
}

//serialController parser function
void onParse(char* message, char* value) {
  if (strcmp(message, "wake-arduino") == 0 && strcmp(value, "1") == 0) {
    // you must respond to this message, or else
    // stele will believe it has lost connection to the arduino
    serialController.sendMessage("arduino-ready", "1");
  }
  else {
    // helpfully alert us if we've sent something wrong :)
    serialController.sendMessage("unknown-command", "1");
  }
}

//tag read function
void readTag() { //function to read tags and store UID into EEPROG if a programming switch is set "LOW"
  if (mfrc522.PICC_IsNewCardPresent()) { //look for presence of RFID tag
    if (mfrc522.PICC_ReadCardSerial()) { //check for readable data
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) tagRead.concat("0"); //if current byte is less than 16, add a leading "0" before hex value
        tagRead.concat(String(mfrc522.uid.uidByte[i], HEX));
        if (storeUID) EEPROM.write(i + addressOffset, mfrc522.uid.uidByte[i]); //write bytes to EEPROM if in programming mode
      }
      tagRead.toUpperCase();
      storeUID = false;
      // Serial.print("Scanned tag UID: ");
      //Serial.println(tagRead);
    }
  }
  else  return;
}

//main

void loop() {
  serialController.update();
  readTag(); //check for RFID tags
  tcs.getRGB(&red, &green, &blue); //get RGB values, returned range 0-255

  //conditionals
  if (tagRead == storedUID[0] && int(red) > redThreshold) {
    serialController.sendMessage("card1", "1");
  }
  else if (tagRead == storedUID[1] && int(blue) > blueThreshold) {
    serialController.sendMessage("card2", "1");
  }
  tagRead = "";//clear last tag read

  //check for any switched programming switches
  if (!digitalRead(tag1ProgramSwitch) && digitalRead(tag2ProgramSwitch)) {
    addressOffset = 0; //store tag #1 UID starting at 1st byte (address 0) in EEPROM
    storeUID = true;
  }
  else if (!digitalRead(tag2ProgramSwitch) && digitalRead(tag1ProgramSwitch)) {
    addressOffset = 4; //store tag #2 UID starting at 8th byte (address 8) in EEPROM
    storeUID = true;
  }
}
