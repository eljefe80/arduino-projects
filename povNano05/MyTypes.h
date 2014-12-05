#ifndef MyTypes_h
#define MyTypes_h

/*
fr0-or C7 : D5
dta-ye D6 : D6
clk-gr B7 : D7
dis-bl B6 : B0
fr1-pu B5 : B1
mot- wh E6 : D4
*/

#define fr0Port 5
#define dtaPort 6
#define clkPort 7
#define disPort 0
#define fr1Port 1
#define motPort 4
#define sensorPin A3

#define fr0High()    (PORTD |= 1<<(fr0Port));       
#define fr0Low()     (PORTD &= ~(1<<(fr0Port)));
#define dataHigh()   (PORTD |= 1<<(dtaPort));
#define dataLow()    (PORTD &= ~(1<<(dtaPort)));
#define clkHigh()    (PORTD |= 1<<(clkPort));       
#define clkLow()     (PORTD &= ~(1<<(clkPort))); 
#define dispHigh()   (PORTB |= 1<<(disPort));       
#define dispLow()    (PORTB &= ~(1<<(disPort)));
#define fr1High()    (PORTB |= 1<<(fr1Port));       
#define fr1Low()     (PORTB &= ~(1<<(fr1Port)));
#define motHigh()    (PORTD |= 1<<(motPort));       
#define motLow()     (PORTD &= ~(1<<(motPort)));


#define wh 0
#define ye 1
#define pu 2 
#define re 3
#define cy 4
#define gr 5
#define bl 6
#define bk 7


#include <WString.h>

typedef struct 
{
  uint8_t addr;
  uint8_t value;
  bool frame;
} povValue;


povValue preHeaderData[1] = { 
  {0xFF,0xFF,true}
};



#endif
