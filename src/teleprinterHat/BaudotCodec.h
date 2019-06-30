class BaudotCodec {
  int letterState;
  int rxState;
  int doAgain;
  int lowerCase;
  static const char  figuresTable[];
  static const char  lettersTable[]; 
  static const char  rxFiguresTable[];
  static const char  rxLettersTable[]; 
  char handleLetterFigure(char, int, int *);
 public:
  char asciiToBaudot(char, int *);
  int baudotToAscii(char);
  BaudotCodec(int);
};
