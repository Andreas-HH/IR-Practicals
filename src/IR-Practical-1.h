#ifndef IR_PRAC_1
#define IR_PRAC_1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

using namespace std;

typedef string document;
typedef string term;

const int BUFFER_SIZE = 1024;

typedef struct docTF {
  document *doc;
  int tf;
} docTF;

class IRSystem {
public:
  int readWord(FILE *file, char *buffer, int pos, char *word);
  void readIndex(string path);
protected:
  int read;
  unordered_map<term, int>   dfIndex;
  unordered_map<term, vector<docTF>> tfIndex;
};

#endif /* IR_PRAC_1 */