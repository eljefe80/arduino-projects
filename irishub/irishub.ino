

/*
This is an examination of Zigbee device communication using an XBee

Specifically using the Lowe's Iris switch.  This device plugs into an outlet
and has a plug on the front for your appliance.  It controls the on/off of the appliance
and measures the power usage as well.  It's a lot like a remote control Kill-a-Watt.

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


void setup() {  
  // start serial
  Serial.begin(9600);
  // and the software serial port
  nss.begin(9600);
  // now that they are started, hook the XBee into 
  // Software Serial
  xbee.setSerial(Serial);
  nss.println("started");
}

void loop() {
    // doing the read without a timer makes it non-blocking, so
    // you can do other stuff in loop() as well.  Things like
    // looking at the console for something to turn the switch on
    // or off (see waaay down below)
    xbee.readPacket();
    // so the read above will set the available up to 
    // work when you check it.
    if (xbee.getResponse().isAvailable()) {
      // got something
      nss.println();
      nss.print("Frame Type is ");
      // Andrew called the XBee frame type ApiId, it's the first byte
      // of the frame specific data in the packet.
      nss.println(xbee.getResponse().getApiId(), HEX);
      //
      // All ZigBee device interaction is handled by the two XBee message type
      // ZB_EXPLICIT_RX_RESPONSE (ZigBee Explicit Rx Indicator Type 91)
      // ZB_EXPLICIT_TX_REQUEST (Explicit Addressing ZigBee Command Frame Type 11)
      // This test code only uses these and the Transmit Status message
      //
      if (xbee.getResponse().getApiId() == ZB_EXPLICIT_RX_RESPONSE) {
        // now that you know it's a Zigbee receive packet
        // fill in the values
        xbee.getResponse().getZBExpRxResponse(rx);
        
        // get the 64 bit address out of the incoming packet so you know 
        // which device it came from
        nss.print("Got a Zigbee explicit packet from: ");
        XBeeAddress64 senderLongAddress = rx.getRemoteAddress64();
        print32Bits(senderLongAddress.getMsb());
        nss.print(" ");
        print32Bits(senderLongAddress.getLsb());
        
        // this is how to get the sender's
        // 16 bit address and show it
        uint16_t senderShortAddress = rx.getRemoteAddress16();
        nss.print(" (");
        print16Bits(senderShortAddress);
        nss.println(")");
        
        // for right now, since I'm only working with one switch
        // save the addresses globally for the entire test module
        switchLongAddress = rx.getRemoteAddress64();
        switchShortAddress = rx.getRemoteAddress16();

        //nss.print("checksum is 0x");
        //nss.println(rx.getChecksum(), HEX);
        
        // this is the frame length
        //nss.print("frame data length is ");
        int frameDataLength = rx.getFrameDataLength();
        //nss.println(frameDataLength, DEC);
        
        uint8_t* frameData = rx.getFrameData();
        // display everything after first 10 bytes
        // this is the Zigbee data after the XBee supplied addresses
        nss.println("Zigbee Specific Data from Device: ");
        for (int i = 10; i < frameDataLength; i++) {
          print8Bits(frameData[i]);
          nss.print(" ");
        }
        nss.println();
        // get the source endpoint
        nss.print("Source Endpoint: ");
        print8Bits(rx.getSrcEndpoint());
        nss.println();
        // byte 1 is the destination endpoint
        nss.print("Destination Endpoint: ");
        print8Bits(rx.getDestEndpoint());
        nss.println();
        // bytes 2 and 3 are the cluster id
        // a cluster id of 0x13 is the device announce message
        nss.print("Cluster ID: ");
        uint16_t clusterId = (rx.getClusterId());
        print16Bits(clusterId);
        nss.println();
        // bytes 4 and 5 are the profile id
        nss.print("Profile ID: ");
        print16Bits(rx.getProfileId());
        nss.println();
        // byte 6 is the receive options
        nss.print("Receive Options: ");
        print8Bits(rx.getRxOptions());
        nss.println();
        nss.print("Length of RF Data: ");
        nss.print(rx.getRFDataLength());
        nss.println();
        nss.print("RF Data Received: ");
        for(int i=0; i < rx.getRFDataLength(); i++){
            print8Bits(rx.getRFData()[i]);
            nss.print(' ');
        }
        nss.println();
        //
        // I have the message and it's from a ZigBee device
        // so I have to deal with things like cluster ID, Profile ID
        // and the other strangely named fields that these devices use
        // for information and control
        //
        if (clusterId == 0x13){
          nss.println("*** Device Announce Message");
          // In the announce message:
          // the next bytes are a 16 bit address and a 64 bit address (10) bytes
          // that are sent 'little endian' which means backwards such
          // that the most significant byte is last.
          // then the capabilities byte of the actual device, but
          // we don't need some of them because the XBee does most of the work 
          // for us.
          //
          // so save the long and short addresses
          switchLongAddress = rx.getRemoteAddress64();
          switchShortAddress = rx.getRemoteAddress16();
          // the data carried by the Device Announce Zigbee messaage is 18 bytes over
          // 2 for src & dest endpoints, 4 for cluster and profile ID, 
          // receive options 1, sequence number 1, short address 2, 
          // long address 8 ... after that is the data specific to 
          // this Zigbee message
          nss.print("Sequence Number: ");
          print8Bits(rx.getRFData()[0]);
          nss.println();
          nss.print("Device Capabilities: ");
          print8Bits(rx.getRFData()[11]);
          nss.println();
        }
        
        if (clusterId == 0x8005){ // Active endpoint response
          nss.println("*** Active Endpoint Response");
          // You should get a transmit responnse packet back from the
          // XBee first, this will tell you the other end received 
          // something.
          // Then, an Active Endpoint Response from the end device
          // which will be Source Endpoint 0, Dest Endpoint 0,
          // Cluster ID 8005, Profile 0
          // it will have a payload, but the format returned by the 
          // Iris switch doesn't match the specifications.
          //
          // Also, I tried responding to this message directly after
          // its receipt, but that didn't work.  When I moved the 
          // response to follow the receipt of the Match Descriptor
          // Request, it started working.  So look below for where I 
          // send the response
        }
        if (clusterId == 0x0006){ // Match descriptor request
          nss.println("*** Match Descriptor Request");
          // This is where I send the Active Endpoint Request 
          // which is endpoint 0x00, profile (0), cluster 0x0005
          uint8_t payload1[] = {0,0};
          ZBExpCommand tx = ZBExpCommand(switchLongAddress,
            switchShortAddress,
            0,    //src endpoint
            0,    //dest endpoint
            0x0005,    //cluster ID
            0x0000, //profile ID
            0,    //broadcast radius
            0x00,    //option
            payload1, //payload
            sizeof(payload1),    //payload length
            myFrameId++);   // frame ID
          xbee.send(tx);
          nss.println();
          nss.print("sent active endpoint request frame ID: ");
          nss.println(myFrameId-1);
          //
          // So, send the next message, Match Descriptor Response,
          // cluster ID 0x8006, profile 0x0000, src and dest endpoints
          // 0x0000; there's also a payload byte
          //
          // {00.02} gave clicks
          uint8_t payload2[] = {0x00,0x00,0x00,0x00,0x01,02};
          tx = ZBExpCommand(switchLongAddress,
            switchShortAddress,
            0,    //src endpoint
            0,    //dest endpoint
            0x8006,    //cluster ID
            0x0000, //profile ID
            0,    //broadcast radius
            0x00,    //option
            payload2, //payload
            sizeof(payload2),    //payload length
            myFrameId++);   // frame ID
          xbee.send(tx);
          nss.println();
          nss.print("sent Match Descriptor Response frame ID: ");
          nss.println(myFrameId-1);
            
          //
          // Odd hardware message #1.  The next two messages are related 
          // to control of the hardware.  The Iris device won't join with
          // the coordinator without both of these messages
          //
          uint8_t payload3[] = {0x11,0x01,0x01};
          tx = ZBExpCommand(switchLongAddress,
            switchShortAddress,
            0,    //src endpoint
            2,    //dest endpoint
            0x00f6,    //cluster ID
            0xc216, //profile ID
            0,    //broadcast radius
            0x00,    //option
            payload3, //payload
            sizeof(payload3),    //payload length
            myFrameId++);   // frame ID
            xbee.send(tx);
            nss.println();
            nss.print("sent funny hardware message #1 frame ID: ");
            nss.println(myFrameId-1);
            //
            // Odd hardware message #2
            //
            uint8_t payload4[] = {0x19,0x01,0xfa,0x00,0x01};
            tx = ZBExpCommand(switchLongAddress,
              switchShortAddress,
              0,    //src endpoint
              2,    //dest endpoint
              0x00f0,    //cluster ID
              0xc216, //profile ID
              0,    //broadcast radius
              0x00,    //option
              payload4, //payload
              sizeof(payload4),    //payload length
              myFrameId++);   // frame ID
              xbee.send(tx);
              nss.println();
              nss.print("sent funny hardware message #2 frame ID: ");
              nss.println(myFrameId-1);
            
        }
        else if (clusterId == 0xf6){
          // This is something specific to the Endpoint devices
          // and I haven't been able to find documentation on it 
          // anywhere
          nss.println("*** Cluster ID 0xf6");
        }
        if (clusterId == 0x00ef){
          //
          // This is a power report, there are two kinds, instant and summary
          //
          nss.print("*** Power Data, ");
          // The first byte is what Digi calls 'Frame Control'
          // The second is 'Transaction Sequence Number'
          // The third is 'Command ID'
          if (rx.getRFData()[2] == 0x81){
            // this is the place where instant power is sent
            // but it's sent 'little endian' meaning backwards
            int power = rx.getRFData()[3] + (rx.getRFData()[4] << 8);
            nss.print("Instantaneous Power is: ");
            nss.println(power);
          }
          else if (rx.getRFData()[2] == 0x82){
            unsigned long minuteStat = (uint32_t)rx.getRFData()[3] + 
              ((uint32_t)rx.getRFData()[4] << 8) + 
              ((uint32_t)rx.getRFData()[5] << 16) + 
              ((uint32_t)rx.getRFData()[6] << 24);
            unsigned long uptime = (uint32_t)rx.getRFData()[7] + 
              ((uint32_t)rx.getRFData()[8] << 8) + 
              ((uint32_t)rx.getRFData()[9] << 16) + 
              ((uint32_t)rx.getRFData()[10] << 24);
            int resetInd = rx.getRFData()[11];
            nss.print("Minute Stat: ");
            nss.print(minuteStat);
            nss.print(" watt seconds, Uptime: ");
            nss.print(uptime);
            nss.print(" seconds, Reset Ind: ");
            nss.println(resetInd);
          }
        }
        if (clusterId == 0x00ee){
          //
          // This is where the current status of the switch is reported
          //
          // If the 'cluster command' is 80, then it's a report, there
          // are other cluster commands, but they are controls to change
          // the switch.  I'm only checking the low order bit of the first
          // byte; I don't know what the other bits are yet.
          if (rx.getRFData()[2] == 0x80){
            if (rx.getRFData()[3] & 0x01)
              nss.println("Light switched on");
            else
              nss.println("Light switched off");
          }
        }
      }
      else if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
        ZBTxStatusResponse txStatus;
        xbee.getResponse().getZBTxStatusResponse(txStatus);
        nss.print("Status Response: ");
        nss.println(txStatus.getDeliveryStatus(), HEX);
        nss.print("To Frame ID: ");
        nss.println(txStatus.getFrameId());
      }
      else {
        nss.print("Got frame type: ");
        nss.println(xbee.getResponse().getApiId(), HEX);
      }
    }
    else if (xbee.getResponse().isError()) {
      // some kind of error happened, I put the stars in so
      // it could easily be found
      nss.print("************************************* error code:");
      nss.println(xbee.getResponse().getErrorCode(),DEC);
    }
    else {
      // I hate else statements that don't have some kind
      // ending.  This is where you handle other things
    }
    if (nss.available() > 0) {
      char incomingByte;
      
      incomingByte = nss.read();
      nss.println(atoi(&incomingByte), DEC);
      if (atoi(&incomingByte) == 0){
        // turn the light off
        lightSet(0);
      }
      else if (atoi(&incomingByte) == 1){
        // turn the light on
        lightSet(1);
      }
    }
}

void lightSet(int val){
  uint8_t payload1[] = {0x11,0x00,0x01,03};
  uint8_t payload2[] = {0x11,0x00,0x02,0x00,0x01};
  
  if (val==0){
    nss.println("Light Off");
  }
  else {
    nss.println("Light On");
    payload2[3] = 0x01;
  }
  ZBExpCommand tx = ZBExpCommand(switchLongAddress,
    switchShortAddress,
    0,    //src endpoint
    2,    //dest endpoint
    0x00ee,    //cluster ID
    0xc216, //profile ID
    0,    //broadcast radius
    0x00,    //option
    payload1, //payload
    sizeof(payload1),    //payload length
    myFrameId++);   // frame ID
    
    xbee.send(tx);
    nss.println();
    nss.print("sent switch off 1 frame ID: ");
    nss.println(myFrameId-1);
  
  tx = ZBExpCommand(switchLongAddress,
    switchShortAddress,
    0,    //src endpoint
    2,    //dest endpoint
    0x00ee,    //cluster ID
    0xc216, //profile ID
    0,    //broadcast radius
    0x00,    //option
    payload2, //payload
    sizeof(payload2),    //payload length
    myFrameId++);   // frame ID
    
    xbee.send(tx);
    nss.println();
    nss.print("sent switch off 2 frame ID: ");
    nss.println(myFrameId-1);
  
}

// these routines are just to print the data with
// leading zeros and allow formatting such that it 
// will be easy to read.
void print32Bits(uint32_t dw){
  print16Bits(dw >> 16);
  print16Bits(dw & 0xFFFF);
}

void print16Bits(uint16_t w){
  print8Bits(w >> 8);
  print8Bits(w & 0x00FF);
}
  
void print8Bits(byte c){
  uint8_t nibble = (c >> 4);
  if (nibble <= 9)
    nss.write(nibble + 0x30);
  else
    nss.write(nibble + 0x37);
        
  nibble = (uint8_t) (c & 0x0F);
  if (nibble <= 9)
    nss.write(nibble + 0x30);
  else
    nss.write(nibble + 0x37);
}
