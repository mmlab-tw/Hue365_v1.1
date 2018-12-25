#include <CapacitiveSensor.h>
#include "SoftwareSerial.h"

/* TOUCH SENSOR */
#define TOUCH_THRES 100
#define SENSE_INTERVAL 50

/* PROXIMITY SENSOR */
#define PROX_THRES 300
#define PROX_SENSE_INTERVAL 50


/* light mode intervals */
#define BLINK_VEL 100
#define BREATH_VEL 800

/* pins on attiny84 */
#define PIN_R 6
#define PIN_G 7
#define PIN_B 8

#define PIN_BASE 2
#define PIN_MODE 4
#define PIN_BRIGHT 5
#define PIN_HUE 3
#define PIN_PROX 9


#define PIN_RX 0
#define PIN_TX 1

/* 1: still, 2: blink, 3: breath */
#define MMODE 3 


CapacitiveSensor _cap_mode=CapacitiveSensor(PIN_BASE,PIN_MODE); 
CapacitiveSensor _cap_prox=CapacitiveSensor(PIN_BASE,PIN_PROX); 

int _mode=0;
int _vhue;
int _vbright=255;
int _vthres;

int _inWhich=0;

float _pre_prox;
long _start_prox,_start_touch;

double thue_,tbright_,tmode_,tprox_,dp_;
int r,g,b;
float smooth = 0.2;

double CapThresLow,CapThresDelta;

SoftwareSerial _soft_serial(PIN_RX, PIN_TX);


void changeMode(){  
  _mode=(_mode+1)%MMODE;
  _soft_serial.print("SET MODE ");
  _soft_serial.println(_mode);

  analogWrite(PIN_R,255);
  analogWrite(PIN_G,255);
  analogWrite(PIN_B,255);  
  delay(100);
  
}

void wheelColor(int p_,float br_,int &r_,int &g_,int &b_){
 p_=255-p_ % 255;   //
 
  if(p_<0 ){
      r_=0; g_=0; b_=0;
  }else if(p_<85){
    r_=p_*3; g_=255-p_*3; b_=0;
  }else if(p_<170){
    p_-=85;
    r_=255-p_*3; g_=0; b_=p_*3;
  }else{
    p_-=170;
    r_=0; g_=p_*3; b_=255-p_*3;
  }

  float pp=br_/255.0;
  r_*=pp;
  g_*=pp;
  b_*=pp;
  
}



void setup(){


  pinMode(PIN_R,OUTPUT);
  pinMode(PIN_G,OUTPUT);
  pinMode(PIN_B,OUTPUT);

  pinMode(PIN_RX,INPUT);
  pinMode(PIN_TX,OUTPUT);
  _soft_serial.begin(9600);


  _cap_mode.set_CS_AutocaL_Millis(0xFFFFFFFF);
  _cap_prox.set_CS_AutocaL_Millis(0xFFFFFFFF);

  _start_prox=_cap_prox.capacitiveSensorRaw(PROX_SENSE_INTERVAL);
  _start_touch=_cap_mode.capacitiveSensorRaw(SENSE_INTERVAL);

  updateCap();
  
}

void loop(){

  tmode_    =_cap_mode.capacitiveSensorRaw(SENSE_INTERVAL);
  float temp =_cap_prox.capacitiveSensorRaw(PROX_SENSE_INTERVAL);
  
  if(_start_prox<=0 || (temp>0 && temp<_start_prox)){
    _start_prox=temp;
    updateCap();
  }
  if(_start_touch<=0 || (tmode_<0 && tmode_<_start_touch)) _start_touch=tmode_;


  
  //smooth
  tprox_      = _pre_prox * smooth+ temp *(1-smooth);
  dp_         = tprox_-_pre_prox;
  _pre_prox   = tprox_;
  
  if(tmode_>_start_touch+TOUCH_THRES){
   changeMode();
  }else{
    if(tprox_>=_start_prox && tprox_<=_start_prox+PROX_THRES){
        _vhue=capToDist(tprox_);
    }
  }
  
  wheelColor(_vhue,_vbright,r,g,b);
  
  float t=1;
  switch(_mode){
    case 1:
      if(millis()%BLINK_VEL>BLINK_VEL/2) t=0;
      break;  
    case 2:
      t=sin((float)(millis()%BREATH_VEL)/BREATH_VEL*PI);
      break;  
  }
  
  analogWrite(PIN_R,255-(float)r*t);
  analogWrite(PIN_G,255-(float)g*t);
  analogWrite(PIN_B,255-(float)b*t);
  printDebug();
  
}

void updateCap(){
  CapThresLow=1.0/_start_prox;
  CapThresDelta=1.0/(_start_prox+(double)PROX_THRES)-CapThresLow;  

  _soft_serial.println("---------------");
 
  
  _soft_serial.print(" ThresLow= ");
  _soft_serial.print(CapThresLow);
  
  _soft_serial.print(" ThresDelta= ");
  _soft_serial.println(CapThresDelta);
  
  _soft_serial.println("---------------");
  
}

float capToDist(float cap_){
  
  float v_=1.0/constrain(cap_,_start_prox,_start_prox+PROX_THRES);
  float d_=255*(v_-CapThresLow)/(CapThresDelta);
 
  return constrain(d_,0,255);
}

void printDebug(){

  
  _soft_serial.print(" sprox= ");
  _soft_serial.print(_start_prox);
  
  _soft_serial.print(" stouch= ");
  _soft_serial.print(_start_touch);
  
  _soft_serial.print("   touch= ");
  _soft_serial.print(tmode_);
  _soft_serial.print("   proxi= ");
  _soft_serial.print(tprox_);

  

  _soft_serial.print("  hue=");
  _soft_serial.print(_vhue);
  
  _soft_serial.print("  (");
  _soft_serial.print(r);
  _soft_serial.print(",");
  _soft_serial.print(g);
  _soft_serial.print(",");
  _soft_serial.print(b);
  _soft_serial.println(")");
  
}



