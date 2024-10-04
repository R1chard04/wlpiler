#include <stdio.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>

#include "structs.h"

using namespace std;

void constructOffsetTable(Node* main, unordered_map<string, int> &table);
void traverseDcls(Node *dcls, unordered_map<string, int> &table, int &fpOffset);
void fillProcNumArguments (Node *root, unordered_map<string, int> &procArgumentNum);
void generateStatements(Node *root, unordered_map<string, int> &table, int &condCounter, unordered_map<string, int> &procArgumentNum);
void generateNonWainProc(Node *root, unordered_map<string, int> &table, int &condCounter, unordered_map<string, int> &procArgumentNum);

int main(){
    unordered_map<string, int> offsetTable;
    unordered_map<string, int> procToNumArguments;
    int conditionalCounter = 0;
    Node *root = new Node(std::cin);
    // root->print(""); // print out the tree for debugging

    cout << ".import print" << endl; cout << ".import init" << endl; cout << ".import new" << endl; cout << ".import delete" << endl;
    Lis(4); Word(4); // load 4 into $4
    Lis(11); Word(1); // load 1 into $11

    Node *mainNode = root->getChild("procedures");
    while (mainNode->rule == "procedures procedure procedures") {
        fillProcNumArguments(mainNode->getChild("procedure"), procToNumArguments);
        mainNode = mainNode->getChild("procedures");
    }   
    mainNode = mainNode->getChild("main");
    
    // cout << "-------" << endl;
    // for (auto const &i : procToNumArguments) {
    //     cout << "key: " << i.first << "; value: " << i.second << endl;
    // }
    // cout << "--------" << endl;

    constructOffsetTable(mainNode, offsetTable); // variable declaration and initilization

    // call init procedure
    if (mainNode->getChild("dcl", 1)->getChild("type")->rule == "type INT STAR") {
        Push(31);
        Lis(3);
        Word("init");
        Jalr(3);
        Pop(31);
    }
    else if (mainNode->getChild("dcl", 1)->getChild("type")->rule == "type INT") {
        Add(2, 0, 0); //pass value 0 in $2 (indicating that there is no input array)
        Push(31);
        Lis(3);
        Word("init");
        Jalr(3);
        Pop(31);
    }

    generateStatements(mainNode->getChild("statements"), offsetTable, conditionalCounter, procToNumArguments);

    generateStatements(mainNode->getChild("expr"), offsetTable, conditionalCounter, procToNumArguments);

    for (auto const &i : offsetTable) { Pop(); }
    Jr(31);

    Node *tmpProc = root->getChild("procedures");
    while (tmpProc->rule == "procedures procedure procedures") {
        generateNonWainProc(tmpProc->getChild("procedure"), offsetTable, conditionalCounter, procToNumArguments);
        tmpProc = tmpProc->getChild("procedures");
    }

    // cout << "OFFSET TABLE" << endl;
    // for (auto const &i : offsetTable) {
    //     cout << "key: " << i.first << "; value: " << i.second << endl;
    // }

    delete root;
    // try {
    //     delete root;

    // } catch(std::runtime_error &e) {
    //     std::cerr << "ERROR: " << e.what() << "\n";
    //     delete root;
    //     return 1;
    // }
    return 0;
}

void fillProcNumArguments (Node *root, unordered_map<string, int> &procArgumentNum) {
    string idName = root->getChild("ID")->lexeme;
    Node *tmp = root->getChild("params");
    int num = 0;
    if (tmp->rule == "params .EMPTY") { procArgumentNum[idName] = 0; return; }
    else if (tmp->rule == "params paramlist") {
        tmp = tmp->getChild("paramlist");
        while (tmp->rule == "paramlist dcl COMMA paramlist") {
            num += 1;
            tmp = tmp->getChild("paramlist");
        }
        procArgumentNum[idName] = num + 1;
    }
}

void generateNonWainProc(Node *root, unordered_map<string, int> &table, int &condCounter, unordered_map<string, int> &procArgumentNum) {
    // "procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"
    string procName = root->getChild("ID")->lexeme;
    Label("P" + procName);
    unordered_map<string, int> procOffsetTable;
    // 1. Set up the frame pointer, compute the table of offsets, and push local variables on the stack as necessary.
    Sub(29, 30, 4);
    Node *tmp = root->getChild("params");
    for (int i = procArgumentNum[procName]; i > 0; i--) {
        // cout << "i: " << i << endl;
        tmp = tmp->getChild("paramlist");
        procOffsetTable[tmp->getChild("dcl")->getChild("ID")->lexeme] = i*4;
    }
    int fpOffset = 0;
    traverseDcls(root->getChild("dcls"), procOffsetTable, fpOffset);
    // 2. Generate code for the statements in the body of the procedure.
    generateStatements(root->getChild("statements"), procOffsetTable, condCounter, procArgumentNum);
    // 3. Generate code for the return expression.
    generateStatements(root->getChild("expr"), procOffsetTable, condCounter, procArgumentNum);
    // 4. Clean up the stack and return.
    fpOffset = (fpOffset/4)*-1;
    for (int i = 0; i < fpOffset; i++) { Pop(); }

    // for (auto const &i : procOffsetTable) {
    //      cout << "key: " << i.first << "; value: " << i.second << endl;
    // }

    Jr(31);
}

void generateStatements(Node *root, unordered_map<string, int> &table, int &condCounter, unordered_map<string, int> &procArgumentNum) {
    if (root->rule == "expr term") {
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
    }
    else if (root->rule == "expr expr PLUS term") {
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
        Pop(5);
        if (root->getChild("expr")->type == "int" && root->getChild("term")->type == "int") {
            Add(3, 5, 3);
        }
        else if (root->getChild("expr")->type == "int*" && root->getChild("term")->type == "int") {
            Mult(3, 4);
            Mflo(3);
            Add(3, 5, 3);
        }
        else if (root->getChild("expr")->type == "int" && root->getChild("term")->type == "int*") {
            Mult(5, 4);
            Mflo(5);
            Add(3, 5, 3);
        }
        else { throw runtime_error("expr expr PLUS term"); }
    }
    else if (root->rule == "expr expr MINUS term") {
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
        Pop(5);
        if (root->getChild("expr")->type == "int" && root->getChild("term")->type == "int") {
            Sub(3, 5, 3);
        }
        else if (root->getChild("expr")->type == "int*" && root->getChild("term")->type == "int") {
            Mult(3, 4);
            Mflo(3);
            Sub(3, 5, 3);
        }
        else if (root->getChild("expr")->type == "int*" && root->getChild("term")->type == "int*") {
            Sub(3, 5, 3);
            Div(3, 4);
            Mflo(3);
        }
    }
    else if (root->rule == "term factor") {
        generateStatements(root->getChild("factor"), table, condCounter, procArgumentNum);
    }
    else if (root->rule == "term term STAR factor") {
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("factor"), table, condCounter, procArgumentNum);
        Pop(5);
        Mult(3, 5);
        Mflo(3);
    }
    else if (root->rule == "term term SLASH factor") {
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("factor"), table, condCounter, procArgumentNum);
        Pop(5);
        Div(5, 3);
        Mflo(3);
    }
    else if (root->rule == "term term PCT factor") {
        generateStatements(root->getChild("term"), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("factor"), table, condCounter, procArgumentNum);
        Pop(5);
        Div(5, 3);
        Mfhi(3);
    }
    else if (root->rule == "factor ID") {
        string idName = root->getChild("ID")->lexeme;
        int offset = table[idName];
        Lw(3, offset, 29);
    }
    else if (root->rule == "factor NUM") {
        string num = root->getChild("NUM")->lexeme;
        Lis(3);
        Word(num);
    }
    else if (root->rule == "factor NULL") {
        Lis(3);
        Word(1);
    }
    else if (root->rule == "factor LPAREN expr RPAREN") {
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum);
    }
    else if (root->rule == "factor AMP lvalue") {
        Node *lvalueRoot = root->getChild("lvalue");

        if (lvalueRoot->rule == "lvalue LPAREN lvalue RPAREN") {
            while (lvalueRoot->rule == "lvalue LPAREN lvalue RPAREN") { lvalueRoot = lvalueRoot->getChild("lvalue"); }
        }

        if (lvalueRoot->rule == "lvalue ID") {
            string idName = lvalueRoot->getChild("ID")->lexeme;
            int offset = table[idName];
            Lis(3); // list $3
            Word(offset); // .word <offset of ID>
            Add(3, 29, 3); // add $3, $29, $3
        }
        else if (lvalueRoot->rule == "lvalue STAR factor") {
            generateStatements(lvalueRoot->getChild("factor"), table, condCounter, procArgumentNum); // code(AMP STAR factor) = code(factor)
        }
    }
    else if (root->rule == "factor STAR factor") {
        generateStatements(root->getChild("factor"), table, condCounter, procArgumentNum);
        Lw(3, 0, 3);
    }
    else if (root->rule == "factor NEW INT LBRACK expr RBRACK") {
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum); // $3 = size of array 
        Add(1, 3, 0); // move $1 = $3
        Push(31);
        Lis(3);
        Word("new");
        Jalr(3);
        Pop(31);
        Bne(3, 0, "label" + to_string(condCounter)); // check if allocation failed; if it failed, $3 = 0
        Add(3, 3, 11);
        Label("label" + to_string(condCounter));
        condCounter += 1;
    }
    else if (root->rule == "statements statements statement") {
        generateStatements(root->getChild("statements"), table, condCounter, procArgumentNum);
        generateStatements(root->getChild("statement"), table, condCounter, procArgumentNum);
    }
    else if (root->rule == "statements") { return; }
    else if (root->rule == "statement lvalue BECOMES expr SEMI") {
        Node *lvalueRoot = root->getChild("lvalue");

        if (lvalueRoot->rule == "lvalue LPAREN lvalue RPAREN") {
            while (lvalueRoot->rule == "lvalue LPAREN lvalue RPAREN") {
                lvalueRoot = lvalueRoot->getChild("lvalue");
            }
        }
        if (lvalueRoot->rule == "lvalue ID") {
            string idName = lvalueRoot->getChild("ID")->lexeme;
            int offset = table[idName];
            generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum); // code(expr)
            Sw(3, offset, 29); // sw $3, <offset of ID>($29)
        }
        else if (lvalueRoot->rule == "lvalue STAR factor") {
            generateStatements(lvalueRoot->getChild("factor"), table, condCounter, procArgumentNum);
            Push(3);
            generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum); // code(expr)
            Pop(5);
            Sw(3, 0, 5);
        }
    }
    else if (root->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        Label("label" + to_string(condCounter));
        string endLabel1 = "endlabel" + to_string(condCounter);
        condCounter += 1;
        string label2 = "label" + to_string(condCounter);
        condCounter += 1;
        
        generateStatements(root->getChild("test"), table, condCounter, procArgumentNum);
        Beq(3, 0, endLabel1);
        
        generateStatements(root->getChild("statements", 1), table, condCounter, procArgumentNum);
        Beq(0, 0, label2);

        Label(endLabel1);
        generateStatements(root->getChild("statements", 2), table, condCounter, procArgumentNum);

        Label(label2);
        
    }
    else if (root->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        string startLabel = "label" + to_string(condCounter);
        string endLabel = "endlabel" + to_string(condCounter);
        Label(startLabel);
        condCounter += 1;
        
        generateStatements(root->getChild("test"), table, condCounter, procArgumentNum);
        Beq(3, 0, endLabel);
        
        generateStatements(root->getChild("statements"), table, condCounter, procArgumentNum);

        Beq(0, 0, startLabel);

        Label(endLabel);
    }
    else if (root->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") { 
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum);
        Add(1, 3, 0); // move print value to $1
        Push(31);
        Lis(3);
        Word("print");
        Jalr(3);
        Pop(31);
    }
    else if (root->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        generateStatements(root->getChild("expr"), table, condCounter, procArgumentNum);
        Beq(3, 11, "label" + to_string(condCounter));
        Add(1, 3, 0); //move array address from $3 to $1
        Push(31);
        Lis(3);
        Word("delete");
        Jalr(3);
        Pop(31);
        Label("label" + to_string(condCounter));
        condCounter += 1;
    }
    else if (root->rule == "test expr EQ expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        Sub(5, 3, 5);
        Add(3, 0, 0);
        Beq(5, 0, "label" + to_string(condCounter));
        Sub(3, 3, 11);
        Label("label" + to_string(condCounter));
        Add(3, 3, 11);
        condCounter += 1;
    }
    else if (root->rule == "test expr NE expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        Sub(5, 3, 5);
        Add(3, 0, 0);
        Bne(5, 0, "label" + to_string(condCounter));
        Sub(3, 3, 11);
        Label("label" + to_string(condCounter));
        Add(3, 3, 11);
        condCounter += 1;
    }
    else if (root->rule == "test expr LT expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        if (root->getChild("expr", 1)->type == "int" && root->getChild("expr", 2)->type == "int") { Slt(3, 5, 3); }
        else if (root->getChild("expr", 1)->type == "int*" && root->getChild("expr", 2)->type == "int*") { Sltu(3, 5, 3); }
    }
    else if (root->rule == "test expr LE expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        Sub(3, 5, 3); // $3 = $5 - $3 (if expr1 < expr2: $3 <= 0)
        Sub(3, 3, 11); // $3 -= 1 ($3 < 0)
        Slt(3, 3, 0); // (if $3 < 0: $3 == 1)
    }
    else if (root->rule == "test expr GE expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        Sub(3, 3, 5); // $3 = $3 - $5 (if expr1 > expr2: $3 <= 0)
        Sub(3, 3, 11); // $3 -= 1 ($3 < 0)
        Slt(3, 3, 0); // (if $3 < 0: $3 == 1)
    }
    else if (root->rule == "test expr GT expr") {
        generateStatements(root->getChild("expr", 1), table, condCounter, procArgumentNum);
        Push(3);
        generateStatements(root->getChild("expr", 2), table, condCounter, procArgumentNum);
        Pop(5);
        if (root->getChild("expr", 1)->type == "int" && root->getChild("expr", 2)->type == "int") { Slt(3, 3, 5); }
        else if (root->getChild("expr", 1)->type == "int*" && root->getChild("expr", 2)->type == "int*") { Sltu(3, 3, 5); }
    }
    else if (root->rule == "factor ID LPAREN RPAREN") { 
        Push(31); Push(29);
        string calledProcName = root->getChild("ID")->lexeme;
        Lis(3);
        Word("P" + calledProcName);
        Jalr(3);
        Pop(29); Pop(31);
        // procArgumentNum[calledProcName] = 0;
    }
    else if (root->rule == "factor ID LPAREN arglist RPAREN") { 
        Push(31); Push(29);
        string calledProcName = root->getChild("ID")->lexeme;
        int num = 1;
        // cout << "proc name: " << calledProcName <<  " with rule: " << root->getChild("arglist")->rule << endl;
        if (root->getChild("arglist")->rule == "arglist expr COMMA arglist") {
            Node *tmpArglistNode = root->getChild("arglist");
            while(tmpArglistNode->rule == "arglist expr COMMA arglist") {
                generateStatements(tmpArglistNode->getChild("expr"), table, condCounter, procArgumentNum);
                Push(3);
                tmpArglistNode = tmpArglistNode->getChild("arglist"); 
                num += 1;
            }
            generateStatements(tmpArglistNode->getChild("expr"), table, condCounter, procArgumentNum);
            Push(3);
            // procArgumentNum[calledProcName] = num;
        }
        else if (root->getChild("arglist")->rule == "arglist expr") {
            generateStatements(root->getChild("arglist")->getChild("expr"), table, condCounter, procArgumentNum);
            Push(3);
            // cout << "proc name: " << calledProcName <<  " has #args: " << num << endl;
            // procArgumentNum[calledProcName] = num;
        }
        
        Lis(3);
        Word("P" + calledProcName);
        Jalr(3);
        for (int i = 0; i < num; i++) { Pop(); }
        Pop(29); Pop(31);
    }
}


void constructOffsetTable(Node* main, unordered_map<string, int> &table) {
    string id1 = main->getChild("dcl", 1)->getChild("ID")->lexeme;
    table[id1] = 8;
    Push(1);
    string id2 = main->getChild("dcl", 2)->getChild("ID")->lexeme;
    table[id2] = 4;
    Push(2);

    Sub(29, 30, 4); // set frame pointer
    int framePointerOffset = 0;
    traverseDcls(main->getChild("dcls"), table, framePointerOffset);
}

void traverseDcls(Node *dcls, unordered_map<string, int> &table, int &fpOffset){
    if (dcls->rule == "dcls dcls dcl BECOMES NUM SEMI") { //type is guaranteed to be int
        string varName = dcls->getChild("dcl")->getChild("ID")->lexeme;
        int value = stoi(dcls->getChild("NUM")->lexeme);
        
        table[varName] = fpOffset; 
        Lis(3); Word(value); Push(3); //Output mips assembly
        fpOffset -= 4;
        for (Node *child : dcls->children) { traverseDcls(child, table, fpOffset); }
    }
    else if (dcls->rule == "dcls dcls dcl BECOMES NULL SEMI") { //type is guaranteed to be int*
        string varName =  dcls->getChild("dcl")->getChild("ID")->lexeme;
        
        table[varName] = fpOffset; //Constant 1 represents NULL
        Lis(3); Word(1); Push(3); //Output mips assembly
        fpOffset -= 4;
        for (Node *child : dcls->children) { traverseDcls(child, table, fpOffset); }
    }
    else {
        for (Node *child : dcls->children) { traverseDcls(child, table, fpOffset); }
    }
}