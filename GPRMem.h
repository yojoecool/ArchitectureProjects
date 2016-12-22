#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <cstddef>
#include <vector>

using namespace std;

// defines the length of each part of the memory
#define TEXT_LENGTH 10000
#define DATA_LENGTH 10000

// defines memory length
#define MEMORY_LENGTH 20000

//defines where each part of memory starts
#define TEXT_START 0
#define DATA_START 10000

// defines where each part of memory ends
#define TEXT_END 10000
#define DATA_END 20000

// create custom types for use in project
typedef uint32_t mem_addr;
typedef uint32_t mem_word;
typedef unsigned char byte;

// memory definition
byte memory[MEMORY_LENGTH];

// structs to keep up with the variables declared
// in the assembly code. Only used to ensure that
// the memory addresses written in the "text" part
// of memory match up correctly with where the
// variables are stored
struct holdJumpLabel {
    string name;
    byte address1;
    byte address2;
};

struct holdBranchInstruction {
    string name;
    byte address;
};

struct holdVariable {
    string name;
    byte address1;
    byte address2;
};

// methods related to memory manipulation are in this class
class GPRMem {
public:
    GPRMem(string fileName);
private:
    bool load_text(string line);
    bool load_data(string line);
};