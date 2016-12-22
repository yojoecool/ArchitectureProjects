#include "GPRMem.h"

// keep up with where the next open part of memory is
int text_nextOpen;
int data_nextOpen;

// vector holding the variables
vector<holdJumpLabel> labels;
vector<holdVariable> variables;
vector<holdBranchInstruction> branches;

// created to ensure that whitespaces and comments do not
// affect the code at runtime
string string_trim(string input) {
    char tab[] = "\t";
    char space[] = " ";
    char whitespace[] = " \t";
    if (input.length() > 0) {
        size_t first = input.find_first_not_of(whitespace);
        if (first != string::npos) {
            input = input.substr(first);
        }
        else return "";
        
        if (input.length() > 0 && input.find("#") != string::npos) {
            input = input.substr(0, input.find("#"));
        }
        
        while (input.length() > 0 && (strcmp(&input.at(input.length() - 1), tab) == 0 || strcmp(&input.at(input.length() - 1), space) == 0)) {
            input = input.substr(0, input.length() - 1);
        }
    }
    return input;
}

// loads the code from a file and loads the
// text and data into memory
GPRMem::GPRMem(string fileName)
{
    text_nextOpen = TEXT_START;
    data_nextOpen = DATA_START;
    
    string line = "";
    ifstream gpr_file(fileName);
    
    int mode = 0;   // 0 = text, 1 = data
    while (getline(gpr_file, line)) {
        line = string_trim(line);
        if (line.find(".data") != string::npos) {
            mode = 1;
            continue;
        }
        else if (line.find(".text") != string::npos) {
            mode = 0;
            continue;
        }
        else if (line.length() <= 1) {
            continue;
        }
        
        switch (mode) {
            case 0:
                // save in text segment
                if (!load_text(line)) {
                    cout << "Error text" << endl;
                }
                break;
            case 1:
                // save in data
                if (!load_data(line)) {
                    cout << "Error data" << endl;
                }
                break;
            default:
                continue;
        }
    }
    gpr_file.close();

    int i;
    for (i = 0; i < branches.size(); i++) {
        int n;
        bool found = false;
        for (n = 0; n < labels.size() && !found; n++) {
            if (branches[i].name == labels[n].name) {
                memory[(int)branches[i].address] = labels[n].address1;
                memory[(int)branches[i].address + 1] = labels[n].address2;

                found = true;
            }
        }
        if (!found) cout << "Branch error\tname: " << branches[i].name << endl;
    }
}

// loads text into memory by first getting the opcode from
// the code and calling appropriate methods
bool GPRMem::load_text(string line) {
    byte opcode; // 1 = addi, 2 = b, 3 = beqz, 4 = bge, 5 = bne, 6 = print
    if (line.find("addi ") != string::npos) {
        opcode = 1;
        
        byte regd, regs, imm1, imm2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regd = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t secondReg = line.find("$");
        comma = line.find(",");
        holdReg = line.substr(secondReg + 1, comma - secondReg);
        
        regs = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        uint16_t holdImm = stoi(line);
        
        imm1 = (holdImm & 0x0000ff00) >> 8;
        imm2 = (holdImm & 0x000000ff);
        
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regd;
        memory[text_nextOpen + 2] = regs;
        memory[text_nextOpen + 3] = imm1;
        memory[text_nextOpen + 4] = imm2;

        text_nextOpen += 5;
        return true;
    }
    else if (line.find("b ") != string::npos && line.find("lb ") == string::npos) {
        opcode = 2;
        
        size_t space = line.find(" ");
        line = line.substr(space + 1);
        
        int i;
        byte labelAddress1;
        byte labelAddress2;
        bool found = false;
        for (i = 0; i < labels.size() && !found; i++) {
            if (line.compare(labels[i].name) == 0) {
                labelAddress1 = labels[i].address1;
                labelAddress2 = labels[i].address2;
                found = true;
            }
        }
        if (!found) {
            holdBranchInstruction temp;
            temp.name = line;
            temp.address = text_nextOpen + 1;
            branches.push_back(temp);
        }
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = labelAddress1;
        memory[text_nextOpen + 2] = labelAddress2;

        text_nextOpen += 3;
        return true;
    }
    else if (line.find("beqz ") != string::npos) {
        opcode = 3;
        
        byte regs;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regs = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        int i;
        byte labelAddress1;
        byte labelAddress2;
        bool found = false;
        for (i = 0; i < labels.size() && !found; i++) {
            if (line.compare(labels[i].name) == 0) {
                labelAddress1 = labels[i].address1;
                labelAddress2 = labels[i].address2;
                found = true;
            }
        }
        if (!found) {
            holdBranchInstruction temp;
            temp.name = line;
            temp.address = text_nextOpen + 2;
            branches.push_back(temp);
        }
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regs;
        memory[text_nextOpen + 2] = labelAddress1;
        memory[text_nextOpen + 3] = labelAddress2;

        text_nextOpen += 4;
        return true;
    }
    else if (line.find("bge ") != string::npos) {
        opcode = 4;
        
        byte reg1, reg2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        reg1 = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t secondReg = line.find("$");
        comma = line.find(",");
        holdReg = line.substr(secondReg + 1, comma - secondReg);
        
        reg2 = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        int i;
        byte labelAddress1;
        byte labelAddress2;
        bool found = false;
        for (i = 0; i < labels.size() && !found; i++) {
            if (line.compare(labels[i].name) == 0) {
                labelAddress1 = labels[i].address1;
                labelAddress2 = labels[i].address2;
                found = true;
            }
        }
        if (!found) {
            holdBranchInstruction temp;
            temp.name = line;
            temp.address = text_nextOpen + 3;
            branches.push_back(temp);
        }
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = reg1;
        memory[text_nextOpen + 2] = reg2;
        memory[text_nextOpen + 3] = labelAddress1;
        memory[text_nextOpen + 4] = labelAddress2;

        text_nextOpen += 5;
        return true;
    }
    else if (line.find("bne ") != string::npos) {
        opcode = 5;
        byte reg1, reg2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        reg1 = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t secondReg = line.find("$");
        comma = line.find(",");
        holdReg = line.substr(secondReg + 1, comma - secondReg);
        
        reg2 = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        int i;
        byte labelAddress1;
        byte labelAddress2;
        bool found = false;
        for (i = 0; i < labels.size() && !found; i++) {
            if (line.compare(labels[i].name) == 0) {
                labelAddress1 = labels[i].address1;
                labelAddress2 = labels[i].address2;
                found = true;
            }
        }
        if (!found) {
            holdBranchInstruction temp;
            temp.name = line;
            temp.address = text_nextOpen + 3;
            branches.push_back(temp);
        }
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = reg1;
        memory[text_nextOpen + 2] = reg2;
        memory[text_nextOpen + 3] = labelAddress1;
        memory[text_nextOpen + 4] = labelAddress2;

        text_nextOpen += 5;
        return true;
    }
    else if (line.find("la ") != string::npos) {
        opcode = 6;
        byte reg;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        reg = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        int i;
        byte varAddress1;
        byte varAddress2;
        bool found = false;
        for (i = 0; i < variables.size() && !found; i++) {
            if (line.compare(variables[i].name) == 0) {
                varAddress1 = variables[i].address1;
                varAddress2 = variables[i].address2;
                found = true;
            }
        }
        if (!found) {
            return false;
        }
        else {
            memory[text_nextOpen] = opcode;
            memory[text_nextOpen + 1] = reg;
            memory[text_nextOpen + 2] = varAddress1;
            memory[text_nextOpen + 3] = varAddress2;

            text_nextOpen += 4;
        }
        return true;
    }
    else if (line.find("lb ") != string::npos) {
        opcode = 7;
        byte regd, regs;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regd = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));

        byte offset;
        size_t par = line.find("(");
        string holdOffset = line.substr(0, par);
        if (holdOffset.size() != 0) {
            offset = stoi(holdOffset);
        }
        else offset = 0;

        line = string_trim(line.substr(par + 1));

        size_t secReg = line.find("$");
        par = line.find(")");
        holdReg = line.substr(secReg + 1, par - secReg);
        
        regs = stoi(holdReg);
        
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regd;
        memory[text_nextOpen + 2] = offset;
        memory[text_nextOpen + 3] = regs;

        text_nextOpen += 4;
        return true;
    }
    else if (line.find("li ") != string::npos) {
        opcode = 8;
        byte regd;
        byte imm1, imm2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regd = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        uint16_t holdImm = stoi(line);
        
        imm1 = (holdImm & 0x0000ff00) >> 8;
        imm2 = (holdImm & 0x000000ff);
        
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regd;
        memory[text_nextOpen + 2] = imm1;
        memory[text_nextOpen + 3] = imm2;

        text_nextOpen += 4;
        return true;
    }
    else if (line.find("subi ") != string::npos) {
        opcode = 9;
        
        byte regd, regs, imm1, imm2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regd = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t secondReg = line.find("$");
        comma = line.find(",");
        holdReg = line.substr(secondReg + 1, comma - firstReg);
        
        regs = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        uint16_t holdImm = stoi(line);
        
        imm1 = (holdImm & 0x0000ff00) >> 8;
        imm2 = (holdImm & 0x000000ff);
        
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regd;
        memory[text_nextOpen + 2] = regs;
        memory[text_nextOpen + 3] = imm1;
        memory[text_nextOpen + 4] = imm2;

        text_nextOpen += 5;
        return true;
    }
    else if (line.find("syscall") != string::npos) {
        opcode = 10;
        memory[text_nextOpen] = opcode;

        text_nextOpen++;
        return true;
    }
    else if (line.find("nop") != string::npos) {
        opcode = 11;
        memory[text_nextOpen] = opcode;

        text_nextOpen++;
        return true;
    }
    else if (line.find("add ") != string::npos) {
        opcode = 12;
        
        byte regd, regs1, regs2;
        
        size_t firstReg = line.find("$");
        size_t comma = line.find(",");
        string holdReg = line.substr(firstReg + 1, comma - firstReg);
        
        regd = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t secondReg = line.find("$");
        comma = line.find(",");
        holdReg = line.substr(secondReg + 1, comma - secondReg);
        
        regs1 = stoi(holdReg);
        
        line = string_trim(line.substr(comma + 1));
        
        size_t thirdReg = line.find("$");
        holdReg = line.substr(thirdReg + 1);
        
        regs2 = stoi(holdReg);
        
        memory[text_nextOpen] = opcode;
        memory[text_nextOpen + 1] = regd;
        memory[text_nextOpen + 2] = regs1;
        memory[text_nextOpen + 3] = regs2;
        
        text_nextOpen += 4;
        return true;
    }
    else {
        size_t colon = line.find(":");
        line = line.substr(0, colon);
        holdJumpLabel temp;
        temp.name = line;
        temp.address1 = (text_nextOpen & 0x0000ff00) >> 8;
        temp.address2 = (text_nextOpen & 0x000000ff);
        
        labels.push_back(temp);
        
        memory[text_nextOpen] = 0;

        text_nextOpen++;
        return true;
    }
    return true;
}

// load the data into memory by finding the variable name,
// saving it to the variables vector, and saving the value
// in the data part of memory
bool GPRMem::load_data(string line) {
    holdVariable temp;
    // variables declaration must be written in the format
    // "name: .variableType value"
    if (line.find(":") != string::npos) {
        temp.name = line.substr(0, line.find(":"));
    }
    else return false;
    
    line = string_trim(line.substr(line.find(":") + 1));
    
    // if the variable is a word, mask its value
    if (line.find("asciiz") != string::npos) {
        line = string_trim(line.substr(line.find("z") + 1));
        
        line = line.substr(1, strlen(line.c_str()) - 5);
        
        // make sure that there is still room in the data
        // part of memory
        if (data_nextOpen < DATA_END) {
            temp.address1 = (data_nextOpen & 0x0000ff00) >> 8;
            temp.address2 = (data_nextOpen & 0x000000ff) >> 0;
            
            int i;
            for (i = 0; i < line.length() && data_nextOpen < DATA_END; i++) {
                memory[data_nextOpen] = line.at(i);
                data_nextOpen++;
            }
            data_nextOpen++;
            
            variables.push_back(temp);
            
            return true;
        }
        cout << "no space" << endl;
        return false;
    }
    
    else if (line.find("space") != string::npos) {
        line = string_trim(line.substr(line.find("space") + 5));
        
        temp.address1 = (data_nextOpen & 0x0000ff00) >> 8;
        temp.address2 = (data_nextOpen & 0x000000ff);
        
        mem_word p = (mem_word) stoi(line);
        
        int i = 0;
        for (i = data_nextOpen; i < p && i < DATA_END; i++) {
            memory[i] = 0;
        }
        
        data_nextOpen += p;
        
        variables.push_back(temp);
        
        return true;
    }
    return false;
}