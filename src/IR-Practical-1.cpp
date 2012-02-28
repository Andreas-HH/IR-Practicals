#include "IR-Practical-1.h" 


void IRSystem::increasePos(FILE* file) {
  if (pos < read) {
    pos++;
    if (pos == read) {
      read = fread(buffer, sizeof(char), BUFFER_SIZE, file);
      pos = 0;
    }
  }
}

int IRSystem::readWord(FILE *file, char *word) {
  int j = 0;
  
  while (buffer[pos] != ' ' && buffer[pos] != '\n') {
    word[j++] = buffer[pos];
    increasePos(file);
  }
  word[j] = '\0';
  increasePos(file);
    
  return 0;
}

void IRSystem::readIndex(string path) {
  int i;
//   int pos = 1; // there is a blank symbol at the start of each line
  int currentDF;
  char word[64];
  stringstream stoi(stringstream::in | stringstream::out);
  term *currentTerm;
  docTF *dtf;
  FILE *indexf = fopen(path.c_str(), "r");
  vector<docTF> freqVec;
  
  pos = 1; // there is a blank at the beginning of each line
  read = fread(buffer, sizeof(char), BUFFER_SIZE, indexf);
  while (pos != read) {
    // reading term
    readWord(indexf, word);
    currentTerm = new string(word);
    // reading document frequency
    readWord(indexf, word);
    stoi << word;
    stoi >> currentDF;
    stoi.clear();
    dfIndex[*currentTerm] = currentDF;
    freqVec = vector<docTF>(currentDF);
    // reading term frequencies
    for (i = 0; i < currentDF; i++) {
      dtf = (docTF*) malloc(sizeof(docTF));
      readWord(indexf, word);
      dtf->doc = new string(word);
      readWord(indexf, word);
      stoi << word;
      stoi >> dtf->tf; // dtf->tf
      stoi.clear();
      freqVec[i] = *dtf;
    }
    tfIndex[*currentTerm] = freqVec;
    increasePos(indexf); // at the end of each line separated by '\n' and ' '
  }
  fclose(indexf);
}

void IRSystem::readDocLengths(string path) {
  char word[64];
  double len;
  document currentDoc;
  stringstream stod(stringstream::in | stringstream::out);
  FILE *lenf = fopen(path.c_str(), "r");
  
  pos = 0;  // no blank symbol in this file
  read = fread(buffer, sizeof(char), BUFFER_SIZE, lenf);
  while (pos != read) {
    readWord(lenf, word);
    currentDoc = string(word);
    readWord(lenf, word);
    stod << word;
    stod >> len;
    stod.clear();
    docLen[currentDoc] = len;
  }
  fclose(lenf);
}

void IRSystem::readRelevantDocuments(string path) {
  char word[64];
  FILE *relf = fopen(path.c_str(), "r");
  
  pos = 0;  // no blank symbol in this file
  read = fread(buffer, sizeof(char), BUFFER_SIZE, relf);
  while (pos != read) {
    readWord(relf, word);
    relevantDocs.insert(string(word));
  }
  fclose(relf);
}

void IRSystem::answerQuery(bool normalise) {
  answerQuery(query, normalise);
}

void IRSystem::answerQuery(set<term> query, bool normalise) {
  int df;
  double currentLen;
  double currentIDF;
  document currentDoc;
  vector<docTF> vDTF;
  set<term>::iterator qiter;
  vector<docTF>::iterator viter;
  set<document>::iterator siter;
  
  for (qiter = query.begin(); qiter != query.end(); qiter++) {
    df = dfIndex[*qiter];
    if (df > 0) {
      currentIDF = 1./((double) df);
      vDTF = tfIndex[*qiter];
      for (viter = vDTF.begin(); viter != vDTF.end(); viter++) {
	currentDoc = *(viter->doc);
        seenDocs.insert(currentDoc);
	if (normalise)
	  docScore[currentDoc] += ((double)viter->tf)*currentIDF/docLen[currentDoc];
	else
	  docScore[currentDoc] += ((double)viter->tf)*currentIDF;
      }
    } else {
      currentIDF = 0.;   // ignore terms that aren't indexed
    }
  }
  
  for (siter = seenDocs.begin(); siter != seenDocs.end(); siter++) {
    currentDoc = *siter;
    ranking.insert(pair<double,document>(docScore[currentDoc], currentDoc));
  }
}

void IRSystem::evaluate(bool print) {
  int num = 0; // number of seen documents
  int foundDocs = 0; // number of seen relevant documents
  double threshold = 0.1; // interval with to output recall->precision value
  double lastRecall = 0.; // recall seen in the last iteration, used for interpolation
  double dp, dr; // delta precision, recall
  double numRel = (double) relevantDocs.size(); // number of relevant documents
  double precision, recall;
  double average = 0;
  double numRecall;
  multimap< double, document >::reverse_iterator riter; // go from high to low score values
  map< double, double >::iterator rpiter; // iterator through nice recall->precision table
  
  if (print) printf("Recall --> Precision \n");
  for (riter = ranking.rbegin(); riter != ranking.rend(); riter++) {
    num++;
    if (relevantDocs.find(riter->second) != relevantDocs.end()) { // contained
      foundDocs++;
      precision = ((double) foundDocs)/((double) num);
      recall = ((double) foundDocs)/numRel;
      recallOnPrecision[recall] = precision;
      if (recall >= threshold) {
	dp = precision - recallOnPrecision[lastRecall];
	dr = recall - lastRecall;  // should be constant actually
	// do the linear interpolation
	niceROP[threshold] = recallOnPrecision[lastRecall] + (threshold - lastRecall) * (dp/dr);
	if (print) printf("%g --> %g \n", threshold, niceROP[threshold]);
	threshold += 0.1;
      }
      lastRecall = recall;
    }
  }
  
  // compute average
  numRecall = (double) recallOnPrecision.size();
  for (rpiter = niceROP.begin(); rpiter != niceROP.end(); rpiter++) { // recallOnPrecision
    average += (rpiter->second)/niceROP.size();
  }
  
  printf("average: %g \n", average);
}

void IRSystem::clearEvaluation() {
  docScore.clear();
  seenDocs.clear();
  ranking.clear();
  recallOnPrecision.clear();
}

void IRSystem::clearQuery() {
  query.clear();
  clearEvaluation();
}

// adds the keyword plus upper/lower case expansions to the internal query.
// sets remove duplicates autmatically
void IRSystem::addKeyWord(term word) {
  int i;
  char altWord[word.size()];
  const char *input = word.c_str();
  
  altWord[0] = toupper(input[0]);
  for (i = 1; input[i] != '\0'; i++) {
    altWord[i] = input[i];
  }
  altWord[i] = '\0';
//   printf("Adding as well: \'%s\' \n", altWord);
  query.insert(string(altWord));
  for (i = 0; input[i] != '\0'; i++) {
    altWord[i] = toupper(input[i]);
  }
  altWord[i] = '\0';
//   printf("Adding as well: \'%s\' \n", altWord);
  query.insert(string(altWord));
  for (i = 0; input[i] != '\0'; i++) {
    altWord[i] = tolower(input[i]);
  }
  altWord[i] = '\0';
//   printf("Adding as well: \'%s\' \n", altWord);
  query.insert(string(altWord));
  
  query.insert(word);
}

int main(int argc, const char** argv) {
  int i;
  set<term> query1;
  set<term> query2;
  IRSystem *irs = new IRSystem();
  
  // the user is allowed to give a custom query as argument
  // evaluation is pointless in that case
  if (argc == 1) {
    query1.insert("financial");
    query1.insert("instruments");
    query1.insert("being");
    query1.insert("traded");
    query1.insert("on");
    query1.insert("the");
    query1.insert("American");
    query1.insert("stock");
    query1.insert("exchange");
    
    query2.insert("stocks");
    query2.insert("shares");
    query2.insert("stock");
    query2.insert("market");
    query2.insert("exchange");
    query2.insert("New");
    query2.insert("York");
    query2.insert("traded");
    query2.insert("trading");
  } else {
    for (i = 1; i < argc; i++) {
      query1.insert(string(argv[i]));
    }
  }
  
  irs->readIndex("data/index.txt");
  irs->readDocLengths("data/doc_lengths.txt");
  irs->readRelevantDocuments("data/relevant.txt");

//   printf("Query 1: ");
  irs->addKeyWord("financial");
  irs->addKeyWord("instruments");
  irs->addKeyWord("being");
  irs->addKeyWord("traded");
  irs->addKeyWord("on");
  irs->addKeyWord("the");
  irs->addKeyWord("American");
  irs->addKeyWord("stock");
  irs->addKeyWord("exchange");
  printf("Query 1 with normalisation and with query expansion: \n");
  irs->answerQuery(true);
  irs->evaluate(true);
  irs->clearEvaluation();
  printf("\n");
  printf("Query 1 without normalisation and with query expansion: \n");
  irs->answerQuery(false);
  irs->evaluate(true);
  irs->clearQuery();
  printf("\n");
  
  printf("Query 1 with normalisation and without query expansion: \n");
  irs->answerQuery(query1, true);
  irs->evaluate(true);
  irs->clearEvaluation();
  printf("\n");
  printf("Query 1 without normalisation and without query expansion: \n");
  irs->answerQuery(query1, false);
  irs->evaluate(true);
  irs->clearQuery();
  printf("\n");
  
//   printf("Query 2: ");
  irs->addKeyWord("stocks");
  irs->addKeyWord("shares");
  irs->addKeyWord("stock");
  irs->addKeyWord("market");
  irs->addKeyWord("exchange");
  irs->addKeyWord("New");
  irs->addKeyWord("York");
  irs->addKeyWord("traded");
  irs->addKeyWord("trading");
  printf("Query 2 with normalisation and with query expansion: \n");
  irs->answerQuery(true);
  irs->evaluate(true);
  irs->clearEvaluation();
  printf("\n");
  printf("Query 2 without normalisation and with query expansion: \n");
  irs->answerQuery(false);
  irs->evaluate(true);
  irs->clearQuery();
  printf("\n");
  printf("Query 2 with normalisation and without query expansion: \n");
  irs->answerQuery(query2, true);
  irs->evaluate(true);
  irs->clearEvaluation();
  printf("\n");
  printf("Query 2 without normalisation and without query expansion: \n");
  irs->answerQuery(query2, false);
  irs->evaluate(true);
  irs->clearEvaluation();
  printf("\n");
  

  return 0;
}
