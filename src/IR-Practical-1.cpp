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
//     printf("Document %s ", word);
    currentDoc = string(word);
    readWord(lenf, word);
    stod << word;
    stod >> len;
    stod.clear();
//     printf("has length %g \n", len);
    docLen[currentDoc] = len;
  }
  fclose(lenf);
}


void IRSystem::checkIndex() {
  /*int i = dfIndex["alphabeta"];
  vector<docTF> v = tfIndex["alphabeta"];
//   docTF tf = v.at(0);
  
  printf("%i \n", i);
  printf("v range: %i \n", v.size());
//   if (v == NULL) printf("v = NULL \n");
  printf("(%s, %i) \n", tfIndex["Maker"].at(4).doc->c_str(), tfIndex["Maker"].at(4).tf);*/
  multimap<double, string> test;
  multimap<double, string>::iterator miter;
  
//   test[0.1] = "a";
//   test[0.5] = "e";
//   test[0.7] = "g";
//   test[0.2] = "b";
  test.insert(pair<double,string>(0.1,"a"));
  test.insert(pair<double,string>(0.5,"f"));
  test.insert(pair<double,string>(0.5,"e"));
  test.insert(pair<double,string>(0.7,"g"));
  test.insert(pair<double,string>(0.2,"b"));
  
  for (miter = test.begin(); miter != test.end(); miter++) {
    printf("%s \n", miter->second.c_str());
  }
}

void IRSystem::answerQuery(list<term> query) {
  int df;
  double currentLen;
  double currentIDF;
  document currentDoc;
  vector<docTF> vDTF;
  set<document> seenDocs;
  list<term>::iterator qiter;
  vector<docTF>::iterator viter;
  set<document>::iterator siter;
  multimap<double, document>::reverse_iterator riter;
  
  for (qiter = query.begin(); qiter != query.end(); qiter++) {
    df = dfIndex[*qiter];
    if (df > 0) {
      currentIDF = 1./((double) df);
      vDTF = tfIndex[*qiter];
      for (viter = vDTF.begin(); viter != vDTF.end(); viter++) {
	currentDoc = *(viter->doc);
        seenDocs.insert(currentDoc);
// 	if (currentDoc == "XIE19981228.0221")
// 	  printf("hans: %i, %g, %g \n", viter->tf, currentIDF, docLen[currentDoc]);
	docScore[currentDoc] += ((double)viter->tf)*currentIDF/docLen[currentDoc];
      }
    } else {
      currentIDF = 0.;   // ignore terms that aren't indexed
    }
    
    printf("%g \n", currentIDF);
  }
  
  for (siter = seenDocs.begin(); siter != seenDocs.end(); siter++) {
    currentDoc = *siter;
    ranking.insert(pair<double,document>(docScore[currentDoc], currentDoc));
  }
  
  printf("ranked list of candidates (%i): \n", seenDocs.size());
  for (riter = ranking.rbegin(); riter != ranking.rend(); riter++) {
    printf("%s [%g] \n", riter->second.c_str(), riter->first);
  }
}


int main(int argc, const char** argv) {
  int i;
  list<term> query;
  IRSystem *irs = new IRSystem();
  
//   printf("%i, \'%s\' \n", argc, argv[0]);
  if (argc == 1) {
    query.push_back("financial");
    query.push_back("instruments");
    query.push_back("being");
    query.push_back("traded");
    query.push_back("on");
    query.push_back("the");
    query.push_back("American");
    query.push_back("stock");
    query.push_back("exchange");
  } else {
    for (i = 1; i < argc; i++) {
      query.push_back(string(argv[i]));
    }
  }
  
  irs->readIndex("data/index.txt");
  irs->readDocLengths("data/doc_lengths.txt");
//   irs->checkIndex();
  irs->answerQuery(query);

  return 0;
}
