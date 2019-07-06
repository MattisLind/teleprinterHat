
typedef char (*readFuncType)();
typedef void (*writeFuncType)(char);
typedef void (*debugFuncType)();

class SoftUART {
  readFuncType  rxBit;
  writeFuncType txBit;
  debugFuncType debug;
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
  SoftUART(readFuncType readFunc, writeFuncType writeFunc, class RingBuffer * txB, class RingBuffer * rxB, debugFuncType dbf);
  void poll();
  void baudotTransmitStateMachine ();
  void baudotReceiveStateMachine ();
};
