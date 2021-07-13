// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
#include "MatrixHardwareCustom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>


#define SENSOR_1 34
#define SENSOR_2 35

const uint8_t sensor_pins[3] = {SENSOR_1, SENSOR_2};

#define COLOR_DEPTH 24                  // leave this as 24 for this sketch

const uint16_t kMatrixWidth  = 64;        // must be multiple of 8
const uint16_t kMatrixHeight = 32;
const uint8_t kPanelType     = SMARTMATRIX_HUB75_32ROW_64COL_MOD8SCAN;   // use SM_PANELTYPE_HUB75_16ROW_MOD8SCAN for common 16x32 panels

const uint8_t kRefreshDepth = 36;       // leave as 36 for this sketch
const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
//const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
//const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
//SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);

void setup() {

  for(uint8_t i=0; i<2; i++) pinMode(sensor_pins[i], INPUT);
   
  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setRotation(rotation270);
  matrix.setBrightness(20);
  
  backgroundLayer.setFont(font6x10);
  
  // do a (normally unnecessary) swapBuffers call to work around ESP32 bug where first swap is ignored
  backgroundLayer.swapBuffers();   
}

/*
void drawBitmap(int16_t x, int16_t y, const ico8x8* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      if(bitmap->pixel_data[(i*bitmap->width + j)*3 + 0]>10) {
        rgb24 pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                        bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                        bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };
        backgroundLayer.drawPixel(x + j, y + i, pixel);
      }
    }
  }
}
*/

static const rgb24 
W = {0xFF, 0xFF, 0xFF},
D = {0x00, 0x00, 0x00},
R = {0xFF, 0x00, 0x00},
Y = {0xFF, 0xDF, 0x00},
G = {0x00, 0xFF, 0x00}; 

float   timers[2] = {0.0};
uint8_t signals[2] = {0};

#define GREEN  0
#define RED    1
#define YELLOW 2
uint8_t states[2] = {GREEN, GREEN};
uint8_t old_states[2] = {GREEN, GREEN};

void inline drawTimer (uint32_t t, uint8_t h) {
    char timer[] = "00:00";
    t /= 20;
    timer[2] = t%2 ? ' ' : ':';
    t /=2;
    timer[4] = '0'+t%10;
    timer[3] = '0'+t/10%6;
    timer[1] = '0'+t/60%10;
    timer[0] = '0'+t/600%6;
    backgroundLayer.drawString(1,h, W, timer);
}

void inline drawSignal(uint8_t i) {
  rgb24 c;
  if (states[i]==255) { 
    timers[i]++; 
    c = R;
  } else if(states[i]>0 && states[i]<255) {
    uint8_t v = abs(8-states[i]%16);
    c = (rgb24){63+v*16,63+v*16,0};
  } else {
     c = G;
     timers[i] = 0;
  }
  backgroundLayer.fillRectangle( 0,  i*32, 31, i*32+31, c);
  if (states[i] == 255) drawTimer(timers[i], i*32);
}

void inline updateSignals() {
  
  for(uint8_t i=0; i<2; i++) {
    uint8_t s = digitalRead(sensor_pins[i]);
    s=1;
    if(signals[i]!=s) {
      if(s) states[i] = 255; // 0 -> 1 to Red
      else  states[i] = 254; // 1 -> 0 to Yellow
    }
    signals[i] = s;

    // timeout yellow state
    if(states[i]>0 && states[i]<255) states[i]--;
  }
  
}

inline void drawSignals() {
   for(uint8_t i=0; i<2; i++) drawSignal(i);
}

void loop() {

  updateSignals();
  drawSignals();
 
  
  backgroundLayer.swapBuffers();   
  delay(20);  
  
  backgroundLayer.fillScreen({0,0,0});
  // delay(20);  
}
