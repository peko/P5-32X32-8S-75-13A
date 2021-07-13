#define CONFIG_ESP_INT_WDT_TIMEOUT_MS 500

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define P_R1 12
#define P_G1 27
#define P_B1 14

#define P_R2 33
#define P_G2 26
#define P_B2 25

#define CH_A 19
#define CH_B 18 
#define CH_C  5

#define CLK  23
#define LAT  22
#define OE   21

#define SetColorPin(pin,  state) state > layer ? gpio |= 1 << pin : gpio &= ~(1 << pin);
#define SetPinFast(pin,  state) state? gpio |= 1 << pin : gpio &= ~(1 << pin);

int row = 0;
int x_t = 0;
int r = 0;
void IRAM_ATTR draw_row() {
  uint32_t gpio = GPIO.out;
  
  // Выключаем дисплей
  SetPinFast(OE, HIGH);
  GPIO.out = gpio;

  // Задаем строку
  SetPinFast(CH_A, row & 1 << 0);
  SetPinFast(CH_B, row & 1 << 1);
  SetPinFast(CH_C, row & 1 << 2);
  GPIO.out = gpio;
  uint8_t r = rand();
  
  // rows 0-7
  // cols 0-63
  // +----- ~ ------+
  // |32          63| RGB 0
  // +----- ~ ------+
  // | 0          31| RGB 0
  // +----- ~ ------+
  // |32          63| RGB 1
  // +----- ~ ------+
  // | 0          31| RGB 1
  // +----- ~ ------+
  
  
  for (uint8_t col = 0; col < 32*4; col++) {
      uint8_t x = (col & 31) + ((col & 64) >> 1);
      uint8_t y = row + 8-((col & 32) >> 2);
      SetPinFast(P_R1, y== 0 && x<=x_t);
      SetPinFast(P_G1, y==15 && x>=x_t);
      SetPinFast(P_B1, x==x_t);
      SetPinFast(P_R2, x<x_t && x_t-x<r%8);
      SetPinFast(P_G2, x<x_t && x_t-x<r%16);
      SetPinFast(P_B2, x<x_t && x_t-x<r%4);
      GPIO.out = gpio;
      
//      // Импульс в клок
//      SetPinFast(CLK, 1);
//      GPIO.out = gpio;
//      SetPinFast(CLK, 0);
//      GPIO.out = gpio;
      
       GPIO.out_w1ts = ((uint32_t)1 << CLK);//SetPinFast is to fast!? wtf!?
       GPIO.out_w1tc = ((uint32_t)1 << CLK);
  }
  

  // Импульс в защёлку
  // SetPinFast(LAT, HIGH);
  // GPIO.out = gpio;
  // SetPinFast(LAT, LOW);
  // GPIO.out = gpio;

  GPIO.out_w1ts = ((uint32_t)1 << LAT);//SetPinFast is to fast!? wtf!?
  GPIO.out_w1tc = ((uint32_t)1 << LAT);

//  // Включаем дисплей
//  SetPinFast(OE, LOW);

  row+=1;
  row = row%8;


}

// Строка отрисовки
int loops = 10;
int loop_n = 0;
int loop_n_on = 9;
// Таймер вызова
hw_timer_t* displayUpdateTimer = NULL;
void IRAM_ATTR onDisplayUpdate() {
  if (loop_n == 0) draw_row();         //Display OFF-time (25 µs).
  if (loop_n == loop_n_on) {
    GPIO.out_w1tc = ((uint32_t)1 << OE);       //Turn Display ON
    //GPIO.out_w1ts = ((uint32_t)1 << OE);       //Turn Display ON
  }
  loop_n = loop_n + 1;
  if (loop_n >= loops) {
      loop_n = 0;
  }
}

//runs faster then default void loop(). why? runs on other core?
void loop2_task(void *pvParameter)
{
 while (true)
 {
  x_t+=1;
  if((x_t%=64) == 0) {
    r+=1;
    r%=8;
    Serial.println(r);

  }
  vTaskDelay(50);
 }
}

void setup() {
  
  Serial.begin(115200);

  pinMode(CLK, OUTPUT);
  pinMode(OE,  OUTPUT);
  pinMode(LAT, OUTPUT);

  pinMode(P_R1, OUTPUT);
  pinMode(P_G1, OUTPUT);
  pinMode(P_B1, OUTPUT);
  pinMode(P_R2, OUTPUT);
  pinMode(P_G2, OUTPUT);
  pinMode(P_B2, OUTPUT);

  pinMode(CH_A, OUTPUT);
  pinMode(CH_B, OUTPUT);
  pinMode(CH_C, OUTPUT);


  xTaskCreate(&loop2_task, "loop2_task", 2048, NULL, 5, NULL);
  
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  displayUpdateTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(displayUpdateTimer, &onDisplayUpdate, true);
  timerAlarmWrite(displayUpdateTimer, 2, true);
  timerAlarmEnable(displayUpdateTimer);

  Serial.println("OK");

}

void loop() {
  // put your main code here, to run repeatedly:

}
