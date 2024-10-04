#ifndef SLRDFA_H
#define SLRDFA_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

class SLRDFA {
    public:
        SLRDFA(std::stringstream &transitionsData, std::stringstream &reductionsData){ 
            populateTransitions(transitionsData);
            populateReductions(reductionsData);
        };
        std::unordered_map<int, std::unordered_map<std::string, int>> transitions;
        std::unordered_map<int, std::unordered_map<std::string, int>> reductions;

        void populateTransitions(stringstream &transitionsData) {
            std::string line;
            std::string currWord = "";
            std::vector<std::string> words = {};

            getline(transitionsData, line); // skip .TRANSITIONS line

            while(getline(transitionsData, line)) {
                line += '\n';
                for (size_t i = 0; i < line.length(); i++){
                    if (line[i] == '\n'){
                        transitions[stoi(words[0])][words[1]] = stoi(currWord);
                        currWord = "";
                        words = {};
                    }
                    else if (line[i] == ' ') { words.push_back(currWord); currWord = ""; }
                    else { currWord += line[i]; }
                }
            }
        }

        void populateReductions(stringstream &reductionsData) {
            std::string line;
            std::string currWord = "";
            std::vector<std::string> words = {};

            getline(reductionsData, line); // skip .REDUCTIONS line

            while(getline(reductionsData, line)) {
                line += '\n';
                for (size_t i = 0; i < line.length(); i++){
                    if (line[i] == '\n'){
                        reductions[stoi(words[0])][currWord] = stoi(words[1]);
                        currWord = "";
                        words = {};
                    }
                    else if (line[i] == ' ') { words.push_back(currWord); currWord = ""; }
                    else { currWord += line[i]; }
                }
            }
        }

        void printTransitions(){
            cout << "-----TRANSITIONS-----" << endl;
            for (const auto& outerPair : transitions) {
                const auto& fromState = outerPair.first;
                const auto& innerMap = outerPair.second;
                // Iterate through the inner unordered_map
                for (const auto& innerPair : innerMap) {
                    std::string symbol = innerPair.first;
                    const int& toState = innerPair.second;
                    std::cout << "From: " << fromState << "; Symbol: " << symbol << "; To: " << toState << std::endl;
                }
            }
        }

        void printReductions(){
            cout << "-----REDUCTIONS-----" << endl;
            for (const auto& outerPair : transitions) {
                const auto& state = outerPair.first;
                const auto& innerMap = outerPair.second;
                // Iterate through the inner unordered_map
                for (const auto& innerPair : innerMap) {
                    std::string symbol = innerPair.first;
                    const int& ruleNumber = innerPair.second;
                    std::cout << "State: " << state << "; Symbol: " << symbol << "; RuleNum: " << ruleNumber << std::endl;
                }
            }
        }
};

#endif