#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cctype>
#include <fstream>
#include <limits.h>
#include <bitset>
#include <algorithm>
#include <deque>

#include "wlp4data.h"
#include "rules.h"
#include "slrdfa.h"

using namespace std;

typedef struct {
    std::string lexeme;
    std::string kind;
} token;

struct Node {
  std::string data;
  std::vector<Node*> children;
  Node(std::string data) : data(data) {}
  ~Node() { for( auto &c : children ) { delete c; } }
  void print(std::string identationPrefix, std::ostream &out = std::cout) {
    std::cout << identationPrefix << this->data << std::endl;

    if (identationPrefix[identationPrefix.length()-2] == '\'') { identationPrefix.pop_back(); identationPrefix.pop_back(); identationPrefix += "  "; }
    else if (identationPrefix[identationPrefix.length()-2] == '|'){ identationPrefix.pop_back(); identationPrefix += " "; }

    for (size_t i = 0; i < this->children.size(); i++) {
        if (i == this->children.size()-1) { this->children[i]->print(identationPrefix + "'-"); }
        else { this->children[i]->print(identationPrefix + "|-"); }
    }
  }
  void printTree(){
    cout << this->data << endl;
    for (size_t i = 0; i < this->children.size(); i++) { this->children[i]->printTree(); }
  }
};

void readInput(std::deque<token> &inputSequence, std::istream &in);
void reduceTrees(std::vector<Node*> &treeStack, Rules::Rule cfgRule);
void reduceStates(SLRDFA &slrdfa, std::vector<int> &stateStack, Rules::Rule cfgRule);
void shift(SLRDFA &slrdfa, std::deque<token> &inputSequence, std::vector<Node*> &treeStack, std::vector<int> &stateStack);
// void tokensToTrees(std::deque<token> &inputSequence, std::vector<Node*> &treeStack);

int main() {
    std::vector<int> stateStack = {0};
    std::vector<Node*> treeStack = {};
    try {
        stringstream CFGData(WLP4_CFG);
        Rules rules {CFGData};
        stringstream TransitionsData(WLP4_TRANSITIONS);
        stringstream ReductionsData(WLP4_REDUCTIONS);
        SLRDFA slrdfa {TransitionsData, ReductionsData};
        // slrdfa.printTransitions();
        // slrdfa.printReductions();
        // rules.printRules();

        std::string input;
        std::deque<token> inputSequence;
        readInput(inputSequence, cin);

        //testing
        // tokensToTrees(inputSequence, treeStack);
        // for (int i = 0; i < treeStack.size(); i++) {
        //     cout << "Element " << i << endl;
        //     treeStack[i]->print("", cout);
        // }
        // reduceTrees(treeStack, rules.allRules[11]);
        // for (int i = 0; i < treeStack.size(); i++) {
        //     cout << "Element " << i << endl;
        //     treeStack[i]->print("", cout);
        // }

        // main parsing loop
        while (!inputSequence.empty()) {
            while (true){
                int currState = stateStack[stateStack.size()-1];
                std::string firstTokenKind = inputSequence.front().kind;
                if ((slrdfa.reductions.find(currState) != slrdfa.reductions.end()) && (slrdfa.reductions[currState].find(firstTokenKind) != slrdfa.reductions[currState].end())) {
                    int ruleNum = slrdfa.reductions[currState][firstTokenKind];
                    reduceTrees(treeStack, rules.allRules[ruleNum]);
                    reduceStates(slrdfa, stateStack, rules.allRules[ruleNum]);
                }
                else { break; }
            }
            shift(slrdfa, inputSequence, treeStack, stateStack);
        }
        reduceTrees(treeStack, rules.allRules[0]);
        // treeStack[0]->print("", cout);
        treeStack[0]->printTree();
        
    x
    for (Node *node : treeStack) { delete node; }
    return 0;
}

void readInput(std::deque<token> &inputSequence, std::istream &in) {
    std::string input = "";
    inputSequence.push_back({"BOF", "BOF"});

    while (std::getline(in, input)) {
        std::string currWord = "";
        token tmp = {"", ""};
        for (size_t i = 0; i < input.length(); i++) {
            if (input[i] == ' ') { tmp.kind = currWord; currWord = ""; }
            else if (i == input.length() - 1) { currWord += input[i]; tmp.lexeme = currWord; inputSequence.push_back(tmp); }
            else { currWord += input[i]; }
        }
    }
    inputSequence.push_back({"EOF", "EOF"});
}

void reduceTrees(std::vector<Node*> &treeStack, Rules::Rule cfgRule) {
    string concatName = cfgRule.leftSymbol;
    for (size_t i = 0 ; i < cfgRule.rightSymbols.size(); i++) { concatName += " " + cfgRule.rightSymbols[i];}
    Node *newNode = new Node(concatName);
    
    int len = cfgRule.rightSymbols.size();
    if (len == 1 && cfgRule.rightSymbols[0] == ".EMPTY") { len = 0; } // if RHS == ".EMPTY", pop NOTHING
    for (int i = len; i > 0; i--){
        Node *tmp = treeStack[treeStack.size()-i];
        newNode->children.push_back(tmp);
    }
    for (int i = len; i > 0; i--) { treeStack.pop_back(); }
    treeStack.push_back(newNode);
}

void reduceStates(SLRDFA &slrdfa, std::vector<int> &stateStack, Rules::Rule cfgRule) {
    int len = cfgRule.rightSymbols.size();
    if (len == 1 && cfgRule.rightSymbols[0] == ".EMPTY") { len = 0; } // if RHS == ".EMPTY", pop NOTHING
    for (int i = 0; i < len; i++) {
        stateStack.pop_back();
    }
    std::string leftSide = cfgRule.leftSymbol;
    int stateStackTop = stateStack[stateStack.size()-1];
    int res = slrdfa.transitions[stateStackTop][leftSide];
    stateStack.push_back(res);
}

void shift(SLRDFA &slrdfa, std::deque<token> &inputSequence, std::vector<Node*> &treeStack, std::vector<int> &stateStack) {
    Node *newNode = new Node(inputSequence.front().kind + " " + inputSequence.front().lexeme);
    int currState = stateStack[stateStack.size()-1];
    treeStack.push_back(newNode);
    if ((slrdfa.transitions.find(currState) == slrdfa.transitions.end()) || (slrdfa.transitions[currState].find(inputSequence.front().kind) == slrdfa.transitions[currState].end())) {
        throw std::runtime_error("ERROR: NO TRANSITION POSSIBLE");
    }
    else {
        int nextState = slrdfa.transitions[currState][inputSequence.front().kind];
        stateStack.push_back(nextState);
        inputSequence.pop_front();
    }
}

// void tokensToTrees(std::deque<token> &inputSequence, std::vector<Node*> &treeStack) {
//     while (!inputSequence.empty()) {
//         Node *newNode = new Node(inputSequence.front().lexeme);
//         treeStack.push_back(newNode);
//         cout << "lexeme: " << inputSequence.front().lexeme << ". kind: " << inputSequence.front().kind << endl;
//         inputSequence.pop_front();
//     }
// }
