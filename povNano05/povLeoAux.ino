void shift_Out_8(povValue* val)
{
  uint8_t i;
  uint8_t mask=128;	
  if(val->frame){
    fr0Low();
  }else{
    fr1Low();
  }
  delayMicroseconds(5);

  for (i = 0; i < 8; i++)  {
    if(!!(val->addr & (mask))){
      dataHigh();
    }
    else{
      dataLow();
    }		
    clkHigh();       
    clkLow(); 
    mask >>= 1;	
  }
  dataLow();
  delayMicroseconds(30);
  if(val->frame){
    fr0High();
  }else{
    fr1High();
  }
}

void shift_Out(povValue* val)
{
  uint8_t i;
  uint8_t mask=128;	
  if(val->frame){
    fr0Low();
  }else{
    fr1Low();
  }

  for (i = 0; i < 8; i++)  {
    if(!!(val->addr & (mask))){
      dataHigh();
    }
    else{
      dataLow();
    }		
    clkHigh();       
    clkLow(); 
    mask >>= 1;	
  }
  mask=128;
  for (i = 0; i < 8; i++)  {
    if(!!(val->value & (mask))){
      dataHigh();
    }
    else{
      dataLow();
    }		
    clkHigh();       
    clkLow(); 
    mask >>= 1;	
  }
  dataLow();

  if(val->frame){
    fr0High();
  }else{
    fr1High();
  }
}


void header(){
  povValue pv;
  for(int val=0;val<18;val++){
    pv.addr=headerData[val][0];
    pv.value=headerData[val][1];
    pv.frame=true;		
    shift_Out(&pv);  
    pv.frame=false;		
    shift_Out(&pv);      
    delayMicroseconds(5);
  }  
  delayMicroseconds(25);
}





