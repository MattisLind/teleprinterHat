#include <stdio.h>
#include <string.h>
#include "../src/RingBuffer.h"
#include "../src/SoftUART.h"

int shiftMode = 0;
int state = -1;
char ch;


class RingBuffer txBuffer;
class RingBuffer rxBuffer;

const char * ryMsg = "RYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRY\r\n";

char figuresTable[] = {22, 23, 19, 1, 11, 16, 21, 7, 6, 24};
char lettersTable[] = {3, 25, 14, 9, 1, 13, 26, 20, 6, 11, 15, 18, 28, 12, 24, 22, 23, 10, 5, 16, 7, 30, 19, 29, 21, 17};


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


// Letter  = 1 figures = 0

int handleLetterFigure(char out, int * letterState, int requestedState, int * doAgain) {
  if (*letterState==requestedState) {
    *doAgain = 0;
    printf("doAgain=0 |");
    return out;    
  }
  else {
    if (requestedState==1) {
      *doAgain = 1;
      *letterState = 1; 
      printf("doAgain=1 letterState=1 |");
      return 31;
    }
    else {
      *doAgain = 1;
      *letterState = 0;
      printf("doAgain=1 letterState=0 |");
      return 27;
    }
  }
}

int asciiToBaudot(char ch, int * letterState, int * doAgain) {
  int out, handled;
  if (ch >= 48 && ch <= 57) {
    // Figures
      out = figuresTable[ch-48];
      return handleLetterFigure(out, letterState, 0, doAgain); 
  } else if (ch >= 65 && ch <= 90)  { 
    out = lettersTable[ch-65]; // letters
    return handleLetterFigure(out, letterState, 1, doAgain); 
    // A-Z letters
  } else if (ch >= 97 && ch <= 122) {
    out = lettersTable[ch-97]; // letters
    return handleLetterFigure(out, letterState, 1, doAgain); 
    // a-z letter
  } else if (ch == ' ') {
    return 4;
  } else if (ch == '\r') {
    return 2;
  } else if (ch == '\n') {
    return 8;
  } else if (ch == '\0') {
    return 0;
  } else {
    handled = true;
    switch (ch) {
      case ',':
        out = 12;
        break;
      case '.':
        out = 28;       
        break;
      case ':': 
        out = 14;
        break;
      case ';':
        out = 30;
        break;
      case '\"':
        out = 17;
        break;
      case '!': 
        out = 13;
        break;
      case '/':
        out = 29;
        break;
      case '-':
        out = 3;
        break;
      case '\'':
        out = 10;
        break;
      case '(': 
        out = 15;
        break;
      case ')':
        out = 18;
        break;
      case '&':
        out = 26;
        break;
      case '#':
        out = 20;
        break;
      case '?':
	out = 25;
	break;
      default:
        handled = false;
	break;
    }
    if (handled)
      return handleLetterFigure(out, letterState, 0, doAgain);
    else 
      return 0;
  }

  
}


volatile char chCnt =0;
volatile int chState;



void loop() {
  char out;
  printf("before checking buffer full | ");
  if (!txBuffer.isBufferFull()) {
    printf("buffer not full |");
    if (shiftMode) {
       printf("shift mode |");
       out = asciiToBaudot(ch, &state, &shiftMode);
       txBuffer.writeBuffer(out);
    } else {
       printf("not shift mode | ");
      if (Serial1.available()) {
        ch = Serial1.read();
	printf("got a character %c |", ch);
        out = asciiToBaudot(ch, &state, &shiftMode);
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
