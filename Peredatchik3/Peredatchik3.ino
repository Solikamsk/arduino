
/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>
 
#define SPEED         (uint16_t)2000
#define RX_PIN        (uint8_t)11
#define TX_PIN        (uint8_t)12 //это пере дача
#define PTT_PIN       (uint8_t)10
#define PTT_INVERTED  false

int U0_PIN = A0;
int U1_PIN = A1;

RH_ASK driver(SPEED, RX_PIN, TX_PIN, PTT_PIN, PTT_INVERTED);
 
void setup() {
  Serial.begin(115200);
  if (! driver.init()) {
    Serial.println(F("RF init failed!"));
    while (true) {
      delay(1);
    }
  }
  pinMode(LED_BUILTIN, OUTPUT);
}

void send_command(uint8_t msg) {
    digitalWrite(LED_BUILTIN, LOW);
    driver.send((uint8_t *)&msg,  sizeof(msg));
    //driver.send(&msg,  strlen(msg));
    driver.waitPacketSent();
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  //uint8_t data[2] = {0, 0};
  //char msg = 110;//null
  static unsigned long tm = 0;
  static uint8_t old_msg = 110;
  uint8_t msg; 
  const int porog = 50;
  const int porog2 = 500;
  static int arrayU[30];
  static byte curNum = 0;
  static boolean flagU = false;
  
  int U0 = analogRead(U0_PIN);
  int U1 = analogRead(U1_PIN);

  if (U0 < 10 && U1 < 10) {
    //нет напражения
    msg = 'z'; //zero
  } else if (U0 > 100 && U1 > 1020) {
    //короткое замыкание
    msg = 'b'; //back 98
  } else if (U0 > 100 && U1  < porog) {
    //нет рабочего процесса
    unsigned long delta;
    unsigned long tm_curr;
    tm_curr = millis();
    delta = tm_curr - tm;
    if (delta >= 5000.00) {
      //Serial.println(delta);
      msg = 'm'; //move 109
    }
  //} else if ((U0 > 1020 && U1 > porog2)) {
  //слишком высокое напряжение. Шаг назад    
  } else {
    //рабочий процесс. Запомним время. 5 сек, чтобы не двигаться вперед
    msg = 's'; //stop 115
    tm = millis();
  }
  
  if (msg == 's') {
    unsigned int SumDelta = 0;

    arrayU[curNum] = U1;
    curNum++;
    if (curNum > 29) {
      curNum = 0;
      flagU = true;
    }  
    
    if (flagU) {
      SumDelta = 0;
      for(int i = 0; i < 29; i++) {
         SumDelta =+ abs(arrayU[i+1] - arrayU[i]);
      } 
      Serial.print("SumDelta ");
      Serial.println(SumDelta);
   }
  } else {
    flagU = false;
    curNum = 0;
  }
  
  if (old_msg != msg) {
    Serial.print("U0; ");
    Serial.print(U0);
    Serial.print(" U1 ");
    Serial.print(U1);
    Serial.print(" msg ");
    Serial.println(byte(msg));
    old_msg = msg;

    send_command(msg); 
    
    if (msg == 'b') {
      Serial.println("waiting");
      delay(17000);
      old_msg = 0;
    }
  } else if (msg == 'm') {  
    send_command(msg);
    Serial.print("move ");
    Serial.println(U1);
    delay(1200);
  } else if (U1 > 150) {
    Serial.println("U1 " && U1);
    //Serial.println(U1);
  }
  /*
  */

  //delay(100);
}
