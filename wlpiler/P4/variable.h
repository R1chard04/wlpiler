#ifndef VARIABLE_H
#define VARIABLE_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

struct Node {
  std::string rule = ""; // only used in nonterminal nodes
  std::string lhs = ""; // only used in nonterminal nodes
  std::string kind = ""; // only used in terminal nodes
  std::string lexeme = ""; // only used in terminal nodes
  std::vector<Node*> children;
  Node(std::string kind, std::string lexeme) : kind(kind), lexeme(lexeme) {}
  ~Node() { for( auto &c : children ) { delete c; } }

  Node(std::istream &in = std::cin) {
    std::string line = ""; 
    getline(std::cin, line); this->rule = line;
    std::vector<std::string> rhsWords = {};
    std::string currWord = "";
    
    for (char x : line) {
        if (x == ' ') { rhsWords.push_back(currWord); currWord = ""; }
        else if (x != ' ') { currWord += x; }
    }
    rhsWords.push_back(currWord);

    this->lhs = rhsWords[0];
    if (rhsWords[rhsWords.size() - 1] == ".EMPTY") { return; }
    
    for (int i = 1; i < rhsWords.size(); i++) {
        if (isupper(rhsWords[i][0])) { // Terminal
            std::string nextLine, k, l, currStr;
            getline(cin, nextLine);
            for (int i = 0; i < nextLine.length(); i++) {
                if (nextLine[i] == ' ') { k = currStr; currStr = ""; }
                else { currStr += nextLine[i]; }
            }
            l = currStr;
            Node *tmp = new Node(k, l); this->children.push_back(tmp);
        }
        else { Node* child = new Node(std::cin); this->children.push_back(child); } // Non-terminal
    }
    return;
  }

  Node *getChild (std::string str, int n = 1) {
      for (int i = 0; i < rule.length(); i++) {
          if (this->children[i]->rule == "") { // terminal
              if (this->children[i]->kind == str) { n -= 1; }
          }
          else { // non-terminal
              if (this->children[i]->lhs == str) { n -= 1; }
          }
          if (n == 0) { return this->children[i]; }
      }
      return nullptr;
  }

  void print(std::string identationPrefix, std::ostream &out = std::cout) {
    if (rule != "") { std::cout << identationPrefix << this->lhs << " " << this->rule << std::endl; } // non-terminal
    else { std::cout << identationPrefix << this->kind << " " << this->lexeme << std::endl; } // terminal
    
    if (identationPrefix[identationPrefix.length()-2] == '\'') { identationPrefix.pop_back(); identationPrefix.pop_back(); identationPrefix += "  "; }
    else if (identationPrefix[identationPrefix.length()-2] == '|'){ identationPrefix.pop_back(); identationPrefix += " "; }

    for (int i = 0; i < this->children.size(); i++) {
        if (i == this->children.size()-1) { this->children[i]->print(identationPrefix + "'-"); }
        else { this->children[i]->print(identationPrefix + "|-"); }
    }
  }
};

class Variable {
    public:
        Variable(Node* root){
            this->traverse(root);
        };
        std::string name;
        std::string type;

        void traverse(Node *tree) {
            if (tree->rule != "dcl type ID") { throw runtime_error("Variable.h: ROOT OF TREE IS NOT \"DCL TYPE ID\""); }
            
            Node *typeChild = tree->getChild("type");
            if (typeChild->getChild("STAR") != nullptr) { this->type = "int*"; }
            else { this->type = "int"; }

            Node *nameChild = tree->getChild("ID");
            this->name = nameChild->lexeme;
        }
};


#endif