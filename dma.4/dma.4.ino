#include <esp32_i2s_parallel_v2.h>

#define P_R1 27
#define P_G1 25
#define P_B1 26

#define P_R2 15
#define P_G2 16
#define P_B2  4

#define CH_A 17
#define CH_B 18
#define CH_C 19

#define CLK  21
#define LAT  22
#define OE   23

#define SIZE 2048
#define R_COUNT     8
#define R_DATA     64
#define R_CAP       2
#define R_LENGTH   (R_DATA + R_CAP)
#define HZ_10M 10000000

int bus[11] = {P_R1, P_G1, P_B1, P_R2, P_G2, P_B2, LAT, OE, CH_A, CH_B, CH_C};

static uint16_t buf[SIZE] = {0};

int desccount = 1;
lldesc_t dmadesc;
    
void setup() {
    for(int i=0; i<11 ; i++) {
      gpio_set_direction((gpio_num_t)bus[i], GPIO_MODE_OUTPUT);
    }
    
    for(int i=0; i<SIZE; i++){
      buf[i] = 1<<7;
    }
  
    link_dma_desc(&dmadesc, &dmadesc, buf, R_LENGTH*R_COUNT);
    
    i2s_parallel_config_t cfg = {
         
        //            0     1     2     3     4     5    6   7     8     9    10
        .gpio_bus={P_R1, P_G1, P_B1, P_R2, P_G2, P_B2, LAT, OE, CH_A, CH_B, CH_C, -1, -1, -1, -1, -1},
        .gpio_clk=CLK,
        .sample_rate=1000000,   
        .sample_width=I2S_PARALLEL_WIDTH_16,        
        .desccount_a=desccount,
        .lldesc_a=&dmadesc,
        .desccount_b=desccount,
        .lldesc_b=&dmadesc,
        .clkphase=1
    };

    // Setup I2S 
    i2s_parallel_driver_install(I2S_NUM_1, &cfg);
   
    // Start DMA Output
    i2s_parallel_send_dma(I2S_NUM_1, &dmadesc);
    
}

void loop() {
    
    static int j = 32;
    
    for(int i=0; i<SIZE; i++){
      buf[i] = 1UL<<7; // OE ON
    }
    
    for(uint16_t r=0; r<R_COUNT; ++r) {
      
      buf[R_LENGTH*r+j+0] |= 1; buf[R_LENGTH*r+0] |= 32;
      buf[R_LENGTH*r+j+1] |= 2; buf[R_LENGTH*r+2] |= 16;
      buf[R_LENGTH*r+j+2] |= 4; buf[R_LENGTH*r+4] |=  8;

      int last = R_LENGTH*(r+1)-1;

      // ABC
      buf[last-2] |= 1<<8;
//      Serial.print(r);
//      Serial.print("-");
//      Serial.println(r<<8); 
      // buf[last-0] |= (1&r)<<8; 
      // buf[last-0] |= (2&r)<<8;
      // buf[last-0] |= (4&r)<<8;
      
      buf[last-2] |=  1<<6; // LATCH ON
      buf[last-2] &=~(1<<7); // OE ON
      buf[last-1] &=~(1<<6); // LATCH OFF
      buf[last-1] |=  1<<7; // OE OFF
      
      // buf[ROWL*r+j+1] |= 1<<6;
      
      //buf[last] &= ~(1<<7);
      //buf[ROWL*r+j+3] &= ~(1<<7);
        
    }    
    
    j++;
    j%=32;
    delay(100);

}
