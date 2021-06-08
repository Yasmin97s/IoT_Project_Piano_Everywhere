#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// touch includes
#include <Wire.h>
#include <Adafruit_MPR121.h>
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();

uint16_t lasttouched1 = 0;
uint16_t currtouched1 = 0;

uint16_t lasttouched2 = 0;
uint16_t currtouched2 = 0;

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(10, 11);

#define LED_PIN    6
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 60
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int pin=6;
const uint32_t BAUD_RATE = 9600;

//MP3 includes, constants, and objects
#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 7//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 8//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you donot use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);

int8_t volume = 0x1a;//0~0x1e (30 adjustable level)
int8_t folderName = 0x03;//folder name must be 01 02 03 04 ...
int8_t fileName = 0x01; // prefix of file name must be 001xxx 002xxx 003xxx 004xxx ...


void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  Serial.begin(BAUD_RATE);
  pinMode(pin,OUTPUT);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Setup touch sensors
  if (!cap1.begin(0x5A)) {
    //Serial.println("MPR121 A not found, check wiring?");
    while (1);
  }
  cap1.setThreshholds(12,6);
  //Serial.println("MPR121 A found!");
  
  if (!cap2.begin(0x5B)) {
    //Serial.println("MPR121 B not found, check wiring?");
    while (1);
  }
  cap2.setThreshholds(12,6);
  //Serial.println("MPR121 B found!");

  // Setup MP3 module
  delay(500);//Requires 500ms to wait for the MP3 module to initialize  
  mp3.setVolume(volume);
  delay(50);//you should wait for >=50ms between two commands
  
}

void loop() {
  currtouched1 = cap1.touched();
  currtouched2 = cap2.touched();

  for (int i = 0; i < 12; i++) {
    if ((currtouched1 & _BV(i)) && !(lasttouched1 & _BV(i)) ) {
      //Serial.print(i);
      //Serial.println(" touched of A");
      colorWipe(i,0);
      String str = "z";
      /*switch (i) {
       case 0:
         mp3.playWithFileName(folderName,0x01);
         str = "a";
         break;
       case 1: 
         mp3.playWithFileName(folderName,0x02);
         str = "b";
         break;
       case 2:
         mp3.playWithFileName(folderName,0x03);
         str = "c";
         break;
       case 3:
         mp3.playWithFileName(folderName,0x04);
         str = "d";
         break;
       case 4:
         mp3.playWithFileName(folderName,0x05);
         str = "e";
         break;
       case 5:
         mp3.playWithFileName(folderName,0x06);
         str = "f"; // e f g h i j k l m n o p
         break;
       case 6:
         mp3.playWithFileName(folderName,0x07);
         str = "g";
         break;
       case 7:
         mp3.playWithFileName(folderName,0x01);
         str = "h";
         break;
       case 8:
         str = "i";
         break;
       case 9:
         str = "j";
         break;
       case 10:
         str = "k";
         break;
       case 11:
         mp3.playWithFileName(folderName,0x01);
         str = "l";
         break;
       default:
         // statements
      break;
     }*/
     str = String((char)('a' + i));
     colorWipe(i,0);
     mp3.playWithFileName(folderName,i+1);
     Serial.print(str);
     delay(50);//you should wait for >=50ms between two MP3 commands
    }
    else if (!(currtouched1 & _BV(i)) && (lasttouched1 & _BV(i)) ) {
      //Serial.print(i);
      //Serial.println(" released of A");
      strip.clear();
      strip.show(); 
    }
  }

  lasttouched1 = currtouched1;
  lasttouched2 = currtouched2;

  if(Serial.available()>0){
    String info = Serial.readStringUntil('\n');
    if (info == "all wrong"){
      colorWipe(19,0);
    } else if(info == "not matched"){
      colorWipe(20,0);
    }
    else if(info=="matched"){
      colorWipe(21,0);
    }
    else if(info=="Waiting"){
      colorWipe(22,0);
    } else {
      playSong(info);
    }
  }
}

//asumes notes are 'a' to 'm'
void playSong(String song){
  int len = song.length()/2;
  for(int i=0; i<len; i++){
    char c = song.charAt(2*i);
    int num = c - 'a';
    mp3.playWithFileName(folderName,num+1);
    colorWipe(num,0);
    delay(500);//you should wait for >=50ms between two MP3 commands 
  }
  strip.clear();
  strip.show();
}

void colorWipe(int i, int wait) {
  uint32_t color=strip.Color(0,   0,   0); //(G,R,B)
  //int offset=0;
  if(i==0){
    color = strip.Color(255,   0,   0); //Green
    //offset=0;
  }
  if(i==1){
    color = strip.Color(0,  255 , 0 ); 
    //offset=2;
  }
   if(i==2){
    color = strip.Color(0,  0 , 255 ); 
    //offset=4;
  }
  if(i==3){
    color = strip.Color(100,  100 , 100 ); 
    //offset=6;
  }
  if(i==4){
    color = strip.Color(50,  250 , 0 ); //Orange
    //offset=8;
  }
  if(i==5){
    color = strip.Color(0,  120 , 50 ); //Pink
    //offset = 10;
  }
  if(i==6){
   color = strip.Color(70,   200,   80);
   //offset=12;
  }
  if(i==11){
   color = strip.Color(200,   150,   200); //White-ish
   //offset=14;
  }
  if(i==19){ // not matched
    color = strip.Color(0,   255,   0);  //Red
    
  }
  if(i==20){ // not matched
    color = strip.Color(50,  250 , 0 ); //Orange
    
  }
  if(i==21){ // matched
    color = strip.Color(255,   0,   0); //Green
  }

  if(i==22){ // Waiting
    color = strip.Color(128,   128,   128); //Gray
  }
  strip.clear();

  if(i >= 19 && i <= 22)
  { 
    for(int j=0; j < 29; j++) { // For each pixel in strip...
      strip.setPixelColor(j, color);         //  Set pixel's color (in RAM)
    }
    strip.show();                          //  Update strip to match
    delay(3000);                           //  Pause for a moment
    if(i != 22){
      strip.clear();
      strip.show();
    }
  }
  else
  {
    //for(int j=offset; j<offset+2; j++) { // For each pixel in strip...
      //strip.setPixelColor(j, color);         //  Set pixel's color (in RAM)
      strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      strip.show();                          //  Update strip to match
      delay(wait);                           //  Pause for a moment
    //}
  }
}
