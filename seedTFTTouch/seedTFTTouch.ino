#include <stdint.h>
#include <TouchScreen.h>
#include <TFT.h>
#include <SD.h>
 
//Measured ADC values for (0,0) and (240-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 240 -1
//TS_MAXY corresponds to ADC value when Y = 320 -1
 
static unsigned int TS_MINX, TS_MAXX, TS_MINY, TS_MAXY;
 
//Touch Screen Co-ordinate mapping register
static unsigned int MapX1, MapX2, MapY1, MapY2;
 
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
 
/* Usage: TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); 
 Where, XP = X plus, YP = Y plus, XM = X minus and YM = Y minus */
//init TouchScreen port pins. This would be reinitialized in setup() based on the hardware detected.
TouchScreen ts = TouchScreen(17, A2, A1, 14, 300); 
 
void setup(void) 
{
  Tft.init();//init TFT
  initTouchScreenParameters(); //initializes Touch Screen parameters based on the detected TFT Touch Schield hardware
 
  //Lines
//  Tft.drawLine(0,0,50,50,RED);//draw a 45degree red line point(0,0) to point(50,50) 
//  Tft.drawVerticalLine(25,0,50,GREEN);//draw a vertical green line point(25,0) to point(25,50)
//  Tft.drawHorizontalLine(0,25,50,BLUE);//draw a horizontal blue line point(0,25) to point(50,25)
 
  //Rectangle
//  Tft.drawRectangle(50,0,80,50,WHITE);//draw a white rectangle, length=80(X-AXIS), width=50(Y-AXIS)
//  Tft.fillRectangle(0,50,50,80,CYAN);//fill a cyan rectangle, length=50(X-AXIS), width=80(Y-AXIS)
 
  //Circle
//  Tft.drawCircle(75,75,25,RED);//draw a red circle, circle centre(75,75) radius=25
//  Tft.fillCircle(150,100,50,GREEN);//fill a green circle, circle centre(150,100) radius=50
 
  //Text
//  Tft.drawChar('S',0,150,2,RED);//draw a char, start from point(0,150) font size 2(16*16)
//  Tft.drawString("Seeed Studio",8,166,2,GREEN);//draw a char, start from point(8,166) font size 2(16*16)
}
 
void loop(void) 
{
  // a point object holds x y and z coordinates
  Point p = ts.getPoint();
 
  p.x = map(p.x, TS_MINX, TS_MAXX, MapX1, MapX2);
  p.y = map(p.y, TS_MINY, TS_MAXY, MapY1, MapY2);
 
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > ts.pressureThreshhold) 
  {
    //p.x;
    //p.y;
    //p.z;
    Tft.fillCircle(p.x,p.y,2,GREEN);
  }
}
 
void initTouchScreenParameters()
{
  //This function initializes Touch Screen parameters based on the detected TFT Touch Schield hardware
 
  if(Tft.IC_CODE == 0x5408) //SPFD5408A TFT driver based Touchscreen hardware detected
  {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ts = TouchScreen(54, A1, A2, 57, 300); //init TouchScreen port pins
#else 
    ts = TouchScreen(14, A1, A2, 17, 300); //init TouchScreen port pins
#endif
    //Touchscreen parameters for this hardware
    TS_MINX = 120;
    TS_MAXX = 910;
    TS_MINY = 120;
    TS_MAXY = 950;
 
    MapX1 = 239;
    MapX2 = 0;
    MapY1 = 0;
    MapY2 = 319;
  }
  else //ST7781R TFT driver based Touchscreen hardware detected
  {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ts = TouchScreen(57, A2, A1, 54, 300); //init TouchScreen port pins
#else 
    ts = TouchScreen(17, A2, A1, 14, 300); //init TouchScreen port pins
#endif 
 
    //Touchscreen parameters for this hardware
    TS_MINX = 140;
    TS_MAXX = 900;
    TS_MINY = 120;
    TS_MAXY = 940;
 
    MapX1 = 239;
    MapX2 = 0;
    MapY1 = 319;
    MapY2 = 0;
  }
}
