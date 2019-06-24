// Letter  = 1 figures = 0

#include "BaudotCodec.h"
#include <stdio.h>

BaudotCodec::BaudotCodec() {
  letterState=-1;
  //figuresTable = ft; 
  //lettersTable = lt;
}

const char BaudotCodec::lettersTable[] =  {3, 25, 14, 9, 1, 13, 26, 20, 6, 11, 15, 18, 28, 12, 24, 22, 23, 10, 5, 16, 7, 30, 19, 29, 21, 17};;
const char BaudotCodec::figuresTable[] =  {22, 23, 19, 1, 11, 16, 21, 7, 6, 24};

char BaudotCodec::handleLetterFigure(char out, int requestedState, int * doAgain) {
  if (letterState==requestedState) {
    *doAgain = 0;
    printf("doAgain=0 |");
    return out;    
  }
  else {
    if (requestedState==1) {
      *doAgain = 1;
      letterState = 1; 
      printf("doAgain=1 letterState=1 |");
      return 31;
    }
    else {
      *doAgain = 1;
      letterState = 0;
      printf("doAgain=1 letterState=0 |");
      return 27;
    }
  }
}

char BaudotCodec::asciiToBaudot(char ch, int * doAgain) {
  int out, handled;
  if (ch >= 48 && ch <= 57) {
    // Figures
      out = figuresTable[ch-48];
      return handleLetterFigure(out, 0, doAgain); 
  } else if (ch >= 65 && ch <= 90)  { 
    out = lettersTable[ch-65]; // letters
    return handleLetterFigure(out, 1, doAgain); 
    // A-Z letters
  } else if (ch >= 97 && ch <= 122) {
    out = lettersTable[ch-97]; // letters
    return handleLetterFigure(out, 1, doAgain); 
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
      return handleLetterFigure(out, 0, doAgain);
    else 
      return 0;
  }
}


char BaudotCodec::baudotToAscii(char ch) {
  return ch;
}
