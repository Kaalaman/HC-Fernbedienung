/*-----( Import needed libraries )-----*/
#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"
#include <LiquidCrystal.h>
#include <Wire.h>

/*-----Belegung -----*/
#define SCHUBPIN     A3
#define JOYSTICK_X   A1  // Analog in für X/Y Achse Joystick
#define JOYSTICK_Y   A2
#define JOYSTICK_SW  6  // Button Joystick auf Pin 6
#define POTIPIN      A4 //Pin A4 für Poti(universell) einlesen


/*----Obejkte------*/
//Alle angeschlossenen Objekte werden deklariert.

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);
RF24 hcRadio (48, 53); // "hcRadio" als Name für den Chip zur Übertragung

const byte SLAVE_ADRESS = 21; //Adresse für Angeschlossener Arduino -> Display
const byte MASTER_ADRESS = 42; // Eigene Adresse des Arduino

/*-----Variablen----*/
//alle Globalen-Variablen definieren
byte addresses[][6] = {"1Node"}; // Adresse für einen Übertragungsweg.

byte bBetriebsModi;
byte pipeNr;
//int dInfo = 0;
int iPoti = 0; // Integer für Auslesen des Universal Potis
//bool bConfig = true; //Boolean für Config von Einstellungen

struct infoStruct {
  int BatU1;
  int BatU2;
  int BatU3;
} hcInfo;


struct dataStruct {
  int Spoti;
  int Xposition;          // Werte Position Joystick
  int Yposition;
  bool switchOn;          // Joystick drücken
  int zaehler;
} myData;

//Alle Funktionen müssen deklariert werden
//void Config ();
void WerteEinlesen();
void AusgabeDisplay(byte);
void Config ();
//



void setup()   /****** SETUP: einmal durchlaufen ******/
{
  // Den Serielen Monitor nutzen. Geschwindigkeit: 115200
  Serial.begin(115200);
  Serial.println(F("HC - Fernbedienung"));
  Serial.println(F("Version 1.0 - Alpha"));

  pinMode(JOYSTICK_SW, INPUT_PULLUP);  // Pin "6" als Eingang nutzen

  hcRadio.begin();  // den NRF24L01 CHip starten
  hcRadio.setChannel(108);  // Kanal über normalem WLAN-Kanälen
  hcRadio.setPALevel(RF24_PA_HIGH);
  hcRadio.openReadingPipe(1, addresses[0]);
  hcRadio.openWritingPipe(addresses[1]);
  hcRadio.setDataRate(RF24_1MBPS);

  hcRadio.enableAckPayload();
  hcRadio.enableDynamicPayloads();
  hcRadio.writeAckPayload(1, &hcInfo, sizeof(hcInfo));          // Vorabladen in Puffer, für erste Übertragung

  hcRadio.startListening();

  Wire.begin ();
  delay(1000);

  Config ();
  Serial.print ("EndeSetup");
}//--Ende Setup---


void loop()   /****** LOOP: Dauerschleife ******/
{
  if (hcRadio.available( &pipeNr)) {//wenn daten per funk verfügbar sind

    WerteEinlesen ();//Funtion zum Eingaben Einlesen

    while (hcRadio.available( &pipeNr)) {
      hcRadio.read( &hcInfo, sizeof(hcInfo));

      hcRadio.writeAckPayload(pipeNr, &myData, sizeof(myData));

      Serial.print(F("Rückmeldung: Poti="));
      Serial.print(myData.Spoti);

      Serial.print(F(" X="));
      Serial.print(myData.Xposition);
      Serial.print(F(" Y="));
      Serial.print(myData.Yposition);

      if ( myData.switchOn == 1){
        Serial.print(F(" ON "));}
      else{
        Serial.print(F(" OFF "));}


      Serial.print("INFO: Batterie: ");
      Serial.print(map(hcInfo.BatU1, 0, 1023, 0, 255));

      AusgabeDisplay(1);

      myData.zaehler++;
      Serial.print("\n");

    } // Ende While-Schleife daten ankommen

  } else {

    //Serial.print ("-else-");
  }



}//--(end main loop )---

//--------------------------------------------------------------
//-----------------Eigene Funktionen----------------------------
//--------------------------------------------------------------

void WerteEinlesen () {

  myData.Spoti = analogRead(SCHUBPIN);
  myData.Xposition = analogRead(JOYSTICK_X);
  myData.Yposition = map(analogRead(JOYSTICK_Y), 0, 1023, 1023, 0);
  myData.switchOn  = !digitalRead(JOYSTICK_SW);
  //Serial.print("test");
}

//------------------------------------------------------------------

void AusgabeDisplay (byte vBetriebsModus){



  Wire.beginTransmission (SLAVE_ADRESS);

  Wire.write ( vBetriebsModus);
  Wire.write ( map(myData.Xposition, 0, 1023, 0, 255));
  Wire.write ( map(myData.Yposition, 0, 1023, 0, 255));
  Wire.write ( map(myData.Spoti, 0, 1023, 0, 228));
  Wire.write ( map(hcInfo.BatU1, 0, 1023, 0, 254));
  Wire.write ( map(hcInfo.BatU2, 0, 1023, 0, 254));
  Wire.write ( map(hcInfo.BatU3, 0, 1023, 0, 254));

  Wire.endTransmission();
}

//----------------------------------------------------------

void Config () {

  bool vbConfig = true;

  while (vbConfig == true ) {
    Serial.print("While");
    WerteEinlesen();
    AusgabeDisplay (1);

  }
  Serial.print ("Konfiguration");
}
