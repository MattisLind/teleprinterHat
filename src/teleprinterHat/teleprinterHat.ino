#define LED_RATE 1250    // in microseconds; should give 0.5Hz toggles
#include "RingBuffer.h"
#include "SoftUART.h"
#include "BaudotCodec.h"

class RingBuffer txBuffer;
class RingBuffer rxBuffer;
class BaudotCodec baudotCodec(1);



const int pwmOutPin = PA0; // pin10
HardwareTimer pwmtimer(2);

// We'll use timer 2
HardwareTimer timer(1);

uint16 maxduty;  
uint32 period, mypulse;
int duty=2200;

int shiftMode = 0;
int state = -1;
char ch;

void txBit (char bit) {
  if (bit & 1) {
    digitalWrite(PA2,HIGH);  
  } else {
    digitalWrite(PA2,LOW);
  }
}

char rxOut = 1;
int rxSum=16;

char rxBit () {
  char in = (~digitalRead(PA6)) & 1;
  if (in==1) {
     rxSum++;
     if (rxSum>16) rxSum=16;
  } else {
    rxSum--;
    if (rxSum<0) rxSum=0;
  }
  if (rxOut == 1 && rxSum < 8) {
     rxOut = 0;
  }
  if (rxOut == 0 && rxSum > 8) {
     rxOut = 1;
  }
  digitalWrite(PC14, rxOut);
  return rxOut;
}

int debugState = 0;

void debug () {
  debugState ^= 1;
  digitalWrite (PC15, debugState);  
}

class SoftUART softUART(rxBit, txBit, &txBuffer, &rxBuffer, debug);

void setup() {
  
  rxBuffer.initBuffer();
  txBuffer.initBuffer();
  pinMode(PC13, OUTPUT);
  pinMode(PC14, OUTPUT);
  pinMode(PC15, OUTPUT);
  digitalWrite (PC15, 0);
  pwmtimer.pause();
  period = 40; // PWM period in useconds, freq 4Hz
  maxduty = pwmtimer.setPeriod(period);
  pwmtimer.refresh();
  pwmtimer.resume();
  pinMode(pwmOutPin, PWM);
  pinMode(PA1, PWM);
  pwmWrite(PA1, 1800); // 1000 = 21mA 1900 = 43 mA 
  pinMode(PA2, OUTPUT);
  pinMode(PA6, INPUT);
  pinMode(PA4, INPUT_ANALOG);
  pinMode(PA5, INPUT_ANALOG);
  Serial1.begin(1200);
  digitalWrite(PA2,HIGH);
  

      // Pause the timer while we're configuring it
    timer.pause();

    // Set up period
    timer.setPeriod(LED_RATE); // in microseconds

    // Set up an interrupt on channel 1
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(handler_led);

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();  
}

volatile int threshold = 0;


volatile int sampleCounter=0;

void handler_led(void) {
   sampleCounter++;
   if (sampleCounter == 8) {
    sampleCounter=0;
    softUART.baudotTransmitStateMachine(); 
   }
   softUART.baudotReceiveStateMachine();
}


void loop() {
  int out, rxChar;
  duty += 1 * (2700-analogRead(PA5));
  if (duty > 2400) {
    duty=2400;
  }
  if (duty < 0) {
    duty=0;
  }

  pwmWrite(pwmOutPin, duty); // 0<=duty<=maxduty
  
  if (!txBuffer.isBufferFull()) {
    if (shiftMode) {
       out = baudotCodec.asciiToBaudot(ch, &shiftMode);
       txBuffer.writeBuffer(out);
    } else {
      if (Serial1.available()) {
        ch = Serial1.read();
        out = baudotCodec.asciiToBaudot(ch, &shiftMode);
        txBuffer.writeBuffer(out);  
      }
    }
  }
  if (!rxBuffer.isBufferEmpty()) {
    rxChar = rxBuffer.readBuffer();
    out = baudotCodec.baudotToAscii(rxChar);
    if (out != -1) {
      Serial1.write(out);
    }
  }
  
}
