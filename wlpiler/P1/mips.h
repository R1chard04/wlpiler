#ifndef MIPS_H
#define MIPS_H
#include <string>
#include <vector>
#include <unordered_map>

class MIPS {
    public:
        MIPS(std::istream &in);
        std::string initial;
        std::unordered_map<std::string, bool> accepting;
        std::unordered_map<std::string, std::unordered_map<char, std::string>> transitions;
        std::vector<std::string> idIntermediateStates = {"int_if", "in", "e", "el", "els", "d", "de", "del", "dele", "delet", 
                                                        "N", "NU", "NUL", "n", "ne", "r", "re", "ret", "retu", "retur", "p",
                                                        "pr", "pri", "prin", "print", "printl", "wain_while", "wa", "wai", "wh",
                                                        "whi", "whil"};
        
        std::string getNextState(std::string currState, char c);
        void simplifiedMaxMunch(std::string s);
};

#endif