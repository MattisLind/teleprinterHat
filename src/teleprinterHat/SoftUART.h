
typedef char (*readFuncType)();
typedef void (*writeFuncType)(char);

class SoftUART {
  readFuncType  rxBit;
  writeFuncType txBit;
  class RingBuffer * txBuffer;
  class RingBuffer * rxBuffer;
  int rxState;
  int sampleCounter;
  int rxDataByte; 
  int bitCnt;
  int baudotTxState;
  int lastChWasCr;
  char txCh;
 public:
  SoftUART(readFuncType readFunc, writeFuncType writeFunc, class RingBuffer * txB, class RingBuffer * rxB);
  void poll();
  void baudotTransmitStateMachine ();
  void baudotReceiveStateMachine ();
};
