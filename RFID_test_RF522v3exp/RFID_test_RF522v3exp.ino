
//User defined variables
uint8_t number_of_tags = 2; //Number of UIDs to be stored in EEPROM
uint8_t tag1_EEPROM_address = 0; //starting address is 0
uint8_t tag2_EEPROM_address = 8; //subsequent tag EEPROM address are offset by another "8"
#define programSw1 14  //pin for program mode to store tag #1 to EEPROM
#define programSw2 15  //pin for program mode to store tag #2 to EEPROM
#define greenLED 2
#define redLED 3
//
#include <Wire.h>
#include <SPI.h>
#include<EEPROM.h>
#include <MFRC522.h>
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);  //Create MFRC522 instance
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X); //Create TCS34725 instance
String tagRead = "";
String stored_UID[2] = {"", ""};
bool storeUID = false;
uint8_t addressOffset = 0;

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(programSw1, INPUT_PULLUP);
  pinMode(programSw2, INPUT_PULLUP);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  Serial.begin(9600);
  
  //***** Load stored UID values from EEPROM into stored_UID array*****
  for (uint8_t j = 0; j < number_of_tags; j++) { 
    for (uint8_t i = 0 + addressOffset; i < 4 + addressOffset; i++) {
      if (EEPROM.read(i) < 0x10) stored_UID[j].concat("0"); //if current byte is <16 add a leading "0" to Hex value
      stored_UID[j].concat(String(EEPROM.read(i), HEX));
      stored_UID[j].toUpperCase();
    }
    Serial.print("Tag UID stored at EEPROM address ");
    Serial.print(addressOffset);
    Serial.print(": ");
    Serial.println(stored_UID[j]);
    addressOffset = addressOffset + 8; //move to next tag address in EEPROM
  }
}

void readTag() { //function to read tags and store UID into EEPROG if program mode selected
  if (!mfrc522.PICC_IsNewCardPresent()) { //look presence of RFID tag
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) { //check for readable data on the RFID tag
    return;
  }
  for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) tagRead.concat("0");
    tagRead.concat(String(mfrc522.uid.uidByte[i], HEX)); //pad hex values <10 with a "0" for readability
    if (storeUID) EEPROM.write(i + addressOffset, mfrc522.uid.uidByte[i]); //write bytes to EEPROM if in programming mode
  }
  tagRead.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading until card is removed
  storeUID = false;
  Serial.print("This tag UID: ");
  Serial.println(tagRead);
}


void loop() {
delay(60);
  readTag();

  //Conditionals
  if (!digitalRead(programSw1)) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    addressOffset = tag1_EEPROM_address;
    storeUID = true;
  }
  else if (!digitalRead(programSw2)) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    addressOffset = tag2_EEPROM_address;
    storeUID = true;
  }
 if (tagRead == stored_UID[0]) { //change here the UID of the card/cards that you want to give access
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    tagRead = "";
  }
  else if (tagRead == stored_UID[1]) {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    tagRead = "";
  }

}
