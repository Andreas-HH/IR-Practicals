all: IR-Practicals

# need -std=c++0x to use unordered_map rather than map
IR-Practicals: src/IR-Practical-1.h src/IR-Practical-1.cpp
	g++ -std=c++0x src/IR-Practical-1.cpp -o IR-Practicals
