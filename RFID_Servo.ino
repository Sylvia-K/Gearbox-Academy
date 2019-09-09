#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <Servo.h>

#define standbyLed 5
#define openLed 4
#define closedLed 3
#define wipebutton 8

Servo servo1;
int pos = 20;

bool programMode = false;  // initialize programming mode to false

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM

// Create MFRC522 instance.
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);


void setup() 
{
  servo1.attach(6); 
  pinMode(wipebutton, INPUT);
  pinMode(standbyLed, OUTPUT);
  pinMode(openLed, OUTPUT);
  pinMode(closedLed, OUTPUT);
  digitalWrite(standbyLed, HIGH);
  digitalWrite(openLed, LOW);
  digitalWrite(closedLed, LOW);

  Serial.begin(9600);  // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  
  Serial.println(F("Be welcome."));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if (EEPROM.read(1) != 143) 
  {
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    do 
    {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(standbyLed, HIGH);    // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(standbyLed, LOW);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) 
      {        // Loop 4 times
        EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
      }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Defined"));
  }
  
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) 
    {          // Read Master Card's UID from EEPROM
      masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
      Serial.print(masterCard[i], HEX);
    }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
}


void loop() 
{
  //Wipe Code - If the Button (wipeB) Pressed while setup run (powered on) it wipes EEPROM
  if (digitalRead(wipebutton) == HIGH) 
  {  // when button pressed pin should get low, button connected to ground
    digitalWrite(closedLed,   HIGH); // Red Led stays on to inform user we are going to wipe
    Serial.println(F("Wipe Button Pressed"));
    Serial.println(F("You have 10 seconds to Cancel"));
    Serial.println(F("This will be remove all records and cannot be undone"));
    bool buttonState = monitorWipeButton(1000); // Give user enough time to cancel operation
    if (buttonState == true && digitalRead(wipebutton) == HIGH) 
    {    // If button still be pressed, wipe EEPROM
      Serial.println(F("Starting Wiping EEPROM"));
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) 
        {    //Loop end of EEPROM address
          if (EEPROM.read(x) == 0) 
          {              //If EEPROM address 0
            // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
          }
          else 
          {
            EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
          }
        }
      Serial.println(F("EEPROM Successfully Wiped"));
      digitalWrite(closedLed, LOW);  // visualize a successful wipe
      delay(200);
      digitalWrite(closedLed,   HIGH);
      delay(200);
      digitalWrite(closedLed, LOW);
      delay(200);
      digitalWrite(closedLed,   HIGH);
      delay(200);
      digitalWrite(closedLed, LOW);
    }
    else 
    {
      Serial.println(F("Wiping Cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
      digitalWrite(closedLed, LOW);
    }
  }
  
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    
    if (programMode) 
    {
      cycleLeds();              // Program Mode cycles through Red Green Blue waiting to read a new card
    }
    else 
    {
      digitalWrite(standbyLed, HIGH);  // Normal mode, blue Power LED is on, all others are off
    }
    
    // When device is in use if wipe button pressed for 10 seconds initialize Master Card wiping
   /* if (digitalRead(wipeB) == LOW) 
    { // Check if button is pressed
      // Visualize normal operation is iterrupted by pressing wipe button Red is like more Warning to user
      digitalWrite(redLed, LED_ON);  // Make sure led is off
      digitalWrite(greenLed, LED_OFF);  // Make sure led is off
      digitalWrite(blueLed, LED_OFF); // Make sure led is off
      // Give some feedback
      Serial.println(F("Wipe Button Pressed"));
      Serial.println(F("Master Card will be Erased! in 10 seconds"));
      bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
      if (buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
        EEPROM.write(1, 0);                  // Reset Magic Number.
        Serial.println(F("Master Card Erased from device"));
        Serial.println(F("Please reset to re-program Master Card"));
        while (1);
      }
      Serial.println(F("Master Card Erase Cancelled"));
    }*/
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
  
  if (programMode) 
  {
    if ( isMaster(readCard) ) 
      { //When in program mode check First If master card scanned again to exit program mode
        Serial.println(F("Master Card Scanned"));
        Serial.println(F("Exiting Program Mode"));
        Serial.println(F("-----------------------------"));
        programMode = false;
        return;
      }
    else 
    {
      if ( findID(readCard) ) 
        { // If scanned card is known delete it
          Serial.println(F("I know this PICC, removing..."));
          deleteID(readCard);
          Serial.println("-----------------------------");
          Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
      else 
        {                    // If scanned card is not known add it
          Serial.println(F("I do not know this PICC, adding..."));
          writeID(readCard);
          Serial.println(F("-----------------------------"));
          Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        }
    }
  }
  else 
  {
    if ( isMaster(readCard)) 
      {    // If scanned card's ID matches Master Card's ID - enter program mode
        programMode = true;
        Serial.println(F("Hello Master - Entered Program Mode"));
        uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
        Serial.print(F("I have "));     // stores the number of ID's in EEPROM
        Serial.print(count);
        Serial.print(F(" record(s) on EEPROM"));
        Serial.println("");
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        Serial.println(F("Scan Master Card again to Exit Program Mode"));
        Serial.println(F("-----------------------------"));
      }
    else 
      {
        if ( findID(readCard) ) 
          { // If not, see if the card is in the EEPROM
            Serial.println(F("Welcome, You shall pass"));
            granted(300);         // Open the door lock for 300 ms
          }
        else 
          {      // If not, show that the ID was not valid
            Serial.println(F("You shall not pass"));
            denied();
          }
       }
  }

  
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) 
{
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, LOW);
  for (pos = 20; pos <= 160; pos += 3) 
      { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        servo1.write(pos);              // tell servo to go to position in variable 'pos'
        delay(100);   // waits 15ms for the servo to reach the position
        digitalWrite(openLed, HIGH); 
      }
  delay(setDelay);          // Hold door lock open for given seconds
  for (pos = 160; pos <= 20; pos -= 3) 
      { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        servo1.write(pos);              // tell servo to go to position in variable 'pos'
        delay(100);   // waits 15ms for the servo to reach the position
        digitalWrite(openLed, HIGH); 
      } 
  digitalWrite(openLed, LOW);
  digitalWrite(standbyLed, HIGH);
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() 
{
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, HIGH);
  delay(5000);
  digitalWrite(closedLed, LOW);
  digitalWrite(standbyLed, HIGH);
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() 
{
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
    { //If a new PICC placed to RFID reader continue
      return 0;
    }
  if ( ! mfrc522.PICC_ReadCardSerial()) 
    {   //Since a PICC placed get Serial and continue
      return 0;
    }
    
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) 
    {  //
      readCard[i] = mfrc522.uid.uidByte[i];
      Serial.print(readCard[i], HEX);
    }
    
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void ShowReaderDetails() 
{
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, HIGH);
    while (true); // do not go further
  }
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) 
{
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) 
    {     // Loop 4 times to get the 4 Bytes
      storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
    }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) 
{
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) 
    {    // Loop once for each EEPROM entry
      readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
      if ( checkTwo( find, storedCard ) ) 
      {   // Check to see if the storedCard read from EEPROM
        return true;
      }
      else 
      {    // If not, return false
         
      }
    }
    return false;
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) 
{   
  for ( uint8_t k = 0; k < 4; k++ )
    {   // Loop 4 times
      if ( a[k] != b[k] ) 
      {     // IF a != b then false, because: one fails, all fail
         return false;
      }
    }
  return true;  
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) 
{
  return checkTwo(test, masterCard);
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) 
{
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) 
    {    // Loop once for each EEPROM entry
      readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
      if ( checkTwo( find, storedCard ) ) 
      {   // Check to see if the storedCard read from EEPROM
        // is the same as the find[] ID card passed
        return i;         // The slot number of the card
      }
    }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) 
{
  if ( !findID( a ) ) 
  {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) 
      {   // Loop 4 times
        EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
      }
    successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else 
  {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) 
{
  if ( !findID( a ) ) 
  {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else 
  {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) 
      {         // Loop the card shift times
        EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
      }
    for ( uint8_t k = 0; k < 4; k++ ) 
      {         // Shifting loop
        EEPROM.write( start + j + k, 0);
      }
    successDelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////////////button wipe function///////////////////////////
bool monitorWipeButton(uint32_t interval) 
{
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  
  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) 
    {
      if (digitalRead(wipebutton) != HIGH)
        return false;
    }
  }
  return true;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() 
{
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, LOW);
  for (uint8_t k = 0; k < 5; k++)
  {
     digitalWrite(openLed, HIGH);
     delay(500);
     digitalWrite(openLed, LOW);
     delay(500);
  }
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() 
{
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, LOW);
  for (uint8_t k = 0; k < 5; k++)
  {
     digitalWrite(closedLed, HIGH);
     delay(500);
     digitalWrite(closedLed, LOW);
     delay(500);
  }
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() 
{
  digitalWrite(standbyLed, LOW);
  digitalWrite(openLed, LOW); 
  digitalWrite(closedLed, LOW);
  for (uint8_t k = 0; k <= 5; k++)
  {
     digitalWrite(standbyLed, HIGH);
     delay(500);
     digitalWrite(standbyLed, LOW);
     delay(500);
  }
}

void cycleLeds() 
{
  digitalWrite(standbyLed, HIGH);  // Make sure red LED is off
  digitalWrite(openLed, LOW);   // Make sure green LED is on
  digitalWrite(closedLed, LOW);   // Make sure blue LED is off
  delay(1000);
  digitalWrite(standbyLed, LOW); // Make sure red LED is off
  digitalWrite(openLed, HIGH);  // Make sure green LED is off
  digitalWrite(closedLed, LOW);  // Make sure blue LED is on
  delay(1000);
  digitalWrite(standbyLed, LOW);  // Make sure red LED is on
  digitalWrite(openLed, LOW);  // Make sure green LED is off
  digitalWrite(closedLed, HIGH);   // Make sure blue LED is off
  delay(1000);
}



