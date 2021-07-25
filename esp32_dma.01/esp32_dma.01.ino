#include <esp32_i2s_parallel_v2.h>

#define P_R1 GPIO_NUM_27
#define P_G1 GPIO_NUM_26
#define P_B1 GPIO_NUM_14

#define P_R2 GPIO_NUM_13
#define P_G2 GPIO_NUM_12
#define P_B2 GPIO_NUM_25

#define CH_A GPIO_NUM_18
#define CH_B GPIO_NUM_17
#define CH_C GPIO_NUM_5

#define CLK  GPIO_NUM_22
#define LAT  GPIO_NUM_21
#define OE   GPIO_NUM_23

// Число строк
#define R_COUNT     8
// Число елементов в строке
#define R_DATA     64
// Дополнение к строке
#define R_CAP       0
// Длинна строки
#define R_LENGTH   (R_DATA+R_CAP)
#define F_LENGTH   (R_LENGTH*R_COUNT)

#define PRINTBIN(Num) for (uint32_t t = (1UL<< (sizeof(Num)*8)-1); t; t >>= 1) Serial.write(Num  & t ? '1' : '0'); // Prints a binary number with leading zeros (Automatic Handling)
#define PRINTBINL(Num) for (int i=0;i<(sizeof(Num)*8);i++) Serial.write(((Num >> i) & 1) == 1 ? '1' : '0'); // Prints a binary number with following Placeholder Zeros  (Automatic Handling)

#define SW(A) ((A)&1 ? (A)-1 : (A)+1)

uint16_t buf[F_LENGTH+4]; // [F_LENGTH];
int desccount = 1;
lldesc_t dmadesc;
    
void setup() {
//    Serial.begin(115200);   

    int gpios[12] = {P_R1, P_G1, P_B1, P_R2, P_G2, P_B2, CLK, LAT, OE, CH_A, CH_B, CH_C};
    for(int i=0; i<12 ; i++) {
      gpio_pad_select_gpio((gpio_num_t)gpios[i]);
      gpio_set_direction((gpio_num_t)gpios[i], GPIO_MODE_OUTPUT);
    }
      
    //buf = (uint16_t*) heap_caps_malloc(F_LENGTH*sizeof(uint16_t), MALLOC_CAP_DMA);
    link_dma_desc(&dmadesc, &dmadesc, buf, F_LENGTH*sizeof(uint16_t));  
   
    i2s_parallel_config_t cfg = {
        //            0     1     2     3     4     5    6   7     8     9    10
        .gpio_bus={P_R1, P_G1, P_B1, P_R2, P_G2, P_B2, LAT, OE, CH_A, CH_B, CH_C, -1, -1, -1, -1},
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
    i2s_parallel_send_dma(I2S_NUM_1, &dmadesc);
    
}

void loop() {
  static uint16_t j = 0;
      // 0x40 LATCH
      // 0x80 EO        
      for(uint16_t row=0; row<R_COUNT; row++) 
      {
        uint16_t* r = &buf[row*R_LENGTH];
        uint16_t abc = row<<8; //(row+R_COUNT-1)%R_COUNT<<8;
        
        for(uint16_t i=0; i<R_LENGTH; i++) {
          r[i] = abc | 0x80;
        }
        for(uint16_t i=0; i<64; i++) {
          r[SW(i)] |= (i+j+row)/2%8 | (i+j+row)/2%8<<3;
        }  
        r[R_LENGTH-1] |= 0x40; 
        r[R_LENGTH-1] &= ~0x80;
        
      }
      // buf[0] &= ~0x80;
      delay(8+j/8);
      j+=1;
      j%=R_LENGTH;
}
