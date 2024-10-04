#ifndef ASM_H
#define ASM_H
#include <string>
#include <vector>
#include <unordered_map>

class ASM {
    public:
        ASM();
        std::vector<std::vector<std::string>> prog;
        std::unordered_map<std::string, int> lookupTable;
        std::unordered_map<std::string, int> functionBits = {
            {"add", 32}, {"sub", 34}, {"slt", 42}, {"sltu", 43},
            {"mult", 24}, {"multu", 25}, {"div", 26}, {"divu", 27},
            {"mfhi", 16}, {"mflo", 18}, {"lis", 20},
            {"jr", 8}, {"jalr", 9}
        };
        std::unordered_map<std::string, int> opcodeBits = {
            {"beq", 4}, {"bne", 5},
            {"lw", 35}, {"sw", 43}
        };

        void createProg(std::istream &in);
        void checkValid();
};

#endif