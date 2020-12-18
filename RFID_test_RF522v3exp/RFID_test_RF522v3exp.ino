//included libraries
#include <Wire.h> //I2C
#include <SPI.h>
#include<EEPROM.h>
#include <MFRC522.h> //Card reader
#include "Adafruit_TCS34725.h" //RGB color sensor

//user defined variables
#define programSw1 14  //pin for program mode to store tag #1 to EEPROM
#define programSw2 15  //pin for program mode to store tag #2 to EEPROM
#define greenLED 2
#define redLED 3
#define rstPin 9 //card reader reset pin
#define ssPin 10 //card reader serial data pin
uint8_t numberTags = 2; //Number of UIDs to be stored in EEPROM
uint8_t tag1EEPROMaddress = 0; //starting address is 0
uint8_t tag2EEPROMaddress = 8; //subsequent tag EEPROM address are offset by another "8"

//other variables, do not change
String tagRead = "";
String storedUID[2] = {"", ""};
bool storeUID = false;
uint8_t addressOffset = 0;

//class instances
MFRC522 mfrc522(ssPin, rstPin);  //Create MFRC522 instance
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X); //Create TCS34725 instance


void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(programSw1, INPUT_PULLUP);
  pinMode(programSw2, INPUT_PULLUP);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  SPI.begin();  // init SPI bus
  Wire.begin(); //init I2C
  Serial.begin(9600);
  mfrc522.PCD_Init();   // Init MFRC522
  //Load stored UID values from EEPROM into stored_UID array
  for (byte j = 0; j < numberTags; j++) {
    for (byte i = 0 + addressOffset; i < 4 + addressOffset; i++) {
      if (EEPROM.read(i) < 0x10) storedUID[j].concat("0"); //if current byte is <16 add a leading "0" to hex value
      storedUID[j].concat(String(EEPROM.read(i), HEX));
      storedUID[j].toUpperCase();
    }
    Serial.print("Tag UID stored at EEPROM address ");
    Serial.print(addressOffset);
    Serial.print(": ");
    Serial.println(storedUID[j]);
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
  for (byte i = 0; i < mfrc522.uid.size; i++) {
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
  readTag(); //read tag function
  //Conditionals to store tag UID to EEPROM
  if (!digitalRead(programSw1)) {
    addressOffset = tag1EEPROMaddress;
    storeUID = true;
  }
  else if (!digitalRead(programSw2)) {
    addressOffset = tag2EEPROMaddress;
    storeUID = true;
  }

  //Conditionals to turn on/off pinouts
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
