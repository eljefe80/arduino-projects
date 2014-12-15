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


typedef struct zdo_device_annce_t {
uint8_t	transaction;
uint8_t	network_addr_le0;
uint8_t	network_addr_le1;
uint8_t ieee_address_lo_le0;
uint8_t ieee_address_lo_le1;
uint8_t ieee_address_lo_le2;
uint8_t ieee_address_lo_le3;
uint8_t ieee_address_hi_le0;
uint8_t ieee_address_hi_le1;
uint8_t ieee_address_hi_le2;
uint8_t ieee_address_hi_le3;
uint8_t	capability; ///< see ZDO_CAPABILITY_* macros
} zdo_device_annce_t;

XBee xbee = XBee();
// create response and command objects we expect to handle
ZBExpRxResponse ZBRx16 = ZBExpRxResponse();
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
uint8_t rstCmd[] = {'F','R'};
uint8_t dHiCmd[] = {'D','H'};
uint8_t dLoCmd[] = {'D','L'};

uint8_t sHiCmd[] = {'S','H'};
uint8_t sLoCmd[] = {'S','L'};
uint8_t ndCmd[] = {'N','D'};
uint8_t myCmd[] = {'M','Y'};
uint8_t mpCmd[] = {'M','P'};

AtCommandRequest atRequest;

AtCommandResponse atResponse = AtCommandResponse();

//void device_announce() {

// Specify the address of the remote XBee (this is the SH + SL)
//XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000ffff);
//ZBExpRxResponse rx = ZBExpRxResponse();
ZBExpCommand txCmd;
ZBTxRequest tx;
XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);
XBeeAddress64 Coordinator = Broadcast;
uint16_t CoordinatorShortAddress = 0xfffe;
// Create a TX Request
//ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

// Send your request
//xbee.send(zbTx);

//}


/////////////////////////////
//SETUP
void setup(){
  nss.begin(9600);
  Serial.begin(9600);
    xbee.setSerial(Serial);
  pinMode(pirPin, INPUT);
  //pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
  //give the sensor some time to calibrate
  nss.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      nss.print(".");
      delay(1000);
      }
    nss.println(" done");
    nss.println("SENSOR ACTIVE");
    //delay(50);

    atRequest = AtCommandRequest(rstCmd);
    nss.println("resetting firmware");
    uint32_t syncStat = sendAtCommand(rstCmd,true);
    while (syncStat != -2){
        delay(1000);
	syncStat = sendAtCommand(rstCmd,syncStat == -1);
	//nss.print(syncStat); 
	}
    delay(2000);
    atRequest = AtCommandRequest(assocCmd);
    nss.println("Waiting for sync");
    syncStat = sendAtCommand(assocCmd,true);
    while (syncStat != 0){
        delay(1000);
	syncStat = sendAtCommand(assocCmd, syncStat == -1);
	//nss.print(syncStat); 
	}
        atRequest = AtCommandRequest(ndCmd);
    nss.println("Parent Network Address");
    syncStat = sendAtCommand(ndCmd,true);
    while (syncStat != 1){
        delay(1000);
	syncStat = sendAtCommand(ndCmd,syncStat == -1);
	//nss.print(syncStat); 
	}
    atRequest = AtCommandRequest(sHiCmd);
    uint32_t srnHi = sendAtCommand(sHiCmd,true); 
    while ((srnHi == -1) || (srnHi == -2)){
        delay(1000);
	srnHi = sendAtCommand(sHiCmd,srnHi == -1);
	//nss.print(syncStat); 
	}
    atRequest = AtCommandRequest(sLoCmd);
    uint32_t srnLo = sendAtCommand(sLoCmd,true);
    while ((srnLo == -1) || (srnLo == -1)){
        delay(1000);
	srnLo = sendAtCommand(sLoCmd,srnLo == -1);
	//nss.print(syncStat); 
	}
    atRequest = AtCommandRequest(myCmd);
    uint32_t my = sendAtCommand(myCmd,true);
    while ((my == -1) || (my == -1)){
        delay(1000);
	my = sendAtCommand(myCmd,my == -1);
	//nss.print(syncStat); 
	}
    nss.print("Serial Number: ");
    XBeeAddress64 myAddr = XBeeAddress64(srnHi, srnLo);
    nss.print(myAddr.getMsb(),HEX);
    nss.print(myAddr.getLsb(),HEX);
    nss.print(":");
    nss.println(my,HEX);
    nss.println();
    nss.println(F("Sending Device Announce to Coordinator"));
    // {0x22,mySrc, mySrcLong};
/*typedef PACKED_STRUCT zdo_device_annce_t {
uint8_t	transaction;
uint16_t	network_addr_le;
addr64 ieee_address_le;
//uint8_t	capability; ///< see ZDO_CAPABILITY_* macros
} zdo_device_annce_t;
*/
    zdo_device_annce_t rrrPayload;
    rrrPayload.transaction = 0x22;
    rrrPayload.network_addr_le1 = my >> 8;
    rrrPayload.network_addr_le0 = my & 0xff;
    rrrPayload.ieee_address_lo_le3 = srnLo >> 24;
    rrrPayload.ieee_address_lo_le2 = (srnLo & 0xff0000) >> 16;
    rrrPayload.ieee_address_lo_le1 = (srnLo & 0xff00) >> 8;
    rrrPayload.ieee_address_lo_le0 = srnLo & 0xff;
    rrrPayload.ieee_address_hi_le3 = srnHi >> 24;
    rrrPayload.ieee_address_hi_le2 = (srnHi & 0xff0000) >> 16;
    rrrPayload.ieee_address_hi_le1 = (srnHi & 0xff00) >> 8;
    rrrPayload.ieee_address_hi_le0 = srnHi & 0xff;
    rrrPayload.capability = 4;
    for (int i = 0; i < sizeof(rrrPayload); i++){
       nss.print(((uint8_t*)&rrrPayload)[i], HEX);
    }
    nss.println();
    txCmd = ZBExpCommand(Broadcast, //This will be broadcast to all devices
      0xfffe, // addr16
      0, //srcEndpoint
      0, //dstEndpoint
      0x0013, //clusterId
      0x0000, //profileId
      0,    //broadcast radius
      0x00,    //option
      (uint8_t*)&rrrPayload, //payload
      sizeof(rrrPayload),    //payload length
      0x00
      );   // frame ID
    xbee.send(txCmd);   

//7e 00 1b 91 00 0d 6f 00 02 90 98 56 7b f1 00 00
//00 06 clusterId
//00 00 profileId
//02 options
//01 transaction
//fd ff etwork_addr_le
//16 c2 profile_id_le
//00 profile_id_le
//01 num_in_clusters
//f0 00 in_cluster_list[1]
//38 cksum
    
      uint8_t rrrPayload2[] = {0x01, 0xfd, 0xff, 0x16, 0xc2, 0x00, 0x01, 0xf0, 0x00};
         txCmd = ZBExpCommand(Coordinator, //This will be broadcast to all devices
      CoordinatorShortAddress, // addr16
      0, //srcEndpoint
      0, //dstEndpoint
      0x0006, //clusterId
      0x0000, //profileId
      0,    //broadcast radius
      0x00,    //option
      rrrPayload2, //payload
      sizeof(rrrPayload2),    //payload length
      0x01
      );   // frame ID
      
    xbee.send(txCmd);    
 }

////////////////////////////
//LOOP
void loop(){
         AtCommandResponse myResponse;
    ModemStatusResponse modemStatus;
    ZBTxStatusResponse zbTxStatus;
    bool XBeeConnected = false;
    xbee.readPacket();
     if (xbee.getResponse().isAvailable()) {
      // got something
      nss.println("Got a response");
        xbee.getResponse().getZBRxResponse(ZBRx16);
       //for (int i = 0; i < ZBRx16.getDataLength(); i++) { 
       //  nss.print(ZBRx16.getData(i));
       // if ((i+1) % 16 == 0){
       //     nss.println();
       // }
       switch (ZBRx16.getApiId()) {
        case ZB_TX_STATUS_RESPONSE:
         nss.print("TX Status Response: ");
         xbee.getResponse().getZBTxStatusResponse(zbTxStatus);
         nss.println(zbTxStatus.getDeliveryStatus());
         nss.println(zbTxStatus.getTxRetryCount());
         nss.println(zbTxStatus.getDiscoveryStatus());
         nss.println(zbTxStatus.isSuccess());
        case MODEM_STATUS_RESPONSE:
         nss.print("Modem status: ");
         xbee.getResponse().getModemStatusResponse(modemStatus);
         nss.println(modemStatus.getStatus());
        case AT_RESPONSE:
         nss.println("got AT_RESPONSE packet:");
         xbee.getResponse().getAtCommandResponse(myResponse);
         nss.print("\tCommand: ");
         nss.print(myResponse.getCommand()[0],HEX);
         nss.println(myResponse.getCommand()[1],HEX);
         break;
        case ZB_EXPLICIT_RX_RESPONSE:
         if (ZBRx16.getOption() == ZB_PACKET_ACKNOWLEDGED) {
          // the sender got an ACK
          nss.println("packet acknowledged");
         } 
         else {
          nss.println("packet not acknowledged");
         }
         
         nss.print("checksum is ");
         nss.println(ZBRx16.getChecksum(), HEX);

         nss.print("packet length is ");
         nss.println(ZBRx16.getPacketLength(), DEC);

         nss.print("Lsb length is ");
         nss.println(ZBRx16.getLsbLength(), DEC);
         nss.print("Msb length is ");
         nss.println(ZBRx16.getLsbLength(), DEC);
         switch (ZBRx16.getClusterId()){
           case 0x0005:
             nss.println("\tReceived Active Endpoint message");
             break;
           case 0x8006:
             nss.println("\Received Match Descriptor message");
             break;
           case 0x00f6:
             nss.println("\tReceived hardware join message 1");
             break;
           case 0x00f0:
           {
             nss.println("\tReceived hardware join message 1");
             uint8_t rrrPayload[] = {0,0};
             txCmd = ZBExpCommand(Coordinator, //This will be broadcast to all devices
              CoordinatorShortAddress, // addr16
              0, //srcEndpoint
              0, //dstEndpoint
              0x0013, //clusterId
              0x0000, //profileId
              0,    //broadcast radius
              0x00,    //option
              (uint8_t*)&rrrPayload, //payload
              sizeof(rrrPayload),    //payload length
              0x00
             );   // frame ID
             xbee.send(txCmd);  
             XBeeConnected = true;
             break;
           }
         }
         break;
        default: {
         nss.print("Source address: ");
         nss.print(ZBRx16.getRemoteAddress64().getMsb(),HEX); 
         nss.print(ZBRx16.getRemoteAddress64().getLsb(),HEX);
         nss.print(" ApiID: ");
         nss.println(ZBRx16.getApiId());
         break;
        }
       } 
     } else if (xbee.getResponse().isError()) {
      nss.print("oh no!!! error code:");
      nss.println(xbee.getResponse().getErrorCode());
    }
    if (XBeeConnected) {
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
         uint8_t rrrPayload[] = {0xfd,0};
         txCmd = ZBExpCommand(Coordinator, //This will be broadcast to all devices
              CoordinatorShortAddress, // addr16
              0, //srcEndpoint
              0, //dstEndpoint
              0x00f7, //clusterId
              0x0000, //profileId
              0,    //broadcast radius
              0x00,    //option
              (uint8_t*)&rrrPayload, //payload
              sizeof(rrrPayload),    //payload length
              0x00
             );   // frame ID
         xbee.send(txCmd); 
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
           uint8_t rrrPayload[] = {0xfe,0};
           txCmd = ZBExpCommand(Coordinator, //This will be broadcast to all devices
              CoordinatorShortAddress, // addr16
              0, //srcEndpoint
              0, //dstEndpoint
              0x00f7, //clusterId
              0x0000, //profileId
              0,    //broadcast radius
              0x00,    //option
              (uint8_t*)&rrrPayload, //payload
              sizeof(rrrPayload),    //payload length
              0x00
             );   // frame ID
           xbee.send(txCmd); 
           delay(50);
           }
       }
    }
  }


void match_descriptor_request() {
      uint8_t rrrPayload[] = {0x12,0x01};
    tx = ZBTxRequest(Coordinator, //This will be broadcast to all devices
      0xfffe,
      2,    //broadcast radius
      0x81,    //option
      rrrPayload, //payload
      sizeof(rrrPayload),    //payload length
      0x00);   // frame ID
    xbee.send(tx);	
}

uint32_t sendAtCommand(uint8_t* command, bool SendCommand) {
  AtCommandResponse myResponse;
  ModemStatusResponse modemStatus;
  if (SendCommand) {
    nss.print("Sending command to the XBee:");
    nss.print(command[0]);
    nss.println(command[1]);
    // send the command
    xbee.send(atRequest);
  }
  // wait up to 5 seconds for the status response
while (1) {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(myResponse);
      if ((myResponse.getCommand()[0] != command[0]) || 
              (myResponse.getCommand()[1] != command[1]))
              continue;
      if (myResponse.isOk()) {
        nss.print("Command [");
        nss.print(myResponse.getCommand()[0]);
        nss.print(myResponse.getCommand()[1]);
        nss.println("] was successful!");

        if (myResponse.getValueLength() > 0) {
          nss.print("Command value length is ");
          nss.println(myResponse.getValueLength(), DEC);

          nss.print("Command value: ");
          uint32_t ret = 0;
          for (int i = 0; i < myResponse.getValueLength(); i++) {
	    ret = (ret *256) + myResponse.getValue()[i];
            nss.print(myResponse.getValue()[i], HEX);
            nss.print(" ");
          }
          nss.println("");
          nss.print("Command: ");
          nss.print(myResponse.getCommand()[0],HEX);
          nss.print(myResponse.getCommand()[1],HEX);
          nss.print(" command: ");
          nss.print(command[0],HEX);
          nss.println(command[1],HEX);
          if ((command[0] == 'N') && (command[1] == 'D')) {
            uint32_t source_addr = myResponse.getValue()[0]*256 + myResponse.getValue()[1];
            uint32_t source_addr_long_hi = myResponse.getValue()[2];
           for (int i=3; i <6; i++){ 
              source_addr_long_hi = (source_addr_long_hi*256) +myResponse.getValue()[i];
           }
            uint32_t source_addr_long_lo = myResponse.getValue()[6];
           for (int i=7; i <10; i++){ 
              source_addr_long_lo = (source_addr_long_lo*256) +myResponse.getValue()[i];
           }
            Coordinator = 
              XBeeAddress64(source_addr_long_hi, source_addr_long_lo);
              CoordinatorShortAddress = source_addr;
          }
          
          if ((myResponse.getCommand()[0] == command[0]) && 
              (myResponse.getCommand()[1] == command[1]))
       	    return ret;
        } else {
           return 0;
        }
      } 
      else {
        nss.print("Command return error code: ");
        nss.println(myResponse.getStatus(), HEX);
	return myResponse.getStatus();
      }
    } else {
      nss.print("Expected AT response but got ");
        if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
        nss.print("Modem status: ");
         xbee.getResponse().getModemStatusResponse(modemStatus);
         nss.println(modemStatus.getStatus());
        }
      nss.println(xbee.getResponse().getApiId(), HEX);
      return -2;
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
      return xbee.getResponse().getErrorCode();
    } 
    else {
      nss.println("No response from radio");  
      return -1;
    }
  }
}
}
