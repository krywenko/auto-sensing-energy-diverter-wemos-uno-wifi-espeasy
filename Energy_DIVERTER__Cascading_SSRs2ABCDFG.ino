#define FILTERSETTLETIME 5000 //  Time (ms) to allow the filters to settle before sending data  
#include "EmonLib.h"
EnergyMonitor ct1,ct2,ct3,ct4;   // Create  instances for each CT channel

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <PWM.h>
#include <EEPROM.h>
int address1 = 0;
int address2 = 2;
int address3 = 4;
int address4 = 6;

//###### Adjustable settings ####### 
const int relay1=7;  //relay pins
const int relay2=8;
const int relay3=12;
const int relay4=13; 
const int uDIS = 100 ;// what pwm pulse on the third SSR to be reached before enabling  relays 1-254
const int DIS = 10 ; // what pwm pulse on the first SSR to be reached before disabling  relays 1-254
const int RD =1 ;  // if using PWM.h and wish to display if relays are activated on LED 4  1= enable 0= disable

int upperRANGE = 15; // the range in which it will not search for better output 
int lowerRANGE = -15;

int PWM =1 ;  //1=enables PWM.h  smother output 0=disable uses standard adruino PWM libary
int FRAC =4 ;  // fraction of grid Hertz ie  60hz-> 2=1/2 (30hz pwm)   4 =1/4 (15 hz pwm)

const int CT1 = 0;          //  divert sensor - Set to 0 to disable if using  optional  diaplay ( wind)
const int CT2 = 1;         // Inverter sensor - Set to 0 to disable 
const int CT3 = 1;        //grid sensor 
const int CT4 = 1;       // windgen  sensor - Set to 0 to disable  disable if using diverter display
const int CT5 = 0;       //  if not using LCD screen this is the  4th CT
const int LCD = 0;             // 1=enable 0=disable
float element = 3000; //wattage of  element  for diversion -  make bigger  then then what you have to decrease  buuble search sensitivity
const int SSR4 =1;          // 1=  4 ssr and disables static,  0=  3 SSR & 1 static ( disable PWM.h)
const int ios = 3;          /// Number of SSR to control 4 MAX if PWM.h diasble otherwise 3
const int pulse = 11;       // pin for pulse  disable if you cascade on 4 ssr  pin 11 does not work if PWM.h enabled 
const int pulse1 = 9;
const int pulse2 = 10;
const int pulse3 = 3;
const int pulse4 = 11;     //enable pulse 4 if you wish 4 cassacding ssr
float DRIFT =1.015 ;      // if you wish to adust hz output 

const int digitalPin = 6;  // digital input pin from esp  
const int digitalPin1 = 5;    // pin for led display  showing overproduction 
const int type = 0;         // 0= casdading -  1 = equal for diverting 
const int ssr=0;            // 0= zerocrossing 1 = phase angle  currently only supports one ssr 
const int AVG=5;

 //#### Non - adjustable
 
 int sw1=0;
 int sw2=0;
 int sw3=0;
 int sw4=0;
 
 int _INPUT =0; 
 int r1=0;
 int power1=0;
 int power2=0;
 int power3=0;
 int power4=0;
 float powerFactor;
 float volt=0;
 int avg_255 =0;
 int avg_ios=0;
 int SEND =0;
 int count =0;
int count2 =0;
int count3=0;
int cnt0 =0;
int cnt1 =0;
int cnt2 =0;
int cnt3 =0;
int cnt4 =0;
int  FREQ;
float FREQ_F = 0;
const int aIn = 0;
int positive;
unsigned long period;
unsigned long mark;
 
float grid = 0;     //grid   usage
float stepa = 0;   // 
float stepb = 1;
float stepc = 1;
float prestep =1;
float step1 = 0;   // 
float step2 = 1;
float step3 = 1;
float prestep1 =1;
float curinvt = 1; //percentage of power uage comparison over or below grid usage 
float curelem =1;
float kw = 0;
int curgrid = 0;       // current  PMW step
int curgrid2 = 0;     //current triac step
float invert =100;
float wind = 100;
float diverter =100;
float per = 0;
int stat ;
int stepbu;
float stepa4 = 0;   
float stepb4 = 1;
float stepc4 = 1;
float prestep4 =0; 
int stepbu4;
int stat4 ;
float curelem4 =1;
int curgrid4 = 0;
int sV;
int full;
int DIVERT = 0;
String value;
int percent = 0;
float TMP;
float DIVS;

typedef struct { int power1, power2, power3, power4, Vrms;} PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;                                                       
 
boolean settled = false;

void setup() 
{

 //############### pwm pulse freq  for  standard pwm  #####################
 TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz 9 10
 //TCCR0B = TCCR0B & B11111000 | B00000101; // for PWM frequency of 61.04 Hz 5 6
 TCCR2B = TCCR2B & B11111000 | B00000111; // for PWM frequency of 30.64 Hz 3 11
 //#########################
 if (PWM ==1){
   InitTimersSafe();  
 }  
  Serial.begin(115200);
  
sw1=  EEPROM.read(address1);
if ( sw1 ==3){ Serial.println("sw1 set to auto");}
sw2=  EEPROM.read(address2);
if ( sw2 ==3){ Serial.println("sw2 set to auto");}
sw3 = EEPROM.read(address3);
if ( sw3 ==3){ Serial.println("sw3 set to auto");}
sw4 = EEPROM.read(address4);
if ( sw4 ==3){ Serial.println("sw4 set to auto");}  
 //##############LCD##################################
  if (LCD==1) {
   lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Power Diverter");
  lcd.setCursor(2,1);
  lcd.print("Stephen krywenko!");
  }

//################ detect Hz ############### 

  boolean st=false;                                  //an indicator to exit the while loop

  unsigned long start = millis(); 
  
  while(st==false) {
  
 while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
  //Serial.println("zero crossing");
  mark = micros();//zero crossing from negative to positive found
  while((millis()-start)<200 && analogRead(aIn)>512 );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
  period = micros()-mark;
  FREQ = ( 1000000 / period);
Serial.println(FREQ);
//Serial.println(analogRead(aIn));
 st = true;
 
 if (period < 5000){
  
  if (count ==0){
  Serial.println("plug in AC adaptor");
       if (LCD==1) {
          lcd.backlight();
          lcd.clear();  
          lcd.setCursor(0,2);
          lcd.print("Check AC Adaptor");
     }
  
  count++;
  
  }
  st = false;
  delay(10000);
 start = millis();
  
 }
  } 
  pinMode(pulse, OUTPUT); 
  if (PWM == 0){
  pinMode(pulse1, OUTPUT); 
  pinMode(pulse2, OUTPUT);
  analogWrite(pulse1, 0 );
  analogWrite(pulse2, 0 ); 
  }
  pinMode(pulse3, OUTPUT); 
  pinMode(pulse4, OUTPUT); 
  analogWrite(pulse3, 0 );
  analogWrite(pulse4, 0 );  //Enable  if you wish to cascade on  4 ssr /disable  pulse other below
pinMode(relay1, OUTPUT);pinMode(relay2, OUTPUT);pinMode(relay3, OUTPUT);pinMode(relay4, OUTPUT);
digitalWrite(relay1, LOW);digitalWrite(relay2, LOW);digitalWrite(relay3, LOW);digitalWrite(relay4, LOW);
 pinMode(digitalPin, INPUT);
pinMode(digitalPin1, INPUT);

   DIVS= 1 ;           // pwm step
  // DIVS= ios*2.55 ;  // percentage of usable steps

//###################### emontx settings  #######################

  if (CT1) ct1.current(1, 60.600);  // Setup emonTX CT channel (ADC input, calibration)
  if (CT2) ct2.current(2, 60.606);  // Calibration factor = CT ratio / burden resistance
  if (CT3) ct3.current(3, 60.606);  // emonTx Shield Calibration factor = (100A / 0.05A) / 33 Ohms
  if (CT4) ct1.current(1, 60.600);  // the same as the 1st CT 
  if (CT5) ct4.current(4, 60.600);  // the 4th CT
  
  if (CT1) ct1.voltage(0, 146.54, 1.7);   // ct.voltageTX(ADC input, calibration, phase_shift) - make sure to select correct calibration for AC-AC adapter  http://openenergymonitor.org/emon/modules/emontx/firmware/calibration. Default set for Ideal Power adapter                                         
  if (CT2) ct2.voltage(0, 146.54, 1.7);   // 268.97 for the UK adapter, 260 for the Euro and 130 for the US.
  if (CT3) ct3.voltage(0, 146.54, 1.7);
  if (CT4) ct1.voltage(0, 146.54, 1.7);   // the same as the 1st CT
  if (CT5) ct4.voltage(0, 146.54, 1.7);   // the 4th CT


 FREQ = FREQ/FRAC;
  count=0;    
  if (PWM==1){
    SetPinFrequency(pulse1, FREQ); 
    SetPinFrequency(pulse2, FREQ);                                                                               
  }
}

//####### Grid Hertz detection ##################

 void Grid_Hz(){
  
  boolean DONE = false; 
 
 while(DONE==false)
 {

period = 0;
unsigned long start = millis(); 
 while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
 while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
  //Serial.println("zero crossing");
  mark = micros();//zero crossing from negative to positive found
  while((millis()-start)<200 && analogRead(aIn)>512 );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
  period = (micros()-mark);
  DONE = true;

  if (period < 7000){
    if (count2 ==0){
    Serial.println(period);
     Serial.println("Check  AC adaptor");
     count2++;     
     }
          if (LCD==1) {
          lcd.backlight();
          lcd.clear();  
          lcd.setCursor(0,2);
          lcd.print("Check AC Adaptor");
     }
         count=0;
         FREQ_F=0;
         //##### disable pwm  safty measure #######
         pwmWrite(pulse1, 0);
         pwmWrite(pulse2, 0);
         analogWrite(pulse3, 0);
         analogWrite(pulse4, 0);
  //       analogWrite(invstatus, 0);
         digitalWrite(relay1, LOW);digitalWrite(relay2, LOW);digitalWrite(relay3, LOW);digitalWrite(relay4, LOW);
         r1=0;
        // analogWrite(invstatus2, 0);
         analogWrite(pulse, 0);
         delay(1000);
         DONE = false;  
         start = millis();      
  }else
  {
    count2=0;
  FREQ = ( 1000000 / period);
  FREQ_F = FREQ_F+ FREQ;
  count++;

  
  if (count ==10){ 
  // Serial.print("Frequency =  "); Serial.println(FREQ_F/10);
   Serial.print("TaskValueSet,2,3,"); Serial.println((FREQ_F/10)*DRIFT); //Frequency
    count=0;
    FREQ_F=0;
  }
 }}}

//############## PWM.h  setting pulse if used #########
 
void settingPWM( int _PIN, int _PWM)
{ 


  while(analogRead(aIn)>512);//wait for positive half period to expire
  while(analogRead(aIn)<512);//wait for negative half period to expire
  delayMicroseconds(period/2);

   pwmWrite(_PIN,_PWM ); 

   /*
      while(analogRead(aIn)>512);//wait for positive half period to expire
  while(analogRead(aIn)<512);//wait for negative half period to expire
   //setting the duty to 50% with the highest possible resolution that 
   //can be applied to the timer (up to 16 bit). 1/2 of 65536 is 32768.
   pwmWriteHR(led, 32768);
   //Serial.println("High Resolution PWM");
   delay(1000);*/

}

//###############

void SWITCH(int Inp){
  if (Inp > 340 && Inp < 370){Serial.println("TaskValueSet,4,1,3"); sw1=3; EEPROM.update(address1, sw1 );}
  if (Inp > 310 && Inp < 340 ){Serial.println("TaskValueSet,4,1,0");digitalWrite(relay1, LOW); EEPROM.update(address1, 10 ); }
  if (Inp > 280 && Inp < 310 ){Serial.println("TaskValueSet,4,1,1");digitalWrite(relay1, HIGH); EEPROM.update(address1, 10 ); }
  
  if (Inp > 250 && Inp < 280 ){ Serial.println("TaskValueSet,4,2,3");sw2=3; EEPROM.update(address2, sw2 );}
  if (Inp > 220 && Inp < 250){Serial.println("TaskValueSet,4,2,0");digitalWrite(relay2, LOW);EEPROM.update(address2, 10 ); }
  if (Inp > 190 && Inp < 220 ){Serial.println("TaskValueSet,4,2,1");digitalWrite(relay2, HIGH);  EEPROM.update(address2, 10 ); }
  
  if (Inp > 160 && Inp < 190 ){Serial.println("TaskValueSet,4,3,3");sw3=3; EEPROM.update(address3, sw3 );}
  if (Inp > 130 && Inp < 160 ){Serial.println("TaskValueSet,4,3,0");digitalWrite(relay3, LOW); EEPROM.update(address3, 10 );}
  if (Inp > 100 && Inp < 130 ){Serial.println("TaskValueSet,4,3,1");digitalWrite(relay3, HIGH); EEPROM.update(address3, 10 ); }
  
  if (Inp > 70 && Inp < 100 ){Serial.println("TaskValueSet,4,4,3"); sw4=3; EEPROM.update(address4, sw4 );}
  if (Inp > 40 && Inp < 70 ){Serial.println("TaskValueSet,4,4,0");digitalWrite(relay4, LOW); EEPROM.update(address4, 10 );}
  if (Inp > 10 && Inp < 40 ){Serial.println("TaskValueSet,4,4,1");digitalWrite(relay4, HIGH);EEPROM.update(address4, 10 );} 
  
}

void loop() 
{
   _INPUT = pulseIn(digitalPin, HIGH, 4200);
if (_INPUT == 0 && digitalRead(digitalPin) == 1) {
    _INPUT = 900;
}
if (_INPUT == 0 && digitalRead(digitalPin) == 0) {
    _INPUT = 0;
}
_INPUT= map(_INPUT, 0,900 , 0, 255);
//######################
  int _INPUTa = pulseIn(digitalPin1, HIGH, 4200);
if (_INPUTa == 0 && digitalRead(digitalPin1) == 1) {
    _INPUTa = 900;
}
if (_INPUTa == 0 && digitalRead(digitalPin1) == 0) {
    _INPUTa = 0;
}
//_INPUTa= map(_INPUTa, 0,900 , 0, 255);
SWITCH(_INPUTa);
//######################

  if ( RD == 1){ if ( r1 >0) {digitalWrite( pulse4, HIGH);}else{ digitalWrite( pulse4, LOW);}}
 if (SEND <50) SEND++;  
 count3++;
 
  if (CT1) {
   
    ct1.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
    emontx.power1 = ct1.realPower;
    diverter =  emontx.power1;
    if (SEND > 21){ power1= (power1+emontx.power1);cnt0++;}
    if (cnt0 >= AVG){
      power1=(power1/AVG);
   Serial.print("TaskValueSet,1,3,"); Serial.println(power1);
    power1=0;
    cnt0=0;
    }                                 
  }
  
  emontx.Vrms = ct1.Vrms*100;                                            // AC Mains rms voltage 
  
  if (CT2) {
    
    ct2.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
    emontx.power2 = ct2.realPower;
    invert = emontx.power2;
    if (SEND > 22) {power2=(power2+emontx.power2); cnt1++;}
    
    if (cnt1 >= AVG){
    power2=(power2/AVG);
    Serial.print("TaskValueSet,1,2,"); Serial.println(power2); 
    power2=0;
    cnt1=0 ;
    }
  } 

  if (CT3) {
    ct3.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
    emontx.power3 = ct3.realPower;
    powerFactor     = ct3.powerFactor; 
    grid = emontx.power3; 
    if (SEND >23) {power3=(power3+emontx.power3);cnt2++;}
    if (cnt2 >= AVG){
   power3=(power3/AVG);
    Serial.print("TaskValueSet,1,1,"); Serial.println(power3);
   // Serial.print("TaskValueSet,2,4,"); Serial.println(powerFactor);
    power3=0;
    cnt2=0;
    }
  } 
  
   if (CT4) {
     ct1.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
    emontx.power1 = ct1.realPower;
    wind = emontx.power1; 
    if (SEND > 21 ){ power1=(power1+emontx.power1);cnt3++;}
    if (cnt3 >= AVG){
    power1=(power1/AVG);
    Serial.print("TaskValueSet,1,3,"); Serial.println(power1);
    power1=0;
    cnt3=0;
    }

  }

    if (CT5) {
   
    ct4.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
    emontx.power4 = ct4.realPower;
    
    if (SEND > 23){ power4= (power4+emontx.power4);cnt4++;}
    if (cnt4 >= AVG){
      power4=(power4/AVG);
   Serial.print("TaskValueSet,2,4,"); Serial.println(power4);
    power4=0;
    cnt4=0;
    }                                 
  }
  volt= (volt+ct1.Vrms);

   
 //######################## Start if bubble Search ###########################
 
  if (invert <0){       // for capture ac adaptor errors is it display consant zero on inverter display -- ct or ac adaptor need to be reversed
    invert = 0;
  }
  if (wind <0){       // for capture ac adaptor errors is it display consant zero on inverter display -- ct or ac adaptor need to be reversed
    wind = 0;
  }
  
/////############### old code left in for later modification #################
if (grid != 0 ) {  
if (invert >=0) {

  step1 = ( grid / invert);
  prestep1 = (step2);

step2 = (prestep1 + step1);
  if (step2 > 1) {
    step2 =1;
  }
  if (step2 < 0) {
    step2 = 0;
  }
  curinvt = (0  + step2);
  curgrid2 = ( 254 * curinvt  );
  curgrid2 =(254-curgrid2);  //inverts the value of curgrid if need be 
}
}
//#############################################################
//################# Cascading bubble search ###################
if (CT3){
if ( (grid < lowerRANGE) || (grid > upperRANGE)){   
//if (grid !=0) {
  //curgrid = 0;
  stepc = (grid / element); 
  prestep = (stepb);

stepb = (prestep + stepc);
  if (stepb > 0) {
    stepb =0;
  }
  if (stepb < (0-ios)) {
    stepb = (0-ios);
  }
  curelem = (0  + stepb);
stepbu=curelem;
curelem = (curelem - stepbu);
  curgrid = ( 254 * curelem  );
  curgrid =(0-curgrid);  //inverts the value of curgrid if need be 
}

//############ static bubble search ###############################
if ( (grid < lowerRANGE) || (grid > upperRANGE)){ 
//if (grid !=0) { 
  //curgrid = 0;
  stepc4 = (grid / element); 
  prestep4 = (stepb4);

stepb4 = (prestep4 + stepc4);
  if (stepb4 > 1) {
    stepb4 =1;
  }
  if (stepb4 < 0) {
    stepb4 = 0;
  }
  curelem4 = (0  + stepb4);
  curgrid4 = ( 255 * curelem4  );
  curgrid4 =(255-curgrid4);  //inverts the value of curgrid if need be 
}

}
//##################  determines  location of  cascading SSR ############
int statc ;
int ivar;
int statb ;

stat = (0-stepbu);
if (curgrid==256){curgrid=0;}

if (stat > (ios-1)) {stat=(ios-1);curgrid=255;full=1;}
if (stat ==0) {ivar = 1;}
else  {ivar = 0;}

//################### end of bubble search ######################


//################### Pusle  for triac or ssr ###################


if (ssr ==0){

  boolean st=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();                    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st==false)                                   //the while loop...
  {
    sV = analogRead(0);                              //using the voltage waveform
    if ((sV < (1024*0.55)) && (sV > (1024*0.45))) st=true;  //check its within range
    if ((millis()-start)>2000) st = true;
  }

  DIVERT = curgrid;
     //DIVERT=map(DIVERT,0,255,0,120);                    //delay before pulse  
    // DIVERT=map(DIVERT,0,120,0,255);  

//##############Static Pulse###############//
if (SSR4 ==0){
analogWrite(pulse,curgrid4);  // single pulse signal for SSR off arduino board // disable if you want 4 cascading ssr
avg_255=avg_255+curgrid4;
if (count3 >= AVG){
avg_255=avg_255/AVG;
Serial.print("TaskValueSet,2,2,"); Serial.println(avg_255); //curgrid4
}
}
//############################//
//########## Cascading Pulse ####### 
Grid_Hz();

  if ( type == 0){
   // Serial.println(" solar Diversion - Cascading");
if (stat != statb) {
  statc=(stat+1);
  statb=stat;
  for(int i=ivar;i < stat; i++){
   
if ( i == 0){
  if (PWM ==0 ){
  analogWrite(pulse1, 255 );
  }else{
    settingPWM(pulse1, 255);
  }
}
if ( i == 1){
  if (PWM ==0 ){
  analogWrite(pulse2, 255 );
  }else{
    settingPWM(pulse2, 255);
  }  
  analogWrite(pulse2, 255 );
}
if ( i == 2){
  analogWrite(pulse3, 255 );
}
if ( i == 3){                   //enable for 4th ssr
  analogWrite(pulse4, 255 );
}
  }
   
  for(int i=statc;i <ios; i++){
if ( i == 0){
  if (PWM ==0 ){
  analogWrite(pulse1, 0 );
   if (sw1 ==3){ digitalWrite(relay1, LOW);}
   if (sw2 ==3){digitalWrite(relay2, LOW);}
   if (sw3 ==3){digitalWrite(relay3, LOW);}
   if (sw4 ==3){digitalWrite(relay4, LOW);}
         r1=0;
      /*  if (r1 ==1){ digitalWrite(relay1, LOW); r1=0; }
        if (r1==2){ digitalWrite(relay2, LOW); r1=1;}
        if (r1 ==3){ digitalWrite(relay3, LOW); r1=2;}
        if (r1 ==4){ digitalWrite(relay4, LOW); r1=3;} */
  }else{
    settingPWM(pulse1, 0);
        // digitalWrite(relay1, LOW);digitalWrite(relay2, LOW);digitalWrite(relay3, LOW);digitalWrite(relay4, LOW);
    if (sw1 ==3){ digitalWrite(relay1, LOW);}
   if (sw2 ==3){digitalWrite(relay2, LOW);}
   if (sw3 ==3){digitalWrite(relay3, LOW);}
   if (sw4 ==3){digitalWrite(relay4, LOW);}
         r1=0;
      /*
       if (r1 ==1){ digitalWrite(relay1, LOW); r1=0; }
       if (r1==2){ digitalWrite(relay2, LOW); r1=1;}
       if (r1 ==3){ digitalWrite(relay3, LOW); r1=2;}
       if (r1 ==4){ digitalWrite(relay4, LOW); r1=3;}*/
  }
}
if ( i == 1){
  if (PWM ==0 ){
  analogWrite(pulse2, 0 );
  }else{
    settingPWM(pulse2, 0);
  }
}
if ( i == 2){
  analogWrite(pulse3, 0 );
}
 if ( i == 3){                  //enable for 4th ssr
  analogWrite(pulse4, 0 );
}

     }
     }


if ( stat == 0){
   while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
  //Serial.println("zero crossing");
  delayMicroseconds(period/2);
 if (PWM ==0 ){ 
   if (_INPUT >5 ) { DIVERT = _INPUT ;}   
  analogWrite(pulse1, DIVERT );
  analogWrite(pulse2, 0 );
 }else{
  if (_INPUT >5 ) { DIVERT = _INPUT ;}
  settingPWM(pulse1, DIVERT);
  settingPWM(pulse2, 0);
 }
  analogWrite(pulse3, 0 );
  if (SSR4 == 1){
  analogWrite(pulse4, 0 ); //enable for 4th ssr
  if ( DIVERT <= DIS){
  if ( count3 >= AVG){
  if (sw1 ==3){ if (r1 ==1){ digitalWrite(relay1, LOW); r1=0;} } 
  if (sw2 ==3){ if (r1==2){ digitalWrite(relay2, LOW); r1=1;}}
  if (sw3 ==3){ if (r1 ==3){ digitalWrite(relay3, LOW); r1=2;}}
  if (sw4 ==3){ if (r1 ==4){ digitalWrite(relay4, LOW); r1=3;}}
  }
}
if (ios ==1){  
if ( DIVERT >= uDIS){  
  if ( count3 >= AVG){
 if (sw1 ==3){ if (r1 ==3){ digitalWrite(relay4, HIGH); r1=4;}}
 if (sw2 ==3){ if (r1 ==2){ digitalWrite(relay3, HIGH); r1=3;}}
 if (sw3 ==3){ if (r1==1){ digitalWrite(relay2, HIGH); r1=2; }}
 if (sw4 ==3){ if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}}
  }}}
  }
  percent = ((DIVERT)/DIVS);
  Serial.print("TaskValueSet,2,1,"); Serial.println(percent);
if ( DIVERT <= DIS){
  if ( count3 >= AVG){
  if (sw1 ==3){ if (r1 ==1){ digitalWrite(relay1, LOW); r1=0;} } 
  if (sw2 ==3){ if (r1==2){ digitalWrite(relay2, LOW); r1=1;}}
  if (sw3 ==3){ if (r1 ==3){ digitalWrite(relay3, LOW); r1=2;}}
  if (sw4 ==3){ if (r1 ==4){ digitalWrite(relay4, LOW); r1=3;}}
  }
}
if (ios ==1){  
if ( DIVERT >= uDIS){  
  if ( count3 >= AVG){
 if (sw1 ==3){ if (r1 ==3){ digitalWrite(relay4, HIGH); r1=4;}}
 if (sw2 ==3){ if (r1 ==2){ digitalWrite(relay3, HIGH); r1=3;}}
 if (sw3 ==3){ if (r1==1){ digitalWrite(relay2, HIGH); r1=2; }}
 if (sw4 ==3){ if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}}
  }}}
  
  //avg_ios=avg_ios+percent;
  //if ( count3 >= AVG){
  //avg_ios= avg_ios/AVG;
 // Serial.print("TaskValueSet,2,1,"); Serial.println(avg_ios);  // percent --  Diverter Percentage  
 // }
}

if ( stat == 1){
   while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
   while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
 //Serial.println("zero crossing");
   delayMicroseconds(period/2);
  if (PWM == 0){
  analogWrite(pulse2, DIVERT );
if (ios ==2){  
if ( DIVERT >= uDIS){  
  if ( count3 >= AVG){
 if (sw1 ==3){ if (r1 ==3){ digitalWrite(relay4, HIGH); r1=4;}}
 if (sw2 ==3){ if (r1 ==2){ digitalWrite(relay3, HIGH); r1=3;}}
 if (sw3 ==3){ if (r1==1){ digitalWrite(relay2, HIGH); r1=2; }}
 if (sw4 ==3){ if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}}
  }}}
  
  }else{
   settingPWM(pulse2, DIVERT); 
  }
if (ios ==2){  
if ( DIVERT >= uDIS){  
  if ( count3 >= AVG){
 if (sw1 ==3){ if (r1 ==3){ digitalWrite(relay4, HIGH); r1=4;}}
 if (sw2 ==3){ if (r1 ==2){ digitalWrite(relay3, HIGH); r1=3;}}
 if (sw3 ==3){ if (r1==1){ digitalWrite(relay2, HIGH); r1=2; }}
 if (sw4 ==3){ if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}}
  }}}
  
  analogWrite(pulse3, 0 );
  if (SSR4 == 1){
  analogWrite(pulse4, 0 );  //enable for 4th ssr
  }
 TMP = (DIVERT+255); percent = (TMP/DIVS);
 Serial.print("TaskValueSet,2,1,"); Serial.println(percent);
 // avg_ios=avg_ios+percent;
 // if ( count3 >= AVG){
 //if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}   if you would like relay1 to come on sooner 
 // avg_ios= avg_ios/AVG;
 // Serial.print("TaskValueSet,2,1,"); Serial.println(avg_ios);  // percent --  Diverter Percentage 
 // }
}
if ( stat == 2){
   while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
 //if (PWM == 0) delayMicroseconds(period/2);
  //Serial.println("zero crossing");
  analogWrite(pulse3, DIVERT );
  if (SSR4 == 1){
  analogWrite(pulse4, 0 );                  //enable for 4th ssr
  }
 TMP = (DIVERT+510);  percent = (TMP/DIVS);
  Serial.print("TaskValueSet,2,1,"); Serial.println(percent);
if (ios ==3){  
if ( DIVERT >= uDIS){  
  if ( count3 >= AVG){
 if (sw1 ==3){ if (r1 ==3){ digitalWrite(relay4, HIGH); r1=4;}}
 if (sw2 ==3){ if (r1 ==2){ digitalWrite(relay3, HIGH); r1=3;}}
 if (sw3 ==3){ if (r1==1){ digitalWrite(relay2, HIGH); r1=2; }}
 if (sw4 ==3){ if (r1 ==0){ digitalWrite(relay1, HIGH); r1=1;}}
  }}
   //avg_ios=avg_ios+percent;
 // if ( count3 >= AVG){
 // avg_ios= avg_ios/AVG;
 // Serial.print("TaskValueSet,2,1,"); Serial.println(avg_ios);  // percent --  Diverter Percentage 
  }
}
if ( stat == 3){
   while((millis()-start)<200 && analogRead(aIn)>512  );//wait for positive half period to expire
  while((millis()-start)<200 && analogRead(aIn)<512 );//wait for negative half period to expire
 delayMicroseconds(period/2);
  //Serial.println("zero crossing");
  analogWrite(pulse4, DIVERT );                  //enable for 4th ssr
  TMP = (DIVERT+765); percent = (TMP/DIVS);
   Serial.print("TaskValueSet,2,1,"); Serial.println(percent);
  //  avg_ios=avg_ios+percent;
  //if ( count3 >= AVG){
 // avg_ios= avg_ios/AVG;
  //Serial.print("TaskValueSet,2,1,"); Serial.println(avg_ios);  // percent --  Diverter Percentage 
  //}
}    
  }

 //####### Unison   pulse ######### 
    
  if (type == 1){
   // Serial.println(" solar Diversion -  In Unison");
    for(int i=0;i < ios; i++){

 DIVERT = curgrid ;

if ( i == 0){
  analogWrite(pulse1, DIVERT );
}
if ( i == 1){
  analogWrite(pulse2, DIVERT );
}
if ( i == 2){
  analogWrite(pulse3, DIVERT );
}
if ( i == 3){
  analogWrite(pulse4, DIVERT );                  //enable for 4th ssr
}
 percent = (DIVERT/DIVS);
   avg_ios=avg_ios+percent;
  if ( count3 >= AVG){
  avg_ios= avg_ios/AVG;
  Serial.print("TaskValueSet,2,1,"); Serial.println(avg_ios);  // percent --  Diverter Percentage 
  }
  } 
  }

//analogWrite(invstatus, curgrid4);      // led display  showing overproduction 

}

//############################# phase angle #########################
      if (ssr==1){
    
    //####### Zerocrossing #######
        
      boolean st=false;                                  //an indicator to exit the while loop
      unsigned long start = millis();                    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st==false)                                       //the while loop...
  {
    sV = analogRead(0);                                  //using the voltage waveform
    if ((sV < (1024*0.55)) && (sV > (1024*0.45))) st=true;  //check its within range
    if ((millis()-start)>2000) st = true;
  }
     sV=map(curgrid4,0,255,10000,0);                    //delay before pulse  
     delayMicroseconds(sV); 
     analogWrite(pulse,curgrid4);                        
     Serial.print("TaskValueSet,2,2,"); Serial.println(curgrid4);
    }

//################# End of  PWM  controll ############################   
//############## LCD ############ 

if (LCD==1) {
kw =  (grid / 1000) ;
per = ( curgrid / 254);
per = (1 - per);
//per = ( 100 * per);
    lcd.backlight();
    lcd.clear();      
    lcd.setCursor(0,0);
    lcd.print("KWATTS ");
    lcd.print(kw);
    lcd.setCursor(0,1);
    lcd.print("Volts  ");
    lcd.print(ct1.Vrms);
    lcd.setCursor(0,2);
    lcd.print("GTI    ");
    lcd.print(invert);
    if (CT1){
    lcd.setCursor(0,3);  
    lcd.print("Divert ");   //displays Diverter usage   
    //lcd.print ( "-");
    lcd.print (diverter);
    }
    if (CT4){
      lcd.setCursor(0,3);
      lcd.print("Wind   ");   // displays  wind inverter output 
      lcd.print (wind);
    }
}
 if (count3 >= AVG){
    
   volt=volt/AVG;
   Serial.print("TaskValueSet,1,4,"); Serial.println(volt);
   volt=0;
   count3=0;
   avg_255=0;
   avg_ios=0;
   }
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;
  //// ##### if you wish to send data  
  if (settled)                                                            
  {     
    Serial.print("TaskValueSet,3,1,"); Serial.println(_INPUTa);  
    if (sw1 ==3) {Serial.println("TaskValueSet,4,1,3");} 
    if (sw2 ==3) {Serial.println("TaskValueSet,4,2,3");} 
    if (sw3 ==3) {Serial.println("TaskValueSet,4,3,3");} 
    if (sw4 ==3) {Serial.println("TaskValueSet,4,4,3");}
                                                                                                         
  }

}
