class BaudotCodec {
  int letterState;
  int doAgain;
  static const char  figuresTable[];
  static const char lettersTable[]; 
  char handleLetterFigure(char, int, int *);
 public:
  char asciiToBaudot(char, int *);
  char baudotToAscii(char);
  BaudotCodec();
};
