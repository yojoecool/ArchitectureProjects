/*
* Added:
* NOP Counter
* IC
* C
* Branch now adds a NOP - Done seperately from the normal NOP for 2 reasons:
*   1. Easy way to avoid increasing currentText
*   2. Easier to add things to this NOP if we need to, since it is a seperate case
* Fetch has been changed to handle this new NOP
* opA and opB have been added to the id_ex struct
* ADDI, ADD, SUBI, and LB now use opA and opB instead of rSrc1 and rSrc2
* Decode and Execute now use opA and opB instead of rSrc1 and rSrc2
*/
#include <stdio.h>
#include <stdint.h>
#include <iomanip>
#include "GPRMem.cpp"


uint16_t reg[32];
bool userMode = true;
int currentText = 0;
bool isBranching = false;
int NOPcounter = 0;
int IC = 0;
bool branchFound = false;   // was the last instruction decoded a branch
bool addNop = false;    // add a NOP to current cycle


// ADDI Rdest, Rsrc1, Imm
uint16_t addi(uint16_t opA, uint16_t imm) {
    return opA + imm;
}

// ADD Rdest, Rsrc1, Rsrc2
uint16_t add(uint16_t opA, uint16_t opB) {
    return opA + opB;
}

// SUBI Rdest, Rsrc1, Imm
uint16_t subi(uint16_t opA, uint16_t imm) {
    return opA - imm;
}
 
// LA Rdest, label
void la(unsigned char rDest, uint16_t label) {
    reg[(int) rDest] = label;
}

// LB Rdest, offset(Rsrc1)
uint16_t lb(uint16_t opA) {
    return (uint16_t)memory[opA];
}

//case END
void end() {
    userMode = false;
}

// SYSCALL
// 4 -> printf
// 8 -> scanf
// 10 -> end the program
void syscall(bool memPhase) {
    int i;
    char input_string[(int)reg[30]];
    int memLocation;
    switch ((int)reg[29]) {
        case 1:
            if (!memPhase) {
                cout << (int16_t)reg[4] << endl;
            }
            break;

        case 4:
            if (memPhase) {
                i = (int)reg[30];
                while ((int)memory[i] != 0) {
                    cout << memory[i];
                    i++;
                }
                printf("\n");
            }
            break;

        case 8:
            if (memPhase) {
                cout << "Enter a string: ";
                std::fgets(input_string, (int)reg[31], stdin);
                memLocation = (int)reg[30];
                for (int i = 0; i < (int)reg[31]; i++) {
                    memory[memLocation + i] = input_string[i];
                }
            }
            break;
            
        case 10:
            userMode = false;
            break;
    }
}


// Latches

struct id_ex {
    int opCode;
    unsigned char rSrc1;
    unsigned char rSrc2;
    unsigned char rDest;
    uint16_t opA;
    uint16_t opB;
    uint16_t imm;
    uint16_t label;     // new PC for branches
    
    id_ex(int opCode = 13, unsigned char rSrc1 = '\0', unsigned char rSrc2 = '\0', unsigned char rDest = '\0',  
          uint16_t opA = 0, uint16_t opB = 0, uint16_t imm = 0, uint16_t label = 0) :
    opCode (opCode), rSrc1 (rSrc1), rSrc2 (rSrc2), rDest (rDest), opA (opA), opB (opB), imm (imm), label (label) {}
};

struct ex_mem {
    int opCode;
    uint16_t ALU_out;
    unsigned char rDest;
    
    ex_mem(int opCode = 13, uint16_t ALU_out = 0, unsigned char rDest = '\0') :
    opCode (opCode), ALU_out (ALU_out), rDest (rDest) {}
};

// WHAT IS MDR
struct mem_wb {
    int opCode;
    uint16_t ALU_out;
    unsigned char rDest;
    
    mem_wb(int opCode = 13, uint16_t ALU_out = 0, unsigned char rDest = '\0') :
    opCode (opCode), ALU_out (ALU_out), rDest (rDest) {}
};

// instruction register
int if_id_new = 0;
int if_id_old = 0;

// [opcode, rs, rt, rd, operand A, operand B, immediate or offset, new PC for branches]
id_ex* id_ex_new = new id_ex;
id_ex* id_ex_old = new id_ex;

// [op code, ALU out, rd]
ex_mem* ex_mem_new = new ex_mem;
ex_mem* ex_mem_old = new ex_mem;

// [op code, MDR?, ALU out, rd]
mem_wb* mem_wb_new = new mem_wb;
mem_wb* mem_wb_old = new mem_wb;


// method for forwarding
void forwarding() {
    if (ex_mem_new->rDest == id_ex_new->rSrc1) {
        id_ex_new->opA = ex_mem_new->ALU_out;
    }
    else if (ex_mem_old->rDest == id_ex_new->rSrc1) {
        id_ex_new->opA = ex_mem_old->ALU_out;
    }

    if (ex_mem_new->rDest == id_ex_new->rSrc2) {
        id_ex_new->opB = ex_mem_new->ALU_out;
    }
    else if (ex_mem_old->rDest == id_ex_new->rSrc2) {
        id_ex_new->opB = ex_mem_old->ALU_out;
    }
}

// Stages

// Fetch: use current value in PC to index memory and retrieve instruction
//          put instruction into if/id
void fetch() {
    if_id_old = if_id_new;
    if (branchFound) {
        addNop = true;
        branchFound = false;
    }
    else if_id_new = currentText;
}

// Decode: reads zero, one, or two values out of register file
//          stores value in id/ex
void decode() {
    // Move new to old
    id_ex_old->opCode = id_ex_new->opCode;
    id_ex_old->rSrc1 = id_ex_new->rSrc1;
    id_ex_old->rSrc2 = id_ex_new->rSrc2;
    id_ex_old->rDest = id_ex_new->rDest;
    id_ex_old->opA = id_ex_new->opA;
    id_ex_old->opB = id_ex_new->opB;
    id_ex_old->imm = id_ex_new->imm;
    id_ex_old->label = id_ex_new->label;
    
    id_ex_new->opCode = (int)memory[if_id_new];
    if (addNop) id_ex_new->opCode = 14;

    if (id_ex_new->opCode != 0) IC++;
    
    switch (id_ex_new->opCode) {
        case 0: // label
            currentText += 1;
            break;
            
        case 1: // addi
            id_ex_new->rDest = memory[if_id_new + 1];
            id_ex_new->rSrc1 = memory[if_id_new + 2];
            id_ex_new->imm = memory[if_id_new + 3] << 8;
            id_ex_new->imm |= memory[if_id_new + 4];

            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            forwarding();
            
            currentText += 5;
            break;

        case 2: // b
            id_ex_new->label = memory[if_id_new + 1] << 8;
            id_ex_new->label |= memory[if_id_new + 2];
            
            branchFound = true;
            currentText = id_ex_new->label;
            break;
            
        case 3: // beqz
            id_ex_new->rSrc1 = memory[if_id_new + 1];
            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            id_ex_new->label = memory[if_id_new + 2] << 8;
            id_ex_new->label |= memory[if_id_new + 3];
            
            forwarding();

            branchFound = true;
            if ((int)id_ex_new->opA == 0) currentText = id_ex_new->label;
            else currentText += 4;
            break;
            
        case 4: // bge
            id_ex_new->rSrc1 = memory[if_id_new + 1];
            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            id_ex_new->rSrc2 = memory[if_id_new + 2];
            id_ex_new->opB = reg[(int)id_ex_new->rSrc2];

            id_ex_new->label = memory[if_id_new + 3] << 8;
            id_ex_new->label |= memory[if_id_new + 4];

            branchFound = true;

            forwarding();

            if ((int16_t)id_ex_new->opA >= (int16_t)id_ex_new->opB) currentText = id_ex_new->label;
            else currentText += 5;
            break;
            
        case 5: // bne
            id_ex_new->rSrc1 = memory[if_id_new + 1];
            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            id_ex_new->rSrc2 = memory[if_id_new + 2];
            id_ex_new->opB = reg[(int)id_ex_new->rSrc2];

            id_ex_new->label = memory[if_id_new + 3] << 8;
            id_ex_new->label |= memory[if_id_new + 4];

            forwarding();
            
            branchFound = true;
            if (id_ex_new->opA != id_ex_new->opB) currentText = id_ex_new->label;
            else currentText += 5;
            break;
            
        case 6: // la
            id_ex_new->rDest = memory[if_id_new + 1];
            id_ex_new->label = memory[if_id_new + 2] << 8;
            id_ex_new->label |= memory[if_id_new + 3];
            
            currentText += 4;
            break;
            
        case 7: // lb
            id_ex_new->rDest = memory[if_id_new + 1];

            id_ex_new->imm = memory[if_id_new + 2];      //offset
            id_ex_new->rSrc1 = memory[if_id_new + 3];

            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            forwarding();
            
            currentText += 4;
            break;
            
        case 8: // li
            id_ex_new->rDest = memory[if_id_new + 1];
            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            id_ex_new->imm = memory[if_id_new + 2] << 8;
            id_ex_new->imm |= memory[if_id_new + 3];

            forwarding();
            
            currentText += 4;
            break;
            
        case 9: // subi
            id_ex_new->rDest = memory[if_id_new + 1];
            id_ex_new->rSrc1 = memory[if_id_new + 2];
            
            id_ex_new->imm = memory[if_id_new + 3] << 8;
            id_ex_new->imm |= memory[if_id_new + 4];

            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];

            forwarding();
            
            currentText += 5;
            break;
            
        case 10: // syscall
            currentText += 1;
            break;

        case 11: // NOP
            currentText += 1;
            NOPcounter += 1;
            break;
            
        case 12: // add
            id_ex_new->rDest = memory[if_id_new + 1];
            id_ex_new->rSrc1 = memory[if_id_new + 2];
            id_ex_new->rSrc2 = memory[if_id_new + 3];

            id_ex_new->opA = reg[(int)id_ex_new->rSrc1];
            id_ex_new->opB = reg[(int)id_ex_new->rSrc2];

            forwarding();
            
            currentText += 4;
            break;

        case 14: // added NOP
            NOPcounter += 1;
            addNop = false;
            break;
            
        default:
            break;
    }
}

// Execute: execute operation
//          put result in ex/mem
// For Branches, ALU_out = 1 for true, 0 for false
void execute() {
    // move new to old
    ex_mem_old->opCode = ex_mem_new->opCode;
    ex_mem_old->ALU_out = ex_mem_new->ALU_out;
    ex_mem_old->rDest = ex_mem_new->rDest;
    
    ex_mem_new->opCode = id_ex_new->opCode;
    
    switch (ex_mem_new->opCode) {

        case 1: // addi
            ex_mem_new->ALU_out = addi(id_ex_new->opA, id_ex_new->imm);
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        case 6: // la
            ex_mem_new->ALU_out = id_ex_new->label;
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        case 7: // lb
            ex_mem_new->ALU_out = lb(id_ex_new->opA);
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        case 8: // li
            ex_mem_new->ALU_out = id_ex_new->imm;
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        case 9: // subi
            ex_mem_new->ALU_out = subi(id_ex_new->opA, id_ex_new->imm);
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        case 10: // syscall
            syscall(false);
            break;
            
        case 12: // add
            ex_mem_new->ALU_out = add(id_ex_new->opA, id_ex_new->opB);
            ex_mem_new->rDest = id_ex_new->rDest;
            break;
            
        default:
            break;
    }
}

// Memory: if instruction in mem is r-type then the mem/wb should buffer the result held in ex/mem
void memStage() {
    mem_wb_old->opCode = mem_wb_new->opCode;
    mem_wb_old->ALU_out = mem_wb_new->ALU_out;
    mem_wb_old->rDest = mem_wb_new->rDest;
    
    mem_wb_new->opCode = ex_mem_new->opCode;
    switch (mem_wb_new->opCode) {
        case 1: // addi
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;
        
        case 6: // la
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;
        
        case 7: // lb
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;
            
        case 8: // li
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;
            
        case 9: // subi
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;

        case 10:
            syscall(true);
            break;
        
        case 12: // add
            mem_wb_new->ALU_out = ex_mem_new->ALU_out;
            mem_wb_new->rDest = ex_mem_new->rDest;
            break;
            
        default:
            break;
    }
    
}

// Write-Back: puts results of R-type instrction or a load into the register file
//              value comes from mem/wb
void write_back() {
    switch (mem_wb_new->opCode) {
        case 1: // addi
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        case 6: // la
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        case 7: // lb
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        case 8: // li
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        case 9: // subi
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        case 12: // add
            reg[(int)mem_wb_new->rDest] = mem_wb_new->ALU_out;
            break;
            
        default:
            break;
    }
}

int main() {
    string fileName;
    char getNextLine[100];
    cout << "Please enter a file name: ";
    cin >> fileName;
    fgets(getNextLine, 100, stdin);
    GPRMem tempMem = GPRMem(fileName);
    
    int C = 0;
    
    while (userMode) {
        write_back();
        memStage();
        execute();
        decode();
        fetch();
        C += 2;
    }

    cout << "\nInstruction Count: " << IC << "\tCycles: " << C << "\tNOP Count: " << NOPcounter << endl;
    return 0;
}
