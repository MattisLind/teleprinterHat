class BaudotCodec {
  int letterState;
  int doAgain;
  const char * figuresTable;
  const char * lettersTable;
  char handleLetterFigure(char, int, int *);
 public:
  char asciiToBaudot(char, int *);
  BaudotCodec(const char *, const char *);
};
