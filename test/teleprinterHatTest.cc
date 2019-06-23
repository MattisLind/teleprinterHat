#include <stdio.h>
#include <string.h>

int shiftMode = 0;
int state = -1;
char ch;

#define SIZE 16

class RingBuffer {
  volatile int inPtr, outPtr, bufferFull, bufferEmpty;
  int buffer[SIZE];
public:
  int isBufferFull () {
    return bufferFull;
  };
  int isBufferEmpty () {
    return bufferEmpty;
  }
  void writeBuffer (int data);
  int readBuffer ();
  void  initBuffer ();
};







void RingBuffer::writeBuffer (int data) {
  printf ("write data %X |", data);
  buffer[inPtr] = data;
  bufferEmpty = 0;
  inPtr++;
  if (inPtr == SIZE) {
    inPtr = 0;
  }
  if (inPtr == outPtr) {
    bufferFull = 1;
  } else {
    bufferFull = 0;
  }
}

int RingBuffer::readBuffer () {
  int out = buffer[outPtr];
  bufferFull = 0;
  outPtr++;
  if (outPtr == SIZE) {
    outPtr = 0;
  }
  if (inPtr == outPtr) {
    bufferEmpty = 1;
  } else {
    bufferEmpty = 0;
  }
  return out;
}

void RingBuffer::initBuffer () {
  bufferEmpty = 1;
  bufferFull = 0;
  inPtr = 0;
  outPtr = 0;
}


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


int baudotTxState = 0;
int lastChWasCr = 0;
char txCh;


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

int rxState=0;
int sampleCounter = 0;
int rxDataByte; 
int bitCnt=0;

void baudotReceiveStateMachine () {
  char bit;
  printf("baudotReceiveStateMachine BEGIN, rxState=%d bitCnt=%d, sampleCounter=%d rxDataByte=%02X", rxState, bitCnt, sampleCounter,rxDataByte);
  switch (rxState) {
    case 0:   // Searching fo start bit 
      if (rxBit() == 0) {
	rxState = 1;
      }
      break;
    case 1:  // Qualify start bit has to be 7 in a row.
      if (rxBit() == 0) {
	// Still a statbit. Need to count to 7 to find the middle of the start bit!
	if (sampleCounter >= 6) {
	  rxState = 2;
	  sampleCounter = 0;
	  rxDataByte = 0;
	  bitCnt=0;
	} else {
	  sampleCounter ++;
	}
      } else {
	rxState = 0; // go back if a marking state detected again. False start bit !
      }
      break;
    case 2: // Sampling in the middle of the bit - first bit!
      bit = rxBit();
      if (sampleCounter >= 15) {
	rxDataByte = rxDataByte << 1;
	rxDataByte |= 1 & bit;
	bitCnt++;
	if (bitCnt==5) {
	  rxState=3;
	}
	sampleCounter=0;
      } else {
	sampleCounter++;
      }
      break;
    case 3: // 1.5 stop bits
      rxBit();
      if (sampleCounter < 22) {
	sampleCounter++;
      } else {
	rxState = 0;
	rxBuffer.writeBuffer(rxDataByte);
      }
  }
}


void baudotTransmitStateMachine ()
{
  switch (baudotTxState) {
  case 0: // First half stop bit
    if (txBuffer.isBufferEmpty()) {
      return;
    }
    if (!lastChWasCr) { // make sure we send CR twice..
      txCh = txBuffer.readBuffer();
      if (txCh==2) lastChWasCr = 1;
    }
    else {
      lastChWasCr = 0;
    }
    txBit(0);
    baudotTxState++;
    break;
  case 1:
    txBit(0);
    baudotTxState++;
    break;
  case 2:
  case 3:
    txBit(txCh >> 4);
    baudotTxState++;
    break;
  case 4:
  case 5:
    txBit(txCh >> 3);
    baudotTxState++;
    break;
  case 6:
  case 7:
    txBit(txCh >> 2);
    baudotTxState++;
    break;
  case 8:
  case 9:
    txBit(txCh >> 1);
    baudotTxState++;
    break;
  case 10:
  case 11:
    txBit(txCh >> 0);
    baudotTxState++;
    break;
  case 12:
  case 13:
    txBit(1);
    baudotTxState++;
    break;
  case 14:
    txBit(1);
    baudotTxState=0;
    break;

  }

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
    baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    baudotTransmitStateMachine();
  }
  printf("\n");
  loop();
  printf("TX:");
  for (i=0;i<15; i++) {
    baudotTransmitStateMachine();
  }
  printf("\n");
  printf("Databits: ");
  for (i=0; i< 60; i++) {
    printf("%d ", txData[i]);
  }
  printf("\n");
  for (i=0; i< 60*8; i++) {
    baudotReceiveStateMachine();
  }
  return 0;
}
