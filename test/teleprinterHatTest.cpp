#include <stdio.h>
#include <string.h>
#include "../src/RingBuffer.h"
#include "../src/SoftUART.h"
#include "../src/BaudotCodec.h"



const char ft [] = {22, 23, 19, 1, 11, 16, 21, 7, 6, 24};
const char lt [] = {3, 25, 14, 9, 1, 13, 26, 20, 6, 11, 15, 18, 28, 12, 24, 22, 23, 10, 5, 16, 7, 30, 19, 29, 21, 17};
class RingBuffer txBuffer;
class RingBuffer rxBuffer;
class BaudotCodec baudotCodec(ft, lt);


class Serial {
  const char * quickMsg;
  int cnt;
 public:
  Serial() : quickMsg("RYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRY\r\n0123456789\r\nABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n-\',!:(\")#?&./;\r\nThe quick brown fox jumps over the lazy dog\r\n!\"#$%&/()=|\\{}[]*~_-:.;,<>"), cnt(0) {}
  int available ();
  char read ();
  void reset() { cnt = 0; };
} Serial1;

int Serial::available () {
  if (cnt < strlen(quickMsg)) {
    return 1;
  } else {
    return 0;
  }

}

char Serial::read() {
  return quickMsg[cnt++];
}




char txData[500];
int txPtr=0;
int rxPtr=0;
int rxCnt=0;
void inline txBit(char data) {
  printf("%d ", data & 1);
  txData[txPtr++]=data&1;
}


char inline rxBit () { 
  char d = txData[rxPtr];
  printf ("rxCnt = %d rxPtr = %d dataBit=%d \n", rxCnt, rxPtr, d);
  if (rxCnt < 7) {
    rxCnt++;
  } else {
    rxCnt=0;
    rxPtr++;
  }

  return d;
}

int shiftMode=0;

char ch;

void loop() {
  char out;
  printf("before checking buffer full | ");
  if (!txBuffer.isBufferFull()) {
    printf("buffer not full |");
    if (shiftMode) {
       printf("shift mode |");
       out = baudotCodec.asciiToBaudot(ch, &shiftMode);
       txBuffer.writeBuffer(out);
    } else {
       printf("not shift mode | ");
      if (Serial1.available()) {
        ch = Serial1.read();
	printf("got a character %c |", ch);
        out = baudotCodec.asciiToBaudot(ch, &shiftMode);
        txBuffer.writeBuffer(out);  
      }
    }
  }
}

int  main () {
  int i;
  char ch;
  class SoftUART softUART(rxBit, txBit, &txBuffer, &rxBuffer);
  rxBuffer.initBuffer();
  txBuffer.initBuffer();
  while (Serial1.available()) {
    loop();
    if (!txBuffer.isBufferEmpty()) {
      ch = txBuffer.readBuffer();
      printf("Read %d\n", ch);
    }
  }
  printf ("buffer is empty: %d\n", txBuffer.isBufferEmpty());
  Serial1.reset();
  for (int i=0; i<SIZE; i++) {
    loop();
    printf ("buffer is full %d |", txBuffer.isBufferFull());
    printf ("buffer is empty: %d \n", txBuffer.isBufferEmpty());

  }
  // Now the buffer should be full!
  printf ("buffer is full %d\n", txBuffer.isBufferFull());
  for (i=0; i<SIZE; i++) {
    ch = txBuffer.readBuffer();
    printf("Read %d | ", ch);
    printf ("buffer is full %d |", txBuffer.isBufferFull());
    printf ("buffer is empty: %d \n", txBuffer.isBufferEmpty());
  }
  printf ("buffer is empty: %d\n", txBuffer.isBufferEmpty());
  
  Serial1.reset();
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    softUART.baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    softUART.baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    softUART.baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    softUART.baudotTransmitStateMachine();
  }
  printf("\n");
  printf("Databits: ");
  for (i=0; i< 60; i++) {
    printf("%d ", txData[i]);
  }
  printf("\n");
  for (i=0; i< 60*8; i++) {
    softUART.baudotReceiveStateMachine();
  }
  return 0;
}
