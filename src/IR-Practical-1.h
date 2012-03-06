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
#include <math.h>

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
  // fills both dfIndex and tfIndex from input file.
  void readIndex(string path);
  // reads tf*idf document lengths from input file and stores values in docLen.
  void readDocLengths(string path);
  // reads relevant docuemnts into relevantDocs.
  void readRelevantDocuments(string path);
  // reads relevant documents ignoring those used for positive feedback.
  // could be constructed as well but we are given the file, so lets use it.
  void readRelevantDocumentsNoFeedback(string path);
  // fills the sets posFeedback and negFeedback.
  void readFeedback(string path);
  // the IRSystem class can store a query internally, the user can still provide a specific query.
  // The internal query allows to apply some query expansion techniques.
  void answerQuery(set<term> query, double scale = 1., bool rank = true);
  void answerQuery(double scale = 1., bool rank = true);
  // this should be called after answerQuery() to add the use of feedback. Otherwise it will
  // ignore the original query.
  void applyFeedback(double alpha, double beta, double gamma);
  // constructs the recall->presicision mapping.
  void evaluate(bool print);
  // constructs the recall->presicision mapping after feedback has been used.
  void evaluateFeedback(bool print);
  // Adds a term to the internal query.
  void addKeyWord(term word);
  // clears the internal query.
  void clearQuery();
  void clearEvaluation();
  void setNormalise(bool n);
protected:
  // current position in reading buffer. Reading happens character-wise.
  int pos;
  // number of read characters.
  int read;
  bool normalise;
  // input buffer.
  char buffer[BUFFER_SIZE];
  // list of terms in the query.
  set<term>                              query;
  unordered_map< term, int >             dfIndex;
  unordered_map< term, vector< docTF > > tfIndex;
  unordered_map< document, double >      docLen;
  set< document >                        relevantDocs;
  set< document >                        relevantDocsNoFeedback;
  set< document >                        posFeedback;
  set< document >                        negFeedback;
  
  unordered_map< document, double >      docScore;
  set< document >                        seenDocs;
  multimap< double, document >           ranking;  // c++ keeps maps sortet wrt. their keys
  map < double, double >                 recallOnPrecision;
  map < double, double >                 niceROP;
  
  // reads a string that lies between two spaces into the word-buffer. The reading buffer may be re-filled.
  int  readWord(FILE* file, char* word);
  // increases position pointer in input buffer, may refill the buffer and sets pos to 0 if end is reached.
  void increasePos(FILE* file);
  void readSetOfDocuments(string path, set< document > &set);
  // creates ranking from seenDocs and docScore.
  void fillRanking();
  // allows to switch easyily between relevantDocs and relevantDocsNoFeedback.
  void evaluateInternal(bool print, set< document > &set, bool ignorePosFeedback);
};

#endif /* IR_PRAC_1 */

