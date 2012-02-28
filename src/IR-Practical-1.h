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

// both documents and terms are identified by strings.
typedef string document;
typedef string term;

const int BUFFER_SIZE = 1024;

// pair of document and term frequency. This is part of a map from terms on doc/tf pairs 
// defining the term frequency in the given document.
typedef struct docTF {
  document *doc;
  int tf;
} docTF;

class IRSystem {
public:
  // reads a string that lies between two spaces into the word-buffer. The reading buffer may be re-filled.
  int  readWord(FILE* file, char* word);
  // fills both dfIndex and tfIndex from input file.
  void readIndex(string path);
  // reads tf*idf document lengths from input file and stores values in docLen.
  void readDocLengths(string path);
  // reads relevant docuemnts into relevantDocs.
  void readRelevantDocuments(string path);
//   void checkIndex();
//   void printRankedList();
  // the IRSystem class can store a query internally, the user can still provide a specific query.
  // The internal query allows to apply some query expansion techniques.
  void answerQuery(list<term> query, bool normalise);
  void answerQuery(bool normalise);
  void evaluate(bool print);
  // Adds a term to the internal query.
  void addKeyWord(term word);
  // clears the internal query.
  void clearQuery();
protected:
  // current position in reading buffer. Reading happens character-wise.
  int pos;
  // number of read characters.
  int read;
  // input buffer.
  char buffer[BUFFER_SIZE];
  // list of terms in the query.
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
  
  // increases position pointer in input buffer, may refill the buffer if end is reached.
  void increasePos(FILE* file);
};

#endif /* IR_PRAC_1 */

