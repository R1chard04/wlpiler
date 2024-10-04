#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cctype>
#include <fstream>
#include <limits.h>
#include <bitset>
#include <algorithm>
#include "asm.h"

using namespace std;

int main(){
    try {
        std::string input;
        ASM assembler{}; // puts code in vector of vectors format
        
        assembler.createProg(cin);

        assembler.checkValid();
        
    } catch(std::runtime_error &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

ASM::ASM () {}

void ASM::createProg(std::istream &in) {
    std::string input = "";
    int index = 0;
    std::string lastKind = "NEWLINE"; // so that we don't have duplicate newlines at a time

    while (std::getline(in, input)) {
        int l = input.length();
        if (l == 0) { cerr << "ERROR\n"; return; }
        vector<std::string> currLine = {};
        std::string currString = "";
        currLine.push_back(std::to_string(index));
        for (int i = 0; i < l; i++) {
            if (input[i] == ' ') {
                currLine.push_back(currString); 
                currString = "";
                continue;
            }
            else if (i == l-1) {
                currString += input[i];
                if (currString == "NEWLINE" && lastKind != "NEWLINE" && lastKind != "LABELDEF") {
                    index += 4;
                }
                currLine.push_back(currString);
                currString = "";
                continue;
            }
            else {
                currString += input[i];
            }
        }
        this->prog.push_back(currLine);
        lastKind = prog[prog.size()-1][1];
    }
    // print for testing
    // for (const std::vector<std::string>& innerVector : this->prog) {
    //     for (string element : innerVector) {
    //         std::cout << element << " ";
    //     }
    //     std::cout << std::endl; // Print a newline after each inner vector
    // }
}

void ASM::checkValid() {
    int l = this->prog.size();

    // loop 1: syntax checking
    for (int i = 0; i < l; i++) {
        if (this->prog[i][1] == "NEWLINE") {
            continue;
        }
        else if (this->prog[i][1] == "DOTID") {
            if (this->prog[i][2] != ".word") {
                std::cerr << "ERROR\n"; return;
            }
            if ((i+1 >= l) && (this->prog[i+1][1] != "HEXINT") && (this->prog[i+1][1] != "DECINT") && (this->prog[i+1][1] != "ID")) {
                std::cerr << "ERROR\n"; return;
            }
            i++;
        }
        else if (this->prog[i][1] == "LABELDEF") {
            std::string label = this->prog[i][2].substr(0, this->prog[i][2].length() - 1);
            if (this->lookupTable.find(label) != this->lookupTable.end()) { // check for duplicates; lookup table contains label
                std::cerr << "ERROR\n"; return;
            } else {
                this->lookupTable[label] = stoi(this->prog[i][0]);
            }
        }
        else if (this->prog[i][1] == "ID") {
            if (this->prog[i][2] == "add" || this->prog[i][2] == "sub" || this->prog[i][2] == "slt" || this->prog[i][2] == "sltu") {
                if (i+5 >= l || this->prog[i+1][1] != "REGISTER" || this->prog[i+2][1] != "COMMA" || this->prog[i+3][1] != "REGISTER" 
                    || this->prog[i+4][1] != "COMMA" || this->prog[i+5][1] != "REGISTER") {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 5; }
            }
            else if (this->prog[i][2] == "beq" || this->prog[i][2] == "bne") {
                if (i+5 >= l || this->prog[i+1][1] != "REGISTER" || this->prog[i+2][1] != "COMMA" || this->prog[i+3][1] != "REGISTER"
                    || this->prog[i+4][1] != "COMMA" || (this->prog[i+5][1] != "HEXINT" && this->prog[i+5][1] != "DECINT" && this->prog[i+5][1] != "ID")) {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 5; }
            }
            else if (this->prog[i][2] == "mult" || this->prog[i][2] == "multu" || this->prog[i][2] == "div" || this->prog[i][2] == "divu") {
                if (i+3 >= l || this->prog[i+1][1] != "REGISTER" || this->prog[i+2][1] != "COMMA" || this->prog[i+3][1] != "REGISTER") {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 3; }
            }
            else if (this->prog[i][2] == "mfhi" || this->prog[i][2] == "mflo" || this->prog[i][2] == "lis") {
                if (i+1 >= l || this->prog[i+1][1] != "REGISTER") {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 1; }
            }
            else if (this->prog[i][2] == "jr" || this->prog[i][2] == "jalr") {
                if (i+1 >= l || this->prog[i+1][1] != "REGISTER") {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 1; }
            }
            else if (this->prog[i][2] == "lw" || this->prog[i][2] == "sw") {
                if (i+6 >= l || this->prog[i+1][1] != "REGISTER" || this->prog[i+2][1] != "COMMA" || (this->prog[i+3][1] != "DECINT" && this->prog[i+3][1] != "HEXINT") ||
                    this->prog[i+4][1] != "LPAREN" || this->prog[i+5][1] != "REGISTER" || this->prog[i+6][1] != "RPAREN") {
                    std::cerr << "ERROR\n"; return;
                }
                else { i += 6; }
            }
        }
        else {
            std::cerr << "ERROR\n"; return;
        }
    }
    // testing
    // for(const auto& key_value: this->lookupTable) {
    //     string key = key_value.first;
    //     int value = key_value.second;    
    //     cout << key << " - " << value << endl;
    // }

    // loop 2: output 32-bit binary encodings
    for (int i = 0; i < l; i++) {
        if (this->prog[i][1] == "NEWLINE" || this->prog[i][1] == "LABELDEF") {
            continue;
        }
        else if (this->prog[i][1] == "DOTID") {
            if (this->prog[i+1][1] == "DECINT") {
                try {  
                    if (stoul(this->prog[i+1][2]) >= 0) {
                        unsigned long int bin = stoul(this->prog[i+1][2]);
                        putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
                    }
                    else {
                        int bin = stoi(this->prog[i+1][2]);
                        putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
                    }
                } catch (...) { std::cerr << "ERROR\n"; return; }

                i += 1;
            }
            else if (this->prog[i+1][1] == "HEXINT") {
                unsigned int num = stoul(this->prog[i+1][2], nullptr, 16);
                putchar(num >> 24); putchar(num >> 16); putchar(num >> 8); putchar(num);
                i += 1;
            }
            else if (this->prog[i+1][1] == "ID") {
                if (this->lookupTable.find(this->prog[i+1][2]) == this->lookupTable.end()) { // lookup table DOES NOT contain label
                    std::cerr << "ERROR\n"; return;
                }
                int bin = this->lookupTable[this->prog[i+1][2]];
                putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
                i += 1;
            }
            else {
                std::cerr << "ERROR\n"; return;
            }
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "add" || this->prog[i][2] == "sub" || this->prog[i][2] == "slt" || this->prog[i][2] == "sltu")) {
            int regd = stoi(this->prog[i+1][2].substr(1));
            int regs = stoi(this->prog[i+3][2].substr(1));
            int regt = stoi(this->prog[i+5][2].substr(1));
            int bin = (regs << 21) | (regt << 16) | (regd << 11) | this->functionBits[this->prog[i][2]];
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
            i += 5;
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "beq" || this->prog[i][2] == "bne")) {
            int regs = stoi(this->prog[i+1][2].substr(1));
            int regt = stoi(this->prog[i+3][2].substr(1));
            int offset;
            if (this->prog[i+5][1] == "HEXINT") {
                try {
                    offset = stoi(this->prog[i+5][2], nullptr, 16);
                    if (offset > 65535) { std::cerr << "ERROR"; return; }
                } catch (...) { std::cerr << "ERROR"; return; }
            } 
            else if (this->prog[i+5][1] == "DECINT") {
                try {
                    offset = stoi(this->prog[i+5][2]);
                    if (offset < -32768 || offset > 32767) { std::cerr << "ERROR"; return; }
                } catch (...) { std::cerr << "ERROR"; return; }
            }
            else if (this->prog[i+5][1] == "ID") {
                try {
                    offset = (this->lookupTable[this->prog[i+5][2]] - (stoi(this->prog[i][0]) + 4)) / 4;
                    if (offset < -32768 || offset > 32767) { std::cerr << "ERROR"; return; }
                } catch (...) { std::cerr << "ERROR"; return; }
            }
            int bin = (this->opcodeBits[this->prog[i][2]] << 26) | (regs << 21) | (regt << 16) | (offset & 0xFFFF);
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
            i += 5;
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "mult" || this->prog[i][2] == "multu" || this->prog[i][2] == "div" || this->prog[i][2] == "divu")) {
            int regs = stoi(this->prog[i+1][2].substr(1));
            int regt = stoi(this->prog[i+3][2].substr(1));

            int bin = (regs << 21) | (regt << 16) | this->functionBits[this->prog[i][2]];
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
            i += 3;
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "mfhi" || this->prog[i][2] == "mflo" || this->prog[i][2] == "lis")) {
            int regd = stoi(this->prog[i+1][2].substr(1));
            
            int bin = (regd << 11) | this->functionBits[this->prog[i][2]];
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);
            i += 1;
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "jr" || this->prog[i][2] == "jalr")) {
            int regs = stoi(this->prog[i+1][2].substr(1));

            int bin = (regs << 21) | this->functionBits[this->prog[i][2]];
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);

            i += 1;
        }
        else if (this->prog[i][1] == "ID" && (this->prog[i][2] == "sw" || this->prog[i][2] == "lw")) {
            int regt = stoi(this->prog[i+1][2].substr(1));
            int regs = stoi(this->prog[i+5][2].substr(1));
            int offset;
            if (this->prog[i+3][1] == "HEXINT") {
                try {
                    offset = stoi(this->prog[i+3][2], nullptr, 16);
                    if (offset > 65535) { std::cerr << "ERROR"; return; }
                } catch (...) { std::cerr << "ERROR"; return; }
            } 
            else if (this->prog[i+3][1] == "DECINT") {
                try {
                    offset = stoi(this->prog[i+3][2]);
                    if (offset < -32768 || offset > 32767) { std::cerr << "ERROR"; return; }
                } catch (...) { std::cerr << "ERROR"; return; }
            }
            int bin = (this->opcodeBits[this->prog[i][2]] << 26) | (regs << 21) | (regt << 16) | (offset & 0xFFFF);
            putchar(bin >> 24); putchar(bin >> 16); putchar(bin >> 8); putchar(bin);

            i += 6;
        }
    }
}