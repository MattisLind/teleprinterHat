#define LED_RATE 10000    // in microseconds; should give 0.5Hz toggles

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

void setup() {
  pinMode(PC13, OUTPUT);
  pwmtimer.pause();
  period = 40; // PWM period in useconds, freq 4Hz
  maxduty = pwmtimer.setPeriod(period);
  pwmtimer.refresh();
  pwmtimer.resume();
  pinMode(pwmOutPin, PWM);
  pinMode(PA1, PWM);
  pwmWrite(PA1, 1800); // 1000 = 21mA 1900 = 43 mA 
  pinMode(PA2, OUTPUT);
  pinMode(PA4, INPUT_ANALOG);
  pinMode(PA5, INPUT_ANALOG);
  Serial1.begin(9600);
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
// int bitcnt=0, chcnt=0, ind=0;
//int RY[] =   {0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1};
//int CRLF[] = {0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1};

/*int printch () {
  if (ind == 1) {
    if (CRLF[bitcnt]==1) {
      digitalWrite(PA2,HIGH);
      digitalWrite(PC13,HIGH);
    } else {
      digitalWrite(PA2,LOW);
      digitalWrite(PC13,LOW);
    }
  } else {
    if (RY[bitcnt] == 1) {
      digitalWrite(PA2,HIGH);
      digitalWrite(PC13,HIGH);
    } else {
      digitalWrite(PA2,LOW);
      digitalWrite(PC13,LOW);
    }
  }
  bitcnt++;
  if (bitcnt >= 30) {
    bitcnt = 0;
    return 1;
  }
  return 0;
}

*/

const char * quickMsg = "The quick brown fox jumps over the lazy dog\r\n";
const char * ryMsg = "RYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRY\r\n";

char figuresTable[] = {13, 29, 25, 16, 10, 1, 21, 28, 12, 3};
char lettersTable[] = {24, 19, 14, 18, 16, 22, 11, 5, 12, 26, 30, 9, 7, 6, 3, 13, 29, 10, 20, 1, 28, 15, 25, 23, 21, 17};

int handleLetterFigure(char out, int * letterState, int requestedState, int * doAgain) {
  if (*letterState==requestedState) {
    *doAgain = 0;
    return out;
    
  }
  else {
    if (letterState) {
       // we are in letter - need to send figure
      *doAgain = 1;
      *letterState = 1; 
      return 27;
    }
    else {
      *doAgain = 1;
      *letterState = 0;
      return 31;
    }
  }
}

int asciiToBaudot(char ch, int * letterState, int * doAgain) {
  int out;
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
    return 0;
  } else if (ch == '\r') {
    return 2;
  } else if (ch == '\n') {
    return 8;
  } else {
    switch (ch) {
      case ',':
        out = 6;
        break;
      case '.':
        out = 7;       
        break;
      case ':': 
        out = 14;
        break;
      case ';':
        out = 15;
        break;
      case '\"':
        out = 17;
        break;
      case '!': 
        out = 22;
        break;
      case '/':
        out = 23;
        break;
      case '-':
        out = 24;
        break;
      case '\'':
        out = 26;
        break;
      case '(': 
        out = 30;
        break;
      case ')':
        out = 9;
        break;
    }
    return handleLetterFigure(out, letterState, 0, doAgain);
  }

  
}


volatile char chCnt =0;
volatile int chState;

void handler_led(void) {
/*  if (threshold==0) {
    threshold=2600;
    digitalWrite(PC13,LOW);
  } else {
    threshold=0;
    digitalWrite(PC13,HIGH);
  }
  */
  /*if (chcnt < 0) {
    ind = 1;
    if (printch() == 1) {
      chcnt = 0;
    }
  } else {
    ind = 0;
    if (printch() == 1) {
      chcnt++;
      if (chcnt >= 30) {
        chcnt = -1;
      }
    }
  }*/


  
}

#define SIZE 16

volatile int inPtr, outPtr, bufferFull, bufferEmpty;
int buffer[SIZE];

int isBufferFull () {
  return bufferFull;
}

int isBufferEmpty () {
  return bufferEmpty;
}

int writeBuffer (int data) {
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

int readBuffer () {
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
}

int initBuffer () {
  bufferEmpty = 1;
  bufferFull = 0;
  inPtr = 0;
  outPtr = 0;
}

void loop() {
  int currentVoltage;
  char out;
//mypulse =  23456; // 0<=mypulse<=period, this is the High pulse of my length in useconds
//duty = map((int32)mypulse, 0, (int32)period, 0, (int32)maxduty);

//while(1){
  //digitalWrite(PA2, HIGH);
  //Serial1.print(analogRead(PA4),DEC);
  //Serial1.print(" ");
  //delay(100);
  //digitalWrite(PA2,LOW);
  //Serial1.print(analogRead(PA4),DEC);
  //Serial1.print(" ");
  //delay(100);
  //if (analogRead(PA4) > threshold) {
  //  digitalWrite(PA2,LOW);
  //} else {
  //  digitalWrite(PA2,HIGH);
  //}
  duty += 1 * (2700-analogRead(PA5));
  if (duty > 2400) {
    duty=2400;
  }
  if (duty < 0) {
    duty=0;
  }
  pwmWrite(pwmOutPin, duty); // 0<=duty<=maxduty

  if (!isBufferEmpty()) {
    if (shiftMode) {
       out = asciiToBaudot(ch, &state, &shiftMode);
       writeBuffer(out);
    } else {
      if (Serial1.available()) {
        ch = Serial1.read();
        out = asciiToBaudot(ch, &state, &shiftMode);
        writeBuffer(out);  
      }
    }
  }
  //};
}
