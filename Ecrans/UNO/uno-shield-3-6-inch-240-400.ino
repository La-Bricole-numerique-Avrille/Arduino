// UNO_ST7793_240x400_wiper.ino
//
// microcontroller Arduino Uno
// display: UNO shield 3.6 inch 240*400 parallel TFT ST7793
// November 3, 2023
// original edited by Floris Wouterlood
// public domain
   
   #include <SPI.h>              
   #include <Adafruit_GFX.h>                                                      // hardware-specific library
   #include <MCUFRIEND_kbv.h>                                                     // hardware-specific library

   MCUFRIEND_kbv tft;

// some principal color definitions
// RGB 565 color picker at https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
   #define WHITE       0xFFFF
   #define BLACK       0x0000
   #define BLUE        0x001F
   #define RED         0xF800
   #define GREEN       0x07E0
   #define CYAN        0x07FF
   #define MAGENTA     0xF81F
   #define YELLOW      0xFFE0
   #define GREY        0x2108 
   #define TEXT_COLOR  0xFFFF
   #define RAINDROP    0x7BCF
   #define LEATHER     0x9346 
   
   #define DEG2RAD 0.0174532925   
   
// center coordinates for wiper pivot
// for screens bigger than 240x320 you only have to change these two x-y coordinates
// there are two semi-independent wipers, defined by their wipers and begin-endWiper parms. 
//
   int pivot0_x, pivot0_y;                                                        // wiper 0 = left wiper
   float wiper0_in_x, wiper0_in_y;
   float wiper0_out_x, wiper0_out_y;
   float wiper0_in_x_old, wiper0_in_y_old;
   float wiper0_out_x_old, wiper0_out_y_old;
   float wiper1_in2_x, wiper1_in2_y; 
   float wiper1_out2_x, wiper1_out2_y; 

   int pivot1_x, pivot1_y;                                                        // wiper 1 = right wiper
   float wiper1_in_x,  wiper1_in_y;
   float wiper1_out_x, wiper1_out_y;
   float wiper1_in_x_old, wiper1_in_y_old;
   float wiper1_out_x_old, wiper1_out_y_old;

   float wiperAngle;
   int sweepTime = 0;
   int beginWiper = 190; 
   int endWiper = 340;
   int sweepDir;

   int distalWiperLength = 100;
   int proximalWiperLength = 30;
   int i,j;
   int jumpLeftRight = 0;

   int x_drop;
   int y_drop;
   int raindropRadius;

   int wheelCenter_x;                                                             // steering wheel
   int wheelCenter_y;      
   float wheel_out_x; 
   float wheel_out_y;
   float wheelRadius; 
   float wheelAngle;


void setup() {

   uint16_t ID;
   ID = tft.readID();                                                             // valid for Uno shields  

   Serial.begin (9600);
   Serial.println (); 
   Serial.println ();   
   Serial.println ("windshield wiper on display 400*240");  
   
   tft.reset();
   tft.begin (ID);                                                                // initialize SPI bus 
   tft.setRotation (3);                                                           // landscape                                          
   tft.fillScreen (BLACK);

   pivot0_x = 140;                                                                // all x-positions refer to this one
   pivot0_y = 140;                                                                // all y-positions refer to this one
   pivot1_x = pivot0_x+120;
   pivot1_y = pivot0_y;
   
   wiper0_in_x_old  = pivot0_x;                                                   // necessary to make start of wiping within wiping areas
   wiper0_in_y_old  = pivot0_y;
   wiper1_in_x_old  = pivot1_x;
   wiper1_in_y_old  = pivot1_y;
   wiper0_out_x_old = pivot0_x;
   wiper0_out_y_old = pivot0_y;
   wiper1_out_x_old = pivot1_x;
   wiper1_out_y_old = pivot1_y;

   tft.drawRoundRect (pivot0_x-110, pivot0_y-110, 330,  110, 10, CYAN);           // windshield     
   tft.drawRoundRect (pivot0_x-108, pivot0_y-109, 327,  108, 10, RAINDROP);       // zone where raindrops will appear
   tft.drawRoundRect (pivot0_x-107, pivot0_y-108, 324,  106, 10, RAINDROP);       // zone where raindrops will appear   

   tft.fillRoundRect (pivot0_x+40, pivot0_y-125, 40,  16, 5, RAINDROP);           // rear mirror 
   tft.drawRoundRect (pivot0_x+40, pivot0_y-125, 40,  16, 5, CYAN);               // rear mirror 
   tft.drawLine (pivot0_x+58, pivot0_y-130, pivot0_x+58, pivot0_y-125, CYAN);     // rear mirror 
   tft.drawLine (pivot0_x+62, pivot0_y-130, pivot0_x+62, pivot0_y-125, CYAN);     // rear mirror  

   wheelCenter_x = pivot0_x-40;                                                   // semicircle steering wheel
   wheelCenter_y = pivot0_y+55;
   for (i=50; i<56; i++)
      {
       wheelRadius = i;
       semiCircleWheel ();
      }

   tft.drawCircle (pivot0_x-40, pivot0_y+21, 15, GREEN);                          // tachometer clock
   tft.drawLine   (pivot0_x-40, pivot0_y+21, pivot0_x-43, pivot0_y+8,  GREEN);    // tachometer needle
   tft.fillRoundRect (pivot0_x-65, pivot0_y+35, 50,  26, 6, LEATHER);             // steering wheel pad
   tft.drawRoundRect (pivot0_x-55, pivot0_y+38, 31,  20, 3, BLACK);               // steering wheel pad

   tft.fillTriangle (pivot0_x-65, pivot0_y+40, pivot0_x-80, pivot0_y+24,          // steering wheel left spoke  
       pivot0_x-60, pivot0_y+35, LEATHER);          
   tft.fillTriangle (pivot0_x-80, pivot0_y+24, pivot0_x-60, pivot0_y+35,          // steering wheel left spoke  
       pivot0_x-77, pivot0_y+22, LEATHER);
   tft.fillTriangle (pivot0_x-19, pivot0_y+34, pivot0_x-3, pivot0_y+20,           // steering wheel right spoke 
       pivot0_x-16, pivot0_y+40, LEATHER);    
   tft.fillTriangle (pivot0_x-3, pivot0_y+20, pivot0_x-16, pivot0_y+40,
       pivot0_x, pivot0_y+22, LEATHER);                                           // steering wheel right spoke 

   tft.drawCircle (pivot0_x+32, pivot0_y+35, 15, GREEN);                          // rpm dial
   tft.drawLine   (pivot0_x+30, pivot0_y+35, pivot0_x+39, pivot0_y+38, GREEN);    // rpm needle
   
   tft.drawCircle (pivot0_x+65, pivot0_y+35, 15, GREEN);                          // temperature dial
   tft.drawLine   (pivot0_x+65, pivot0_y+35, pivot0_x+58, pivot0_y+27, GREEN);    // temperature needle

   tft.drawRoundRect (pivot0_x+130, pivot0_y+20, 86,  41, 6, LEATHER);            // glove compartment
   tft.drawCircle (pivot0_x+173, pivot0_y+28, 3, LEATHER);                        // button on glove compartment
  
   j = beginWiper;
}


void loop(void){

   if (j == beginWiper)
      {
       sweepDir = 1;
       Serial.println ("wipe");
      }
 
   j=j+sweepDir;
   raindrops ();
   wiperAngle= (j*DEG2RAD);                                                       // convert angle to radians
   sweepWiper_0 (); // left wiper
   sweepWiper_1 (); // right wiper
   delay(sweepTime);
 
   if (j == endWiper) sweepDir = -1;
}


void sweepWiper_0 (){                                                             // left wiper

   tft.drawLine (wiper0_in_x_old, wiper0_in_y_old, wiper0_out_x_old,              // remove old wiper blade
        wiper0_out_y_old, BLACK);   
   tft.drawLine (wiper0_in_x_old-1, wiper0_in_y_old-1, wiper0_out_x_old-1,
       wiper0_out_y_old-1, BLACK);  
   tft.drawLine (pivot0_x, pivot0_y, wiper0_in_x_old, wiper0_in_y_old, BLACK);    // remove old wiper shaft

   tft.drawPixel (wiper0_in_x_old, wiper0_in_y_old, RAINDROP); 
   tft.drawPixel (wiper0_out_x_old, wiper0_out_y_old, RAINDROP); 

   wiper0_out_x = (pivot0_x + (distalWiperLength*cos (wiperAngle)));
   wiper0_out_y = (pivot0_y + (distalWiperLength*sin (wiperAngle)));
   wiper0_in_x = (pivot0_x + (proximalWiperLength*cos (wiperAngle)));
   wiper0_in_y = (pivot0_y + (proximalWiperLength*sin (wiperAngle)));
   
   tft.drawLine (wiper0_in_x, wiper0_in_y, wiper0_out_x, wiper0_out_y, WHITE);
   tft.drawLine (pivot0_x, pivot0_y, wiper0_in_x, wiper0_in_y, RED);

   wiper0_out_x_old = wiper0_out_x;
   wiper0_out_y_old = wiper0_out_y;
   wiper0_in_x_old  = wiper0_in_x;
   wiper0_in_y_old  = wiper0_in_y;
   
   tft.fillCircle (pivot0_x,pivot0_y,2,GREEN);                                    // restore centerpoint
}


void sweepWiper_1 (){                                                             // right wiper

   tft.drawLine (wiper1_in_x_old, wiper1_in_y_old, wiper1_out_x_old,              // remove old wiper blade
      wiper1_out_y_old, BLACK);  
   tft.drawLine (wiper1_in_x_old-1, wiper1_in_y_old-1, wiper1_out_x_old-1,
      wiper1_out_y_old-1, BLACK);   
   tft.drawLine (pivot1_x, pivot1_y, wiper1_in_x_old, wiper1_in_y_old, BLACK);    // remove old wiper shaft

   tft.drawPixel (wiper1_in_x_old, wiper1_in_y_old, RAINDROP); 
   tft.drawPixel (wiper1_out_x_old, wiper1_out_y_old, RAINDROP); 

   wiper1_out_x = (pivot1_x + (distalWiperLength*cos (wiperAngle)));
   wiper1_out_y = (pivot1_y + (distalWiperLength*sin (wiperAngle)));
   wiper1_in_x  = (pivot1_x + (proximalWiperLength*cos (wiperAngle)));
   wiper1_in_y  = (pivot1_y + (proximalWiperLength*sin (wiperAngle)));
   
   tft.drawLine (wiper1_in_x, wiper1_in_y, wiper1_out_x, wiper1_out_y, WHITE);
   tft.drawLine (pivot1_x, pivot1_y, wiper1_in_x, wiper1_in_y, RED);

   wiper1_out_x_old = wiper1_out_x;
   wiper1_out_y_old = wiper1_out_y;
   wiper1_in_x_old  = wiper1_in_x;
   wiper1_in_y_old  = wiper1_in_y;
   
   tft.fillCircle (pivot1_x,pivot1_y,2,GREEN);                                    // restore centerpoint
}

void raindrops (){                                                                // raindops keep fallin' on my head

   x_drop = random (pivot0_x-105, pivot0_x+216);
   y_drop = random (pivot0_y-105, pivot0_y-5);
   raindropRadius = random (1,4);
   tft.drawCircle (x_drop, y_drop, raindropRadius, RAINDROP);              
}

void semiCircleWheel (){
   for (j=180; j<360; j++){
   wheelAngle = j*DEG2RAD;
   wheel_out_x = (wheelCenter_x + (wheelRadius*cos (wheelAngle)));
   wheel_out_y = (wheelCenter_y + (wheelRadius*sin (wheelAngle)));
   tft.drawPixel (   wheel_out_x,    wheel_out_y, LEATHER);
   }
}
