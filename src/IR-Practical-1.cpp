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
  
  pos = 1;
  read = fread(buffer, sizeof(char), BUFFER_SIZE, indexf);
  while (pos != read) {
    readWord(indexf, word);
    currentTerm = new string(word);
    readWord(indexf, word);
    stoi << word;
    stoi >> currentDF;
    stoi.clear();
    dfIndex[*currentTerm] = currentDF;
    freqVec = vector<docTF>(currentDF);
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


// void IRSystem::checkIndex() {
//   /*int i = dfIndex["alphabeta"];
//   vector<docTF> v = tfIndex["alphabeta"];
// //   docTF tf = v.at(0);
//   
//   printf("%i \n", i);
//   printf("v range: %i \n", v.size());
// //   if (v == NULL) printf("v = NULL \n");
//   printf("(%s, %i) \n", tfIndex["Maker"].at(4).doc->c_str(), tfIndex["Maker"].at(4).tf);*/
//   multimap<double, string> test;
//   multimap<double, string>::iterator miter;
//   
// //   test[0.1] = "a";
// //   test[0.5] = "e";
// //   test[0.7] = "g";
// //   test[0.2] = "b";
//   test.insert(pair<double,string>(0.1,"a"));
//   test.insert(pair<double,string>(0.5,"f"));
//   test.insert(pair<double,string>(0.5,"e"));
//   test.insert(pair<double,string>(0.7,"g"));
//   test.insert(pair<double,string>(0.2,"b"));
//   
//   for (miter = test.begin(); miter != test.end(); miter++) {
//     printf("%s \n", miter->second.c_str());
//   }
// }

void IRSystem::answerQuery(bool normalise) {
  answerQuery(query, normalise);
}

void IRSystem::answerQuery(list<term> query, bool normalise) {
  int df;
  double currentLen;
  double currentIDF;
  document currentDoc;
  vector<docTF> vDTF;
  list<term>::iterator qiter;
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
  int num = 0;
  int foundDocs = 0;
  double threshold = 0.1;
  double lastRecall = 0.;
  double dp, dr;
  double numRel = (double) relevantDocs.size();
  double precision, recall;
  double average = 0;
  double numRecall;
  multimap< double, document >::reverse_iterator riter;
  map< double, double >::iterator rpiter;
  
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

// void IRSystem::printRankedList() {
//   int num = 0;
//   int foundDocs = 0;
//   double numRel = (double) relevantDocs.size();
//   double precision, recall;
//   multimap<double, document>::reverse_iterator riter;
//   
//   printf("ranked list of candidates (%i): \n", seenDocs.size());
//   printf("name [score] [Precision] [Recall] \n");
//   for (riter = ranking.rbegin(); riter != ranking.rend(); riter++) {
//     num++;
//     if (relevantDocs.find(riter->second) != relevantDocs.end()) { // contained
//       foundDocs++;
//     }
//     precision = ((double) foundDocs)/((double) num);
//     recall = ((double) foundDocs)/numRel;
//     printf("%s \t [%g] \t [%g] \t [%g] \n", riter->second.c_str(), riter->first, precision, recall);
//   }
// }

void IRSystem::clearQuery() {
  query.clear();
  docScore.clear();
  seenDocs.clear();
  ranking.clear();
  recallOnPrecision.clear();
  printf("\n\n");
}

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
//   query.push_back(string(altWord));
  for (i = 0; input[i] != '\0'; i++) {
    altWord[i] = toupper(input[i]);
  }
  altWord[i] = '\0';
//   printf("Adding as well: \'%s\' \n", altWord);
//   query.push_back(string(altWord));
  
  query.push_back(word);
}

int main(int argc, const char** argv) {
  int i;
  list<term> query1;
  list<term> query2;
  IRSystem *irs = new IRSystem();
  
//   if (argc == 1) {
//     query1.push_back("financial");
//     query1.push_back("instruments");
//     query1.push_back("being");
//     query1.push_back("traded");
//     query1.push_back("on");
//     query1.push_back("the");
//     query1.push_back("American");
//     query1.push_back("stock");
//     query1.push_back("exchange");
//     
//     query2.push_back("stocks");
//     query2.push_back("shares");
//     query2.push_back("stock");
//     query2.push_back("market");
//     query2.push_back("exchange");
//     query2.push_back("New");
//     query2.push_back("York");
//     query2.push_back("traded");
//     query2.push_back("trading");
//   } else {
//     for (i = 1; i < argc; i++) {
//       query1.push_back(string(argv[i]));
//     }
//   }
  
  irs->readIndex("data/index.txt");
  irs->readDocLengths("data/doc_lengths.txt");
  irs->readRelevantDocuments("data/relevant.txt");
//   irs->checkIndex();
  printf("Query 1: ");
  irs->addKeyWord("financial");
  irs->addKeyWord("instruments");
  irs->addKeyWord("being");
  irs->addKeyWord("traded");
  irs->addKeyWord("on");
  irs->addKeyWord("the");
  irs->addKeyWord("American");
  irs->addKeyWord("stock");
  irs->addKeyWord("exchange");
  irs->answerQuery(false);
  irs->evaluate(true);
  irs->clearQuery();
  
  printf("Query 2: ");
  irs->addKeyWord("stocks");
  irs->addKeyWord("shares");
  irs->addKeyWord("stock");
  irs->addKeyWord("market");
  irs->addKeyWord("exchange");
  irs->addKeyWord("New");
  irs->addKeyWord("York");
  irs->addKeyWord("traded");
  irs->addKeyWord("trading");
  irs->answerQuery(false);
  irs->evaluate(true);

  return 0;
}
