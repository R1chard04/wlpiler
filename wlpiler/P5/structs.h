#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

struct Node {
    std::string type = "";
    std::vector<std::string> rhs = {}; // stores symbols on the rhs of the CFG rule in a nonterminal node
    std::string rule = ""; // only used in nonterminal nodes
    std::string lhs = ""; // only used in nonterminal nodes
    std::string kind = ""; // only used in terminal nodes
    std::string lexeme = ""; // only used in terminal nodes
    std::vector<Node*> children = {};
    Node(std::string kind, std::string lexeme) : kind(kind), lexeme(lexeme) {}
    ~Node() { for( auto &c : children ) { delete c; } }

    Node(std::istream &in = std::cin) {
        std::string line = ""; 
        getline(std::cin, line);
        // std::cout << line << endl;
        stringstream ss(line);
        std::string currWord = "";
        
        bool firstPart = true;
        while (ss >> currWord) {
            // cout << "currWord: " << currWord << endl;
            if (firstPart) {
                firstPart = false;
                lhs = currWord;
                rule += currWord;
            }
            else if (currWord == ".EMPTY") { break; }
            else if (currWord == ":") { ss >> currWord; type = currWord; break; }
            else {
                this->rhs.push_back(currWord);
                this->rule += " " + currWord;
            }
        }
        
        for (int i = 0; i < rhs.size(); i++) {
            if (isupper(rhs[i][0])) { // Terminal
                std::string nextLine, k, l, currStr;
                getline(cin, nextLine);
                stringstream ss(nextLine); ss >> k; ss >> l;
                Node *tmp = new Node(k, l); this->children.push_back(tmp);
            }
            else { Node* child = new Node(std::cin); this->children.push_back(child); } // Non-terminal
        }
        return;
    }

    Node *getChild (std::string str, int n = 1) {
      for (int i = 0; i < this->children.size(); i++) {
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
    if (rule != "") { std::cout << identationPrefix << this->lhs << " " << this->rule; } // non-terminal
    else { std::cout << identationPrefix << this->kind << " " << this->lexeme; } // terminal
    
    if (this->type != "") { std::cout << " ; type:" << this->type; }
    cout << endl;

    if (identationPrefix[identationPrefix.length()-2] == '\'') { identationPrefix.pop_back(); identationPrefix.pop_back(); identationPrefix += "  "; }
    else if (identationPrefix[identationPrefix.length()-2] == '|'){ identationPrefix.pop_back(); identationPrefix += " "; }

    for (int i = 0; i < this->children.size(); i++) {
        if (i == this->children.size()-1) { this->children[i]->print(identationPrefix + "'-"); }
        else { this->children[i]->print(identationPrefix + "|-"); }
    }
  }
};

void Add(int d, int s, int t) {
    std::cout << "add $" << d << ", $" << s << ", $" << t << endl;
}

void Sub(int d, int s, int t) {
    std::cout << "sub $" << d << ", $" << s << ", $" << t << endl;
}

void Mult(int s, int t) {
    std::cout << "mult $" << s << ", $" << t << endl;
}

void Multu(int s, int t) {
    std::cout << "multu $" << s << ", $" << t << endl;
}

void Div(int s, int t) {
    std::cout << "div $" << s << ", $" << t << endl;
}

void Divu(int s, int t) {
    std::cout << "divu $" << s << ", $" << t << endl;
}

void Mfhi(int d) {
    std::cout << "mfhi $" << d << endl;
}

void Mflo(int d) {
    std::cout << "mflo $" << d << endl;
}

void Lis(int d) {
    std::cout << "lis $" << d << endl;
}

void Slt(int d, int s, int t) {
    std::cout << "slt $" << d << ", $" << s << ", $" << t << endl;
}

void Sltu(int d, int s, int t) {
    std::cout << "sltu $" << d << ", $" << s << ", $" << t << endl;
}

void Jr (int s) {
    std::cout << "jr $" << s << endl;
}

void Jalr(int s) {
    std::cout << "jalr $" << s << endl;
}

void Beq(int s, int t, string label) {
    std::cout << "beq $" << s << ", $" << t << ", " + label << endl;
}

void Bne(int s, int t, string label) {
    std::cout << "bne $" << s << ", $" << t << ", " + label << endl;
}

void Lw(int t, int i, int s) {
    std::cout << "lw $" << t << ", " << i << "($" << s << ")" << endl;
}

void Sw(int t, int i, int s) {
    std::cout << "sw $" << t << ", " << i << "($" << s << ")" << endl;
}

void Word(int i) {
    std::cout << ".word " << i << endl;
}
void Word(std::string label) {
    std::cout << ".word " + label << endl;
}
void Label(std::string name) {
    std::cout << name + ":" << endl;
}

void Push(int s) {
    Sw(s, -4, 30); Sub(30, 30, 4);
}

void Pop(int d) {
    Add(30, 30, 4); Lw(d, -4, 30);
}

void Pop() {
    Add(30, 30, 4);
}