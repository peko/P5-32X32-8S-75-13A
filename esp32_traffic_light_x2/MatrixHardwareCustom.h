 // Note: only one MatrixHardware_*.h file should be included per project

#ifndef MATRIX_HARDWARE_H
#define MATRIX_HARDWARE_H

// formula used is 80000000L/(cfg->clkspeed_hz + 1), must result in >=2.  Acceptable values 26.67MHz, 20MHz, 16MHz, 13.34MHz...
#define ESP32_I2S_CLOCK_SPEED (20000000UL)

#define HUB75_ADAPTER_LITE_V0_PINOUT    9

#ifdef GPIOPINOUT
#pragma GCC error "GPIOPINOUT previously set!"
#endif

#define GPIOPINOUT HUB75_ADAPTER_LITE_V0_PINOUT

#pragma message "MatrixHardware: HUB75 Adapter Lite V0 pinout"

//Upper half RGB
#define BIT_R1  (1<<0)   
#define BIT_G1  (1<<1)   
#define BIT_B1  (1<<2)   
//Lower half RGB
#define BIT_R2  (1<<3)   
#define BIT_G2  (1<<4)   
#define BIT_B2  (1<<5)   

// Control Signals
#define BIT_LAT (1<<6) 
#define BIT_OE  (1<<7)  

#define BIT_A (1<<8)    
#define BIT_B (1<<9)    
#define BIT_C (1<<10)   
#define BIT_D (1<<11)   
#define BIT_E (1<<12)   

// ADDX is output on RGB pins and stored in external latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
#define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
#define MATRIX_DATA_STORAGE_TYPE uint16_t
#define CLKS_DURING_LATCH   0

#define R1_PIN  GPIO_NUM_2
#define G1_PIN  GPIO_NUM_0
#define B1_PIN  GPIO_NUM_5

#define R2_PIN  GPIO_NUM_4
#define G2_PIN  GPIO_NUM_17
#define B2_PIN  GPIO_NUM_16

#define A_PIN   GPIO_NUM_5
#define B_PIN   GPIO_NUM_18
#define C_PIN   GPIO_NUM_19
#define D_PIN   -1
#define E_PIN   -1

#define CLK_PIN GPIO_NUM_21
#define LAT_PIN GPIO_NUM_22
#define OE_PIN  GPIO_NUM_23


//#define DEBUG_PINS_ENABLED
#define DEBUG_1_GPIO    GPIO_NUM_1
#define DEBUG_2_GPIO    GPIO_NUM_3

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
