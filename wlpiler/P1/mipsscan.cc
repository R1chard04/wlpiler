#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cctype>
#include <fstream>
#include <limits.h>
#include "mips.h"
#include "dfa.h"

using namespace std;

const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";

bool inIntRange(std::string strNum);

bool isChar(std::string s);
bool isRange(std::string s);
std::string squish(std::string s);
int hexToNum(char c);
char numToHex(int d);
std::string escape(std::string s);
std::string unescape(std::string s);

int main() {
    try {
        std::string input;
        std::stringstream s(DFAstring);
        MIPS dfa{s};

        while (std::getline(cin, input)){
            dfa.simplifiedMaxMunch(input + "\n");
        }
        
    } catch(std::runtime_error &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

MIPS::MIPS(std::istream &in){
    std::string s;
    // Skip blank lines at the start of the file
    while(true) {
        if (!(std::getline(in, s))) {
        throw std::runtime_error
            ("Expected " + STATES + ", but found end of input.");
        }
        s = squish(s);
        if (s == STATES) {
            break;
        }
        if (!s.empty()) {
            throw std::runtime_error
                ("Expected " + STATES + ", but found: " + s);
        }
    }
    // Print states
    bool initial = true;
    while(true) {
        if (!(in >> s)) {
            throw std::runtime_error
                ("Unexpected end of input while reading state set: " 
                + TRANSITIONS + "not found.");
        }
        if (s == TRANSITIONS) {
            break;
        } 
        // Process an individual state
        bool accepting = false;
        if (s.back() == '!' && s.length() > 1) {
            accepting = true;
            s.pop_back();
        }

        this->accepting[s] = accepting; // MAP STATES TO ACCEPTING BOOL VALUE

        if (initial) {
            this->initial = s;
        }
        initial = false;
    }
    // Print transitions
    std::getline(in, s); // Skip .TRANSITIONS header
    while(true) {
        if (!(std::getline(in, s))) {
            // We reached the end of the file
            break;
        }
        s = squish(s);
        if (s == INPUT) {
            break;
        } 
        // Split the line into parts
        std::string lineStr = s;
        std::stringstream line(lineStr);
        std::vector<std::string> lineVec;
        while(line >> s) {
            lineVec.push_back(s);
        }
        if(lineVec.empty()) {
            // Skip blank lines
            continue;
        }
        if (lineVec.size() < 3) {
            throw std::runtime_error
                ("Incomplete transition line: " + lineStr);
        }
        // Extract state information from the line
        std::string fromState = lineVec.front();
        std::string toState = lineVec.back();
        // Extract character and range information from the line
        std::vector<char> charVec;
        for (int i = 1; i < lineVec.size()-1; ++i) {
            std::string charOrRange = escape(lineVec[i]);
            if (isChar(charOrRange)) {
                char c = charOrRange[0];
                if (c < 0 || c > 127) {
                    throw std::runtime_error
                        ("Invalid (non-ASCII) character in transition line: " + lineStr + "\n"
                        + "Character " + unescape(std::string(1,c)) + " is outside ASCII range");
                }
                charVec.push_back(c);
            } else if (isRange(charOrRange)) {
                for(char c = charOrRange[0]; charOrRange[0] <= c && c <= charOrRange[2]; ++c) {
                    charVec.push_back(c);
                }
            } else {
                throw std::runtime_error
                ("Expected character or range, but found "
                + charOrRange + " in transition line: " + lineStr);
            }
        }
        // Print a representation of the transition line
        for ( char c : charVec ) {
            transitions[fromState][c] = toState; // unescape(std::string(1,c))[0]
        }
    }
    // We ignore .INPUT sections, so we're done
    // cout << "Accepting States -----" << endl;
    // for (const auto& pair : this->accepting) {
    //   std::cout << "Key: " << pair.first << ", Value: " << pair.second << endl;
    // }
    // cout << "Transitions ------" << endl;
    // for (const auto& outerPair : transitions) {
    //     const std::string& currentState = outerPair.first;
    //     const auto& innerMap = outerPair.second;
    //     // Iterate through the inner unordered_map
    //     for (const auto& innerPair : innerMap) {
    //         char inputSymbol = innerPair.first;
    //         const std::string& nextState = innerPair.second;
    //         std::cout << "From " << currentState << " on input '" << inputSymbol << "' to " << nextState << std::endl;
    //     }
    // }
}

std::string MIPS::getNextState(std::string currState, char c) {
    auto it = transitions.find(currState);
    if (it != transitions.end()) {
        // Key1 exists in the outer map
        std::unordered_map<char, std::string>& innerMap = it->second;
        auto innerIt = innerMap.find(c);
        if (innerIt != innerMap.end()) {
            std::string nextState = this->transitions[currState][c];
            return nextState;
        }
    }
    return "NF";
}

void MIPS::simplifiedMaxMunch(std::string s) {
    std::string currState = this->initial;
    std::string tmpToken = "";

    int i = 0;
    while (i < s.length()) {
        std::string nextState = this->getNextState(currState, s[i]);
        
        if (nextState != "NF") {
            tmpToken += s[i];
            i++;
            currState = nextState;
        }
        else { // we have reached a state where there is no transition to another state
            if (this->accepting[currState]) {
                if (currState == "NEWLINE") {
                    std::cout << "NEWLINE" << std::endl;
                }
                else if (currState == "REGISTER") {
                    try {
                      if (stoi(tmpToken.substr(1)) > 31) { std::cerr << "ERROR"; }
                    } catch (...) { std::cerr << "ERROR"; return; }

                    std::cout << currState << " " << tmpToken << std::endl;
                }
                else if (currState == "DECINT") {
                    if (!inIntRange(tmpToken)) { std::cerr << "ERROR"; return; }
                    std::cout << currState << " " << tmpToken << std::endl;
                }
                else if (currState == "HEXINT") {
                    try {
                        if ((unsigned long int)stoul(tmpToken, nullptr, 16) > UINT_MAX) { std::cerr << "ERROR"; return; }
                    } catch (...) { std::cerr << "ERROR"; return; }

                    std::cout << currState << " " << tmpToken << std::endl;
                }
                else if (currState == "ZERO" && tmpToken == "0"){ // a single 0 is DECINT state, not ZERO
                    std::cout << "DECINT" << " " << tmpToken << std::endl;
                }
                else if (currState != "?WHITESPACE" && currState != "?COMMENT"){
                    std::cout << currState << " " << tmpToken << std::endl;
                }
                currState = this->initial;
                tmpToken = "";
            }
            else { std::cerr << "ERROR"; return; }
        }
    }
    if (this->accepting[currState]) {
        if (currState == "NEWLINE") {
            std::cout << "NEWLINE" << std::endl;
        }
        else if (currState == "REGISTER") {
            try {
              if (stoi(tmpToken.substr(1)) > 31) { std::cerr << "ERROR"; return; }
            } catch (...) { std::cerr << "ERROR"; return; }

            std::cout << currState << " " << tmpToken << std::endl;
        }
        else if (currState == "DECINT") {
            if (!inIntRange(tmpToken)) { std::cerr << "ERROR"; return; }

            std::cout << currState << " " << tmpToken << std::endl;
        } 
        else if (currState == "HEXINT") {
            try {
                if (stoul(tmpToken, nullptr, 16) > UINT_MAX) { std::cerr << "ERROR"; return; }
            } catch (...) { std::cerr << "ERROR"; return; }

            std::cout << currState << " " << tmpToken << std::endl;
        }
        else if (currState == "ZERO" && tmpToken == "0"){ // a single 0 is DECINT state, not ZERO
            std::cout << "DECINT" << " " << tmpToken << std::endl;
        }
        else if (currState != "?WHITESPACE" && currState != "?COMMENT"){
            std::cout << currState << " " << tmpToken << std::endl;
        }
        currState = this->initial;
        tmpToken = "";
    }
    else {
        std::cerr << "ERROR";
    }
}

//// Helper functions

bool inIntRange(std::string strNum) {
    if (strNum[0] == '-') {
        try {
            if ((signed long int)stoi(strNum) < -2147483648) { return false; }
        } catch (...) { return false; }
    }
    else {
        try {
            if ((unsigned long int)stoul(strNum) > 4294967295) { return false; }
        } catch (...) { return false; }
    }
    return true;
}

bool isChar(std::string s) {
  return s.length() == 1;
}

bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}

std::string squish(std::string s) {
  std::stringstream ss(s);
  std::string token;
  std::string result;
  std::string space = "";
  while(ss >> token) {
    result += space;
    result += token;
    space = " ";
  }
  return result;
}

int hexToNum(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return 10 + (c - 'a');
  } else if ('A' <= c && c <= 'F') {
    return 10 + (c - 'A');
  }
  // This should never happen....
  throw std::runtime_error("Invalid hex digit!");
}

char numToHex(int d) {
  return (d < 10 ? d + '0' : d - 10 + 'A');
}

std::string escape(std::string s) {
  std::string p;
  for(int i=0; i<s.length(); ++i) {
    if (s[i] == '\\' && i+1 < s.length()) {
      char c = s[i+1]; 
      i = i+1;
      if (c == 's') {
        p += ' ';            
      } else
      if (c == 'n') {
        p += '\n';            
      } else
      if (c == 'r') {
        p += '\r';            
      } else
      if (c == 't') {
        p += '\t';            
      } else
      if (c == 'x') {
        if(i+2 < s.length() && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
          if (hexToNum(s[i+1]) > 8) {
            throw std::runtime_error(
                "Invalid escape sequence \\x"
                + std::string(1, s[i+1])
                + std::string(1, s[i+2])
                +": not in ASCII range (0x00 to 0x7F)");
          }
          char code = hexToNum(s[i+1])*16 + hexToNum(s[i+2]);
          p += code;
          i = i+2;
        } else {
          p += c;
        }
      } else
      if (isgraph(c)) {
        p += c;            
      } else {
        p += s[i];
      }
    } else {
       p += s[i];
    }
  }  
  return p;
}

std::string unescape(std::string s) {
  std::string p;
  for(int i=0; i<s.length(); ++i) {
    char c = s[i];
    if (c == ' ') {
      p += "\\s";
    } else
    if (c == '\n') {
      p += "\\n";
    } else
    if (c == '\r') {
      p += "\\r";
    } else
    if (c == '\t') {
      p += "\\t";
    } else
    if (!isgraph(c)) {
      std::string hex = "\\x";
      p += hex + numToHex((unsigned char)c/16) + numToHex((unsigned char)c%16);
    } else {
      p += c;
    }
  }
  return p;
}

