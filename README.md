# TSCK Crisper Arduino Code
  This code is a proof of concept test for the TSCK crisper component. The purpose
  of this test is to detect valid combinations of RFID tag UIDS, and color spots,
  and upon finding a valid combination, send a message via SerialController indicating 
  the combination name and its value as being true, or "1".
 
  This test uses an MFRC522 RFID card reader to read MIFARE 1K classic RIFD tag UIDs
  with the option to store the UIDs into EEPROM on the Arduino.  To sense the color spots,
  an Adafruit TSC34725 color sensor is used. When an RFID tag is sensed, it is checked 
  against the tag UIDs that are stored in EEPROM on the Arduino.
  
  To store a tag UID into EEPROM, an input pin assigned to a specific EEPROM address must be set "LOW".
  The next tag that is scanned after the input pin is set "LOW" will have its UID
  stored into the corresponding EEPROM location.  The UID requires 4 bytes of space.
  The stored tag UIDs are referenced inside conditional statements in the main loop.
