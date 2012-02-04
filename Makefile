all: IR-Practical-1

# need -std=c++0x to use unordered_map rather than map
IR-Practical-1: src/IR-Practical-1.h src/IR-Practical-1.cpp
	g++ -std=c++0x src/IR-Practical-1.cpp -o IR-Practical-1
