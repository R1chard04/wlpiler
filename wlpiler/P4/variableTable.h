#ifndef VARIABLETABLE_H
#define VARIABLETABLE_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "variable.h"

using namespace std;

class VariableTable {
    public:
        VariableTable(){
            symbolTable = {};
        };
        unordered_map<string, Variable> symbolTable;

        void add(Variable var) {
            if (symbolTable.find(var.name) != symbolTable.end()) { throw runtime_error("DUPLICATE VARIABLE DECLARATION"); }
            symbolTable[var.name] = var;
        }

        Variable get(string str) {
            if (symbolTable.find(str) == symbolTable.end()) { throw runtime_error("USE OF UNDECLARED VARIABLE"); }
            return symbolTable[str];
        }
};

#endif