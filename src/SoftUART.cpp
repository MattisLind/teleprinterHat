#include "SoftUART.h"
#include "RingBuffer.h"
#include <stdio.h>


SoftUART::SoftUART(readFuncType readFunc, writeFuncType writeFunc, class RingBuffer * txB, class RingBuffer * rxB) {
  rxBit=readFunc;
  txBit=writeFunc;
  txBuffer = txB;
  rxBuffer = rxB;
  rxState=0;
  sampleCounter = 0;
  bitCnt=0;
  baudotTxState = 0;
  lastChWasCr = 0;
}

void SoftUART::baudotReceiveStateMachine () {
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
	rxBuffer->writeBuffer(rxDataByte);
      }
  }
}


void SoftUART::baudotTransmitStateMachine ()
{
  switch (baudotTxState) {
  case 0: // First half stop bit
    if (txBuffer->isBufferEmpty()) {
      return;
    }
    if (!lastChWasCr) { // make sure we send CR twice..
      txCh = txBuffer->readBuffer();
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

void SoftUART::poll() {

}
