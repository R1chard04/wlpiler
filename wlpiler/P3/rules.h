#ifndef RULES_H
#define RULES_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

class Rules {
    public:
        Rules(std::stringstream &rulesData){ populateRules(rulesData); };

        typedef struct {
            std::string leftSymbol;
            std::vector<std::string> rightSymbols;
        } Rule;

        std::vector<Rule> allRules = {};

        void populateRules(std::stringstream &rulesData){
            Rule tmpRule = {"", {}};
            std::string currWord = "";
            bool first = true;
            string line;

            getline(rulesData, line); // skip .CFG line

            while (getline(rulesData, line)) {
                line += '\n';
                for (size_t i = 0; i < line.length(); i++){
                    if (line[i] == '\n'){
                        tmpRule.rightSymbols.push_back(currWord);
                        this->allRules.push_back(tmpRule);
                        tmpRule = {"", {}};
                        currWord = "";
                        first = true;
                    }
                    else if (line[i] == ' ') {
                        if (first) { tmpRule.leftSymbol = currWord; first = false; currWord = ""; }
                        else { tmpRule.rightSymbols.push_back(currWord); currWord = ""; }
                    }
                    else { currWord += line[i]; }
                }
            }
            
        }

        void printRules(){
            cout << allRules.size() << endl;
            for (Rule r : this->allRules){
                cout << "Left Symbol: " << r.leftSymbol << endl;
                cout << "Right Symbol: "; 
                for (string s : r.rightSymbols) { cout << s << ","; }
                cout << endl;
            }
        }
};

#endif