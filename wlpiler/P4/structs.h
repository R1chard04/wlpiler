#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

struct Node {
    std::string type = "";
    std::string rule = ""; // only used in nonterminal nodes
    std::string lhs = ""; // only used in nonterminal nodes
    std::string kind = ""; // only used in terminal nodes
    std::string lexeme = ""; // only used in terminal nodes
    std::vector<Node*> children = {};
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
    // cout << "getChild is getting child with str: " << str << endl;
      for (int i = 0; i < this->children.size(); i++) {
          if (this->children[i]->rule == "") { // terminal
            //   cout << "terminal: " << this->children[i]->kind << endl;
              if (this->children[i]->kind == str) { n -= 1; }
          }
          else { // non-terminal
            //   cout << "non-terminal: " << children[i]->lhs << endl;
              if (this->children[i]->lhs == str) { n -= 1; }
          }
          if (n == 0) { return this->children[i]; }
      }
    //   cout << str << " does not exist as a child" << endl;
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

struct Variable {
    Variable(Node* root){
        this->traverse(root);
    };
    std::string name;
    std::string type;
    ~Variable () {}

    void traverse(Node *tree) {
        if (tree->rule != "dcl type ID") { throw runtime_error("ERROR: Variable.h: ROOT OF TREE IS NOT \"DCL TYPE ID\""); }
        
        Node *typeChild = tree->getChild("type");
        if (typeChild->getChild("STAR") != nullptr) { this->type = "int*"; }
        else { this->type = "int"; }

        Node *nameChild = tree->getChild("ID");
        this->name = nameChild->lexeme;
    }
};

struct VariableTable {
    VariableTable() {
        symbolTable = {};
    };
    unordered_map<string, Variable*> symbolTable;
    ~VariableTable() {
        cout << "variabletable destructor is called" << endl;
        // for(auto it=symbolTable.begin(); it!=symbolTable.end(); it++) {
        //     delete it->second;
        // }
        for (auto& pair : symbolTable) {
            delete pair.second;
        }
    }

    void add(Variable* var) {
        if (symbolTable.find(var->name) != symbolTable.end()) {
            cout << "HEREREERE" << endl;
            throw runtime_error("ERROR: DUPLICATE VARIABLE DECLARATION"); 
        }
        symbolTable[var->name] = var;
    }

    Variable* get(string str) {
        if (symbolTable.find(str) == symbolTable.end()) { throw runtime_error("ERROR: USE OF UNDECLARED VARIABLE"); }
        return symbolTable[str];
    }
};

struct Procedure {
    Procedure(Node* root){
        extractInfo(root);
    };
    ~Procedure() {}
    string name;
    vector<string> signature;
    VariableTable table;
    bool mainProg;

    void extractInfo(Node *tree) {
        this->table = {};
        this->signature = {};
        if (tree->lhs == "main") { this->mainProg = true; }
        else if (tree->lhs == "procedure") { this->mainProg = false; }
        else { throw runtime_error("ERROR: NOT A PROCEDURE"); }
        
        // name and signature (which includes variable table)
        // cout << "Getting procedure name and params" << endl;
        if (this->mainProg) {
            Node *progNameNode = tree->getChild("WAIN");
            this->name = progNameNode->lexeme;
            // cout << "name of procedure is: " << progNameNode->lexeme << endl;

            Node *param1 = tree->getChild("dcl", 1); 
            extractSignature(param1);

            // cout << "extracting signature 2" << endl;
            Node *param2 = tree->getChild("dcl", 2); 
            extractSignature(param2);
        }
        else {
            Node *progNameNode = tree->getChild("ID");
            this->name = progNameNode->lexeme;

            Node *progParams = tree->getChild("params");
            extractSignature(progParams);
            // if (progParams->children.size() == 0) { this->signature = {}; }
            // else { extractSignature(progParams); }
        }

        // dcls
        // cout << "Getting procedure DCLs" << endl;
        Node *progDcls = tree->getChild("dcls");
        traverse(progDcls);

        //statements
        // cout << "Getting procedure statements" << endl;
        Node *progStatements = tree->getChild("statements");
        traverse(progStatements);

        // return expr
        // cout << "Getting procedure return expressions" << endl;
        Node *progReturnExpr = tree->getChild("expr");
        traverse(progStatements);
        
    }

    void extractSignature (Node *root) {
        // cout << "extractSignature is getting root with rule: " << root->rule << endl;
        if (root->rule == "dcl type ID") {
            Variable *dcl = new Variable(root);
            signature.push_back(dcl->type);
            this->table.add(dcl);
        }
        else {
            for (Node *child : root->children) { extractSignature(child); }
        }
    }

    void traverse(Node *root) {
        if (root->rule == "dcls dcls dcl BECOMES NUM SEMI") {
            Variable *dcl = new Variable(root->getChild("dcl"));
            if (dcl->type != "int") { throw runtime_error("ERROR: dcls dcls dcl BECOMES NUM SEMI RULE'S DCL IS NOT TYPE INT"); }
            this->table.add(dcl);
            for (Node *child : root->children) { traverse(child); }
        }
        else if (root->rule == "dcls dcls dcl BECOMES NULL SEMI") {
            Variable *dcl = new Variable(root->getChild("dcl"));
            if (dcl->type != "int*") { throw runtime_error("ERROR: dcls dcls dcl BECOMES NUM SEMI RULE'S DCL IS NOT TYPE INT*"); }
            this->table.add(dcl);
            for (Node *child : root->children) { traverse(child); }
        }
        else {
            for (Node *child : root->children) { traverse(child); }
        }
    }
};

struct ProcedureTable {
    ProcedureTable(){
        procTable = {};
    };
    ~ProcedureTable() {
        cout << "procedure table destructor is called" << endl;
        // for(auto it=procTable.begin(); it!=procTable.end(); it++) {
        //     delete it->second;
        // }
        for (auto& pair : procTable) {
            delete pair.second;
        }
    }
    unordered_map<string, Procedure*> procTable;

    void add(Procedure *proc) {
        if (procTable.find(proc->name) != procTable.end()) { throw runtime_error("ERROR: DUPLICATE PROCEDURE DECLARATION"); }
        procTable[proc->name] = proc;
    }

    Procedure *get(string procName) {
        if (procTable.find(procName) == procTable.end()) { throw runtime_error("ERROR: UNDECLARED PROCEDURE CALLED"); }
        return procTable[procName];
    }

    void printProcTable () {
        
        for (auto const &i : procTable) {
            cout << i.first << endl << "Signature: " << i.second->signature.size() << endl;
            for (int x = 0; x < i.second->signature.size(); x++){
                cout << i.second->signature[x] << " ";
            }
            cout << endl;

            unordered_map<string, Variable*> st = i.second->table.symbolTable;
            cout << "Variable Names : " << endl;
            for (auto const &j : st) {
                cout << j.first << " with type: ";
                const Variable *v = j.second;
                cout << v->type << endl;
            }
        }
    }

};