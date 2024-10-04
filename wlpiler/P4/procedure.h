#ifndef PROCEDURE_H
#define PROCEDURE_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "variableTable.h"

using namespace std;

class Procedure {
    public:
        Procedure(Node* root){};
        string name;
        vector<string> signature;
        VariableTable table;

        void traverse(Node *tree) {
            Node *procNameNode = tree->getChild("ID");
            this->name = procNameNode->lexeme;

        }
};

#endif