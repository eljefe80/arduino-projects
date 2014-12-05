#include <Flash.h>
#include "MyTypes.h"
#include "image01.h"

int sensorValue;
int sensorThreshold;
uint8_t image=0;
uint16_t imageDisplayTime=100;

unsigned long startTime;

void createLine(povValue* line, _FLASH_ARRAY<uint8_t> source){
  uint8_t redMask=B100;
  uint8_t grnMask=B010;
  uint8_t bluMask=B001;
  uint8_t redVal;
  uint8_t grnVal;
  uint8_t bluVal;
  uint8_t colour;
  uint8_t shift;  
  uint8_t address[]={0x80,0x81,0x82,0xC0,0xC1,0xC2};
  uint8_t addrOffset=0;
  uint8_t addressIndx;
  bool frameState=false;
  uint8_t rowStart; 

  for(uint8_t lineNo=0;lineNo<12;lineNo+=3){ 
      
    redVal=0;
    grnVal=0;
    bluVal=0;
    if(lineNo>5){ 
      addrOffset=6;
      frameState=true;  
    }
    addressIndx=lineNo-addrOffset;
    rowStart=(((int)lineNo/3)*8);
  
    for(int rowIndx=rowStart;rowIndx<(rowStart+8);rowIndx++){
      colour=source[rowIndx];
      shift=rowStart+7-rowIndx;
      redVal+=((redMask & colour)&&redMask)<<shift;
      grnVal+=((grnMask & colour)&&grnMask)<<shift;
      bluVal+=((bluMask & colour)&&bluMask)<<shift;
    } 
  
    line[lineNo].value=redVal;
    line[lineNo+1].value=grnVal;
    line[lineNo+2].value=bluVal;
    
    for(uint8_t lineInc=0;lineInc<3;lineInc++){
      line[lineNo+lineInc].addr=address[addressIndx+lineInc];
      line[lineNo+lineInc].frame=frameState;
    }
   
  }

}

void setImage(_FLASH_TABLE<uint8_t> image,uint8_t imageSize){  
  for(int lineIndx=0;lineIndx<imageSize;lineIndx++){
    createLine(columnData[lineIndx],image[lineIndx]);  
  } 
  for(int lineIndx=imageSize;lineIndx<fbColumns;lineIndx++){
    createLine(columnData[lineIndx],image[0]);  
  }  
}

void setup()
{

  DDRB = B00000011;
  DDRD = B11110000;
  
  delay(20);
  PORTB = B00000000;
  PORTD = B00010000;
  
  setImage(bird1,50);

  delay(100);
  sensorThreshold=700;
  fr0High();
  dataHigh();
  clkHigh();
  dispHigh(); 
  fr1High();
  delay(400);  
  fr1Low();
  delayMicroseconds(16);
  fr1High();
  delayMicroseconds(170); 
  //clock and mosi are low
  dataLow();
  clkLow();
  
  delayMicroseconds(100); 
  shift_Out_8(&preHeaderData[0]); 
  header(); 
  delay(100);  
  for(int val=0;val<12;val++){		
    shift_Out(&columnData[0][val]);  
  }
  delayMicroseconds(25);
  for(int column=0;column<180;column++){
    for(int val=0;val<12;val++){		
      shift_Out(&columnData[0][val]);  
      delayMicroseconds(1);
    }
    delayMicroseconds(100);
  }
  delayMicroseconds(400);
  startTime=millis();
}

void loop()
{ 
  while(analogRead(sensorPin)>sensorThreshold){} 
  
  if(millis()-startTime>imageDisplayTime){
    image++;
    switch(image){
       case 0:
         setImage(bird1,23);
       break;
       case 1:
         setImage(bird2,23);
       break;
       case 2:
         setImage(bird3,23);
       break;
       case 3:
         setImage(bird4,23);
       break;
       case 4:
         setImage(bird5,23);
       break;
       case 5:
         setImage(bird6,23);
       break;
       case 6:
         setImage(bird7,23);
       break;
       case 7:
         setImage(bird8,23);
       break;
       
       default:
         image=0;
         setImage(bird1,23);         
       break;
       
    }
    startTime=millis();
  }else{  
    for(int columnPix=11;columnPix>=0;columnPix--){		
        shift_Out(&columnData[0][columnPix]);
    }   
    for(int columnIndx=1;columnIndx<fbColumns;columnIndx++){
      for(int columnPix=11;columnPix>=0;columnPix--){		
        shift_Out(&columnData[columnIndx][columnPix]);  
      }
    } 
    for(int columnPix=11;columnPix>=0;columnPix--){		
      shift_Out(&columnData[0][columnPix]);  
    }
  }
}
