#include <string>
std::string DFAstring = R"(
.STATES
start
ID!
LABELDEF!
dot
DOTID!
ZERO!
minus
DECINT!
zerox
HEXINT!
dollar
REGISTER!
COMMA!
LPAREN!
RPAREN!
cr
NEWLINE!
?WHITESPACE!
?COMMENT!
.TRANSITIONS
start a-z A-Z     ID
ID    a-z A-Z 0-9 ID
ID    :           LABELDEF
start . dot
dot   a-z A-Z     DOTID
DOTID a-z A-Z 0-9 DOTID
start  0   ZERO
start  1-9 DECINT
start  -   minus
minus  0-9 DECINT
DECINT 0-9 DECINT
ZERO   0-9 DECINT
ZERO   x   zerox
zerox  0-9 a-f A-F HEXINT
HEXINT 0-9 a-f A-F HEXINT
start    $   dollar 
dollar   0-9 REGISTER
REGISTER 0-9 REGISTER
start , COMMA 
start ( LPAREN
start ) RPAREN
start \n NEWLINE
start \r cr
cr    \n NEWLINE
start       \s \t ?WHITESPACE
?WHITESPACE \s \t ?WHITESPACE
start    ; ?COMMENT
?COMMENT \x00-\x09 \x0B \x0C \x0E-\x7F ?COMMENT
)";