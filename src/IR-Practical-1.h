#ifndef IR_PRAC_1
#define IR_PRAC_1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <list>
#include <set>

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
  int  readWord(FILE* file, char* word);
  void readIndex(string path);
  void readDocLengths(string path);
  void readRelevantDocuments(string path);
  void checkIndex();
  void printRankedList();
  void answerQuery(list<term> query, bool normalise);
  void answerQuery(bool normalise);
  void evaluate(bool print);
  void addKeyWord(term word);
  void clearQuery();
protected:
  int pos;
  int read;
  char buffer[BUFFER_SIZE];
  list<term>                             query;
  unordered_map< term, int >             dfIndex;
  unordered_map< term, vector< docTF > > tfIndex;
  unordered_map< document, double >      docLen;
  set< document >                        relevantDocs;
  
  unordered_map< document, double >      docScore;
  set< document >                        seenDocs;
  multimap< double, document >           ranking;  // c++ keeps maps sortet wrt. their keys
  map < double, double >                 recallOnPrecision;
  map < double, double >                 niceROP;
  
  void increasePos(FILE* file);
};

#endif /* IR_PRAC_1 */

