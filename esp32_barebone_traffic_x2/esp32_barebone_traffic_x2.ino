#define CONFIG_ESP_INT_WDT_TIMEOUT_MS 500

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*

#define P_R1 12
#define P_G1 27
#define P_B1 14

#define P_R2 13
#define P_G2 26
#define P_B2 25

#define CH_A 19
#define CH_B 18 
#define CH_C  5

#define CLK  23
#define LAT  22
#define OE   21

*/

#define P_R1 2
#define P_G1 0
#define P_B1 15

#define P_R2  4
#define P_G2 17
#define P_B2 16

#define CH_A  5
#define CH_B 18  
#define CH_C 19

#define CLK  21
#define LAT  22
#define OE   23



#define SetColorPin(pin,  state) state > layer ? gpio |= 1 << pin : gpio &= ~(1 << pin);
#define SetPinFast(pin,  state) state? gpio |= 1 << pin : gpio &= ~(1 << pin);

int row = 0;
int c = 0;
int r = 0;

// Сколько по времени занято
float   timers[2] = {0.0};
// Сигал от датчика
uint8_t signals[2] = {0};
// Стейт
// 0      - зеленый
// 1..254 - желтый
// 255    - красный
uint8_t states[2] = {0};


void inline update_signals() {
  
  for(uint8_t i=0; i<2; i++) {
    // uint8_t s = digitalRead(sensor_pins[i]);
    // Тест - если рандом, мееняем сигнал
    uint16_t r = rand()%100;
    // Serial.println(r);
    uint8_t s = ( r == 1) ? !signals[i] : signals[i];
    // Если сигнал изменился
    if(signals[i]!=s) {
      //Serial.print("Signal changed: ");
      //Serial.print(i);
      //Serial.print("-");
      //Serial.println(s);
      // С нуля на единицу, зеленый -> переходит в красный
      if(s) states[i] = 255; // 0 -> 1 to Red
      // С единицы на ноль красный -> желтый
      else states[i] = 254; // 1 -> 0 to Yellow
      //else states[i] = 0;
    }
    signals[i] = s;

    // Если красный - таймер увеличиваем
    if(states[i]==255) { 
      timers[i]++;
    // Если зеленый таймер сбрасываем
    } if (states[i]==0) {
      timers[i] = 0;
    }
   
    // Если желтый, стейт постепенно сматываем до нуля
    if(states[i]>0 && states[i]<255) states[i]--;
  }
  
}


// Рисуем одну строку
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
      // 64 диода на панель 
      uint8_t i = col/64;
      // КРАСНЫЙ
      if (states[i]==255) { 
        SetPinFast(P_R1, 1);
        SetPinFast(P_G1, 0);  
        SetPinFast(P_B1, 0);
        SetPinFast(P_R2, 1);
        SetPinFast(P_G2, 0);  
        SetPinFast(P_B2, 0);      
      
      // ЖЕЛТЫЙ моргает
      } else if(states[i]>0 && states[i]<255) {
        // яркость желтого (зигзаг)
        // uint8_t v = abs(8-states[i]%16);
        // v = 63+v*16;
        // uint8_t r = rand()%128;
        // uint8_t c = v<r;
        uint8_t c = states[i]/4%2;
        SetPinFast(P_R1, c);
        SetPinFast(P_G1, c);  
        SetPinFast(P_B1, 0);
        SetPinFast(P_R2, c);
        SetPinFast(P_G2, c);  
        SetPinFast(P_B2, 0);

      // ЗЕЛЕНЫЙ
      } else {
        SetPinFast(P_R1, 0);
        SetPinFast(P_G1, 1);  
        SetPinFast(P_B1, 0);
        SetPinFast(P_R2, 0);
        SetPinFast(P_G2, 1);  
        SetPinFast(P_B2, 0);
      }
      
      GPIO.out = gpio;
      
//      // Импульс в клок
//      SetPinFast(CLK, 1);
//      GPIO.out = gpio;
//      SetPinFast(CLK, 0);
//      GPIO.out = gpio;
      
       GPIO.out_w1ts = ((uint32_t)1 << CLK); //SetPinFast is to fast!? wtf!?
       GPIO.out_w1tc = ((uint32_t)1 << CLK);
  }
  

  // Импульс в защёлку
  SetPinFast(LAT, HIGH);
  GPIO.out = gpio;
  SetPinFast(LAT, LOW);
  GPIO.out = gpio;

// Включаем дисплей
// SetPinFast(OE, LOW);

  // переключаем строку
  row+=1;
  row = row%8;
}

// Строка отрисовки
int loops = 10;
int loop_n = 0;
int loop_n_on = 0;
int loop_n_off = 1;

// Таймер вызова
hw_timer_t* displayUpdateTimer = NULL;
void IRAM_ATTR onDisplayUpdate() {
  if (loop_n == 0) draw_row();         //Display OFF-time (25 µs).
  if (loop_n == loop_n_on) {
    GPIO.out_w1tc = ((uint32_t)1 << OE);       //Turn Display ON
  };
  
  if (loop_n == loop_n_off) {
    GPIO.out_w1ts = ((uint32_t)1 << OE);       //Turn Display OFF
  }
  loop_n = (loop_n + 1)%loops;  
}

//runs faster then default void loop(). why? runs on other core?
void loop2_task(void *pvParameter)
{
 while (true)
 {
  c+=1;
  if((c%=64) == 0) {
    r+=1;
    r%=8;
  }
  update_signals();
  vTaskDelay(100);
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
 
  // delay(20);  

}
