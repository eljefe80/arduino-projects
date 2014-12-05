#include <XBee.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create response and command objects we expect to handle 
ZBExpRxResponse rx = ZBExpRxResponse();
XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);

XBeeAddress64 switchLongAddress;
uint16_t switchShortAddress;
uint16_t myFrameId=1;
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

int pin5 = 0;

int statusLed = 13;
int errorLed = 13;

void flashLed(int pin, int times, int wait) {

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);`
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}
void setup() {
    Serial.begin(9600);
  xbee.setSerial(Serial);
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  //xbee.begin(9600);
          uint8_t payload1[] = {0,0};
          ZBExpCommand tx = ZBExpCommand(Broadcast,
            0xfffe,
            0,    //src endpoint
            0,    //dest endpoint
            0x0005,    //cluster ID
            0x0000, //profile ID
            0,    //broadcast radius
            0x00,    //option
            payload1, //payload
            sizeof(payload1),    //payload length
            myFrameId++); 
          xbee.send(tx);

  // flash TX indicator
  flashLed(statusLed, 1, 100);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!

    // should be a znet tx status
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        flashLed(statusLed, 5, 50);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        flashLed(errorLed, 3, 500);
      }
    }
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");
    //nss.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(errorLed, 2, 50);
  }

  delay(1000);

          uint8_t payload2[] = {0x00,0x00,0x00,0x00,0x01,02};
          tx = ZBExpCommand(Broadcast,
            0xfffe,
            0,    //src endpoint
            0,    //dest endpoint
            0x8005,    //cluster ID
            0x0000, //profile ID
            0,    //broadcast radius
            0x00,    //option
            payload2, //payload
            sizeof(payload2),    //payload length
            myFrameId++);   // frame ID
          xbee.send(tx);
}

void loop() {
  
}
