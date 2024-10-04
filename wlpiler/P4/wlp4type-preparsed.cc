#include <stdio.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>

#include "structs.h"

using namespace std;

void collectProcedures(Node *root, ProcedureTable &procTable);
void annotateTypes (Node *root, Procedure *currProc, ProcedureTable &procTable);
vector<string> getSignature(Node* root, vector<string> sig);
void checkStatements(Node *root);

int main(){
    ProcedureTable procTable;

    Node* root = new Node(std::cin);
    // root->print(""); // print out the tree for debugging 
    // cout << "Collecting procedures" << endl;
    collectProcedures(root, procTable);

    // procTable.printProcTable();
    delete root;
    // for(auto it=procTable.procTable.begin(); it!=procTable.procTable.end(); it++) {
    //     delete it->second;
    // }
    // for (Node* tmp : procTable.procTable)

    return 0;
}

void collectProcedures(Node *root, ProcedureTable &procTable) {
    // cout << "Root has rule: " << root->rule << " with lhs: " << root->lhs << endl;
    if (root->lhs == "procedure" || root->lhs == "main") {
        Procedure *proc = new Procedure(root);
        // cout << "Adding procedure to procedure table" << endl;
        procTable.add(proc);
        annotateTypes(root, proc, procTable);
        
        if (root->lhs == "main") {
            if (proc->signature.size() != 2) { throw runtime_error("ERROR: MAIN DOES NOT HAVE 2 PARAMETERS"); }
            if (proc->signature[1] != "int") { throw runtime_error("ERROR: SECOND PARAM OF MAIN IS NOT TYPE INT"); }
        }

        if (root->getChild("expr")->type != "int") {throw runtime_error("ERROR: RETURN TYPE IS NOT INT"); }

        checkStatements(root);
    }
    else {
        for (Node *child : root->children){
            collectProcedures(child, procTable);
        }
    }
}

void annotateTypes (Node *root, Procedure *currProc, ProcedureTable &procTable) {
    for (Node *child : root->children) { annotateTypes(child, currProc, procTable); }
    if (root->rule == "expr term") {
        root->type = root->getChild("term")->type;
    }
    else if (root->rule == "expr expr PLUS term") {
        string type1 = root->getChild("expr", 1)->type;
        string type2 = root->getChild("term", 1)->type;
        if (type1 == "int" && type2 == "int") { root->type = "int"; }
        else if ((type1 == "int*" && type2 == "int") || (type1 == "int" && type2 == "int*")) { root->type = "int*"; }
        else { throw runtime_error("ERROR: expr expr PLUS term RULE IS WRONG"); }
    }
    else if (root->rule == "expr expr MINUS term") {
        string type1 = root->getChild("expr", 1)->type;
        string type2 = root->getChild("term", 1)->type;
        if (type1 == "int" && type2 == "int") { root->type = "int"; }
        else if (type1 == "int*" && type2 == "int") { root->type = "int*"; }
        else if (type1 == "int*" && type2 == "int*") { root->type = "int"; }
        else { throw runtime_error("ERROR: expr expr MINUS term RULE IS WRONG"); }
    }
    else if (root->rule == "term term STAR factor") {
        string type1 = root->getChild("term", 1)->type;
        string type2 = root->getChild("factor", 1)->type;
        if (type1 == "int" && type2 == "int") { root->type = "int"; }
        else { throw runtime_error("ERROR: term term STAR factor RULE IS WRONG"); }
    }
    else if (root->rule == "term term SLASH factor") {
        string type1 = root->getChild("term", 1)->type;
        string type2 = root->getChild("factor", 1)->type;
        if (type1 == "int" && type2 == "int") { root->type = "int"; }
        else { throw runtime_error("ERROR: term term SLASH factor RULE IS WRONG"); }
    }
    else if (root->rule == "term factor") {
        root->type = root->getChild("factor")->type;     
    }
    else if (root->rule == "term term PCT factor") { // PCT == %
        string type1 = root->getChild("term", 1)->type;
        string type2 = root->getChild("factor", 1)->type;
        if (type1 == "int" && type2 == "int") { root->type = "int"; }
        else { throw runtime_error("ERROR: term term PCT factor RULE IS WRONG"); }
    } 
    else if (root->rule == "factor ID") {
        string idName = root->getChild("ID")->lexeme;
        Variable *tmp = currProc->table.get(idName); // check that ID exists in current procedure
        string type1 = tmp->type;
        if (type1 == "int" || type1 == "int*") { root->type = type1; }
        else { throw runtime_error("ERROR: factor ID RULE IS WRONG"); }
    }
    else if (root->rule == "factor NUM") {
        root->type = "int";
    }
    else if (root->rule == "factor NULL") {
        root->type = "int*";
    }
    else if (root->rule == "factor LPAREN expr RPAREN") {
        root->type = root->getChild("expr")->type;
    }
    else if (root->rule == "factor AMP lvalue") {
        string type1 = root->getChild("lvalue", 1)->type;
        if (type1 == "int") { root->type = "int*"; }
        else { throw runtime_error("ERROR: factor AMP lvalue RULE IS WRONG"); }
    }
    else if (root->rule == "factor STAR factor") {
        string type1 = root->getChild("factor", 1)->type;
        if (type1 == "int*") { root->type = "int"; }
        else { throw runtime_error("ERROR: factor STAR factor RULE IS WRONG"); }
    }
    else if (root->rule == "factor NEW INT LBRACK expr RBRACK") {
        string type1 = root->getChild("expr", 1)->type;
        if (type1 == "int") { root->type = "int*"; }
        else { throw runtime_error("ERROR: factor NEW INT LBRACK expr RBRACK RULE IS WRONG"); }
    }
    else if (root->rule == "factor ID LPAREN RPAREN") {
        // check that ID refers to a declared procedure
        string idName = root->getChild("ID")->lexeme;
        if (currProc->table.symbolTable.find(idName) != currProc->table.symbolTable.end()) { throw runtime_error("ERROR: ID'S LEXEME IS IN SYMBOL TABLE"); } // ID's lexeme IS NOT in local variable table
        Procedure *calledProc = procTable.get(idName); // ID's lexeme IS in procedure table

        // check that signature matches the sequence of arguments (in this case, no arguments)
        if (calledProc->signature.size() != 0) { throw runtime_error("ERROR: SIGNATURE SIZE IS NOT ZERO"); }
        root->type = "int";
    }
    else if (root->rule == "factor ID LPAREN arglist RPAREN") {
        string idName = root->getChild("ID")->lexeme;
        if (currProc->table.symbolTable.find(idName) != currProc->table.symbolTable.end()) { throw runtime_error("ERROR: ID'S LEXEME IS IN SYMBOL TABLE"); } // ID's lexeme IS NOT in local variable table
        Procedure *calledProc = procTable.get(idName); // ID's lexeme IS in procedure table

        // check that signature matches the sequence of arguments
        vector<string> idSignature = getSignature(root->getChild("arglist"), {});
        if (idSignature.size() != calledProc->signature.size()) { throw runtime_error("ERROR: SIGNATURE SIZES DO NOT MATCH (FUNCTION AND ITS CALL)"); }
        for (int i = 0; i < idSignature.size(); i++) {
            // cout << "calling proc: " << idSignature[i] << "; actual proc: " << calledProc->signature[i] << endl;
            if (idSignature[i] != calledProc->signature[i]) { throw runtime_error("ERROR: SIGNATURES DO NOT MATCH (FUNCTION AND ITS CALL)"); }
        }
        root->type = "int";
    }
    else if (root->rule == "lvalue ID") {
        string idName = root->getChild("ID")->lexeme;
        Variable *tmp = currProc->table.get(idName); // check that ID exists in current procedure
        if (tmp->type == "int" || tmp->type == "int*") { root->type = tmp->type; }
        else { throw runtime_error("ERROR: lvalue ID RULE IS WRONG"); }
    }
    else if (root->rule == "lvalue STAR factor") {
        string type1 = root->getChild("factor", 1)->type;
        if (type1 == "int*") { root->type = "int"; }
        else { throw runtime_error("ERROR: lvalue STAR factor RULE IS WRONG"); }
    }
    else if (root->rule == "lvalue LPAREN lvalue RPAREN") {
        root->type = root->getChild("lvalue")->type;
    }
}

vector<string> getSignature(Node* root, vector<string> sig) {
    Node* tmp = root;
    while (tmp->rule == "arglist expr COMMA arglist") {
        sig.push_back(tmp->getChild("expr")->type);
        tmp = tmp->getChild("arglist");
    }
    sig.push_back(root->getChild("expr")->type);
    return sig;
}

void checkStatements(Node *root) {
    for (Node *child : root->children) { checkStatements(child); }
    
    // When statement derives lvalue BECOMES expr SEMI, the derived lvalue and the derived expr must have the same type.
    if (root->rule == "statement lvalue BECOMES expr SEMI") {
        if (root->getChild("lvalue")->type != root->getChild("expr")->type) { throw runtime_error("ERROR: LVALUE AND EXPR TYPES DO NOT MATCH"); }
    }
    // When statement derives PRINTLN LPAREN expr RPAREN SEMI, the derived expr must have type int.
    else if (root->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        if (root->getChild("expr")->type != "int") { throw runtime_error("ERROR: PRINTLN EXPR IS NOT TYPE INT"); }
    }
    // When statement derives DELETE LBRACK RBRACK expr SEMI, the derived expr must have type int*.
    else if (root->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        if (root->getChild("expr")->type != "int*") { throw runtime_error("ERROR: DELETE EXPR IS NOT TIME INT*"); }
    }
    // Whenever test directly derives a sequence containing two exprs, they must both have the same type.
    else if (root->rule == "test expr EQ expr" || root->rule == "test expr NE expr" || root->rule == "test expr LT expr" || 
            root->rule == "test expr LE expr" || root->rule == "test expr GE expr" || root->rule == "test expr GT expr") {
        string type1 = root->getChild("expr", 1)->type;
        string type2 = root->getChild("expr", 2)->type;
        if (type1 != type2) { throw runtime_error("ERROR: TEST EXPRS DO NOT HAVE SAME TYPE"); }
    }
}