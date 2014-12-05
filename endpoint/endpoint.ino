/* 
 * //////////////////////////////////////////////////
 * //making sense of the Parallax PIR sensor's output
 * //////////////////////////////////////////////////
 *
 * Switches a LED according to the state of the sensors output pin.
 * Determines the beginning and end of continuous motion sequences.
 *
 * @author: Kristian Gohlke / krigoo (_) gmail (_) com / http://krx.at
 * @date:   3. September 2006 
 *
 * kr1 (cleft) 2006 
 * released under a creative commons "Attribution-NonCommercial-ShareAlike 2.0" license
 * http://creativecommons.org/licenses/by-nc-sa/2.0/de/
 *
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * (http://www.parallax.com/detail.asp?product_id=555-28027)
 *
 * The sensor's output pin goes to HIGH if motion is present.
 * However, even if motion is present it goes to LOW from time to time, 
 * which might give the impression no motion is present. 
 * This program deals with this issue by ignoring LOW-phases shorter than a given time, 
 * assuming continuous motion is present during these phases.
 *  
 */

#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create response and command objects we expect to handle
ZBExpRxResponse rx = ZBExpRxResponse();
XBeeAddress64 switchLongAddress;
uint16_t switchShortAddress;
uint16_t myFrameId=1;      // for debugging, it's nice to know which messages is being handled

// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 2 to Tx and 3 to Rx of the XBee
// I know this sounds backwards, but remember that output
// from the Arduino is input to the Xbee
#define ssRX 2
#define ssTX 3
SoftwareSerial nss(ssRX, ssTX);


/////////////////////////////
//VARS
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 5; //30;        

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 500;  

boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 4;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;

uint8_t assocCmd[] = {'A','I'};

AtCommandRequest atRequest = AtCommandRequest(assocCmd);

AtCommandResponse atResponse = AtCommandResponse();

//void device_announce() {

// Specify the address of the remote XBee (this is the SH + SL)
//XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000ffff);

// Create a TX Request
//ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

// Send your request
//xbee.send(zbTx);

//}


/////////////////////////////
//SETUP
void setup(){
  nss.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
  //give the sensor some time to calibrate
  nss.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      nss.print(".");
      delay(1000);
      }
    nss.println(" done");
    nss.println("SENSOR ACTIVE");
    delay(50);
    Serial.begin(9600);
    xbee.setSerial(Serial);
    sendAtCommand();
  }

////////////////////////////
//LOOP
void loop(){

     if(digitalRead(pirPin) == HIGH){
       digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
       if(lockLow){  
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;            
         nss.println("---");
         nss.print("motion detected at ");
         nss.print(millis()/1000);
         nss.println(" sec"); 
         delay(50);
         }         
         takeLowTime = true;
       }

     if(digitalRead(pirPin) == LOW){       
//       digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           lockLow = true;                        
           nss.print("motion ended at ");      //output
           nss.print((millis() - pause)/1000);
           nss.println(" sec");
           delay(50);
           }
       }
  }


void sendAtCommand() {
  nss.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        nss.print("Command [");
        nss.print(atResponse.getCommand()[0]);
        nss.print(atResponse.getCommand()[1]);
        nss.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          nss.print("Command value length is ");
          nss.println(atResponse.getValueLength(), DEC);

          nss.print("Command value: ");
          
          for (int i = 0; i < atResponse.getValueLength(); i++) {
            nss.print(atResponse.getValue()[i], HEX);
            nss.print(" ");
          }

          nss.println("");
        }
      } 
      else {
        nss.print("Command return error code: ");
        nss.println(atResponse.getStatus(), HEX);
      }
    } else {
      nss.print("Expected AT response but got ");
      nss.println(xbee.getResponse().getApiId(), HEX);
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
    } 
    else {
      nss.println("No response from radio");  
    }
  }
}
