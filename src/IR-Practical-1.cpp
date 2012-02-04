#include "IR-Practical-1.h" 


int IRSystem::readWord(FILE *file, char *buffer, int pos, char *word) {
  int i = pos, j = 0;
  
  while (buffer[i] != ' ' && buffer[i] != '\n') {
    word[j++] = buffer[i++];
    if (i == BUFFER_SIZE) {
      read = fread(buffer, sizeof(char), BUFFER_SIZE, file);
      if (read < 0) return -1;
      i = 0;
    }
  }
  word[j] = '\0';
  i++;
  if (i == BUFFER_SIZE) {
    i = 0;
    read = fread(buffer, sizeof(char), BUFFER_SIZE, file);
  }
    
  return i;
}


void IRSystem::readIndex(string path) {
  int i;
  int pos = 1; // there is a blank symbol at the start of each line
  int currentDF;
  char buffer[BUFFER_SIZE];
  char word[64];
  stringstream stoi(stringstream::in | stringstream::out);
  term *currentTerm;
  docTF *dtf;
  FILE *indexf = fopen(path.c_str(), "r");
  vector<docTF> freqVec;
  
  read = fread(buffer, sizeof(char), BUFFER_SIZE, indexf);
  while (read == BUFFER_SIZE) {
    pos = readWord(indexf, buffer, pos, word);
    currentTerm = new string(word);
    pos = readWord(indexf, buffer, pos, word);
    stoi << word;
    stoi >> currentDF;
    stoi.clear();
    dfIndex[*currentTerm] = currentDF;
    freqVec = vector<docTF>(currentDF);
    for (i = 0; i < currentDF; i++) {
      dtf = (docTF*) malloc(sizeof(docTF));
      pos = readWord(indexf, buffer, pos, word);
      dtf->doc = new string(word);
      pos = readWord(indexf, buffer, pos, word);
      stoi << word;
      stoi >> dtf->tf; // dtf->tf
      stoi.clear();
      freqVec[i] = *dtf;
    }
    tfIndex[*currentTerm] = freqVec;
    pos++;
    if (pos == BUFFER_SIZE) {
      pos = 0;
      read = fread(buffer, sizeof(char), BUFFER_SIZE, indexf);
    }
  }
//   printf("(%s, %i) \n", tfIndex["Maker"].at(4).doc->c_str(), tfIndex["Maker"].at(4).tf);
  
  fclose(indexf);
}

int main(int argc, const char** argv) {
  IRSystem *irs = new IRSystem();
  irs->readIndex("data/index.txt");
  
//   irs->writeStuff();
//   irs->readStuff();
  return 0;
}
