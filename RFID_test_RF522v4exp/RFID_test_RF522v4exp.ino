
//D.Bailey
//Science Museum of MN
//12-18-2020

//included libraries
#include <Wire.h> //I2C
#include <SPI.h>
#include<EEPROM.h>
#include "MFRC522.h" //Card reader library
#include "Adafruit_TCS34725.h" //RGB color sensor library

//definitions
#define rstPin 9 //card reader reset pin
#define ssPin 10 //card reader serial data pin
#define greenLED 2
#define redLED 3
#define tag1ProgramSwitch 14  //pin for program mode to store tag #1 to EEPROM
#define tag2ProgramSwitch 15  //pin for program mode to store tag #2 to EEPROM

//variables
String storedUID[2] = {"", ""};  //stored tag UID array

//other variables, do not change
String tagRead = "";
bool storeUID = false;
uint8_t addressOffset = 0;

//class instances
MFRC522 mfrc522(ssPin, rstPin);  //Create MFRC522 instance
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X); //Create TCS34725 instance

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(tag1ProgramSwitch, INPUT_PULLUP);
  pinMode(tag2ProgramSwitch, INPUT_PULLUP);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  SPI.begin();  // init SPI bus
  Wire.begin(); //init I2C
  mfrc522.PCD_Init(); //init MFRC522
  Serial.begin(9600);

  //Load stored UID values from EEPROM into stored_UID array
  for (byte j = 0; j < 2; j++) { //iterate for the number of tags to be read
    for (byte i = 0 + addressOffset; i < 4 + addressOffset; i++) { //MIFARE classic 1K UID is 4 bytes in length
      if (EEPROM.read(i) < 0x10) storedUID[j].concat("0"); //if current byte is <16 add a leading "0" to hex value
      storedUID[j].concat(String(EEPROM.read(i), HEX));
      storedUID[j].toUpperCase();
    }
    Serial.print("Tag UID stored at EEPROM address ");
    Serial.print(addressOffset);
    Serial.print(": ");
    Serial.println(storedUID[j]);
    addressOffset += 8; //shift to next 8 bytes in EEPROM to read or write next tag UID
  }
}

//tag read function

void readTag() { //function to read tags and store UID into EEPROG if program mode selected
  if (!mfrc522.PICC_IsNewCardPresent()) { //look presence of RFID tag
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) { //check for readable data on the RFID tag
    return;
  }
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) tagRead.concat("0"); //if current byte is <16 add a leading "0" to hex value
    tagRead.concat(String(mfrc522.uid.uidByte[i], HEX));
    if (storeUID) EEPROM.write(i + addressOffset, mfrc522.uid.uidByte[i]); //write bytes to EEPROM if in programming mode
  }
  tagRead.toUpperCase();
  mfrc522.PICC_HaltA(); //stop reading until card is removed
  storeUID = false;
  Serial.print("Scanned tag UID: ");
  Serial.println(tagRead);
}

//main

void loop() {
  readTag(); //tag read function

  //check for any switched programming switches
  if (!digitalRead(tag1ProgramSwitch) && digitalRead(tag2ProgramSwitch)) {
    addressOffset = 0; //store tag #1 UID starting at 1st byte (address 0) in EEPROM
    storeUID = true;
  }
  else if (!digitalRead(tag2ProgramSwitch) && digitalRead(tag1ProgramSwitch)) {
    addressOffset = 8; //store tag #2 UID starting at 8th byte (address 8) in EEPROM
    storeUID = true;
  }

  //conditionals to turn on/off pinouts
  if (tagRead == storedUID[0]) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    tagRead = "";
  }
  else if (tagRead == storedUID[1]) {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    tagRead = "";
  }
}
