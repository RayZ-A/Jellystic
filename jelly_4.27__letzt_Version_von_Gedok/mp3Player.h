#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

bool dfInitialized = false;
int lastSample = 1;
int rxPin = 12;
int txPin = 11;

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/rxPin, /*tx =*/txPin);
#define FPSerial softSerial
#else
#define FPSerial Serial
#endif

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setupMp3()
{
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    
#if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/rxPin, /*tx =*/txPin);
#else
  FPSerial.begin(9600);
#endif

  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  // Use serial to communicate with mp3
  dfInitialized = myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true);
  if (!dfInitialized) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    return;
  }

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  myDFPlayer.volume(25);  // set volume value. From 0 to 30
  myDFPlayer.loop(lastSample);  // loop the first mp3
}


void loopMp3()
{
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}


bool playMp3(int sample)
{
  int newSample = sample+1;
  if (newSample != lastSample)
  {
    myDFPlayer.loop(newSample);
    lastSample = newSample;
  }
}


void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}