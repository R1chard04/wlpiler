#include <string>
std::string DFAstring = R"(
.STATES
start
int_if!
IF!
in!
INT!
wain_while!
wa!
wai!
WAIN!
wh!
whi!
whil!
WHILE!
p!
pr!
pri!
prin!
print!
printl!
PRINTLN!
r!
re!
ret!
retu!
retur!
RETURN!
n!
ne!
NEW!
N!
NU!
NUL!
NULL!
d!
de!
del!
dele!
delet!
DELETE!
e!
el!
els!
ELSE!
ID!
LPAREN!
RPAREN!
LBRACE!
RBRACE!
LBRACK!
RBRACK!
BECOMES!
PLUS!
MINUS!
STAR!
SLASH!
PCT!
AMP!
COMMA!
SEMI!
GT!
GE!
LT!
LE!
EQ!
excla
NE!
?WHITESPACE!
?COMMENT!
NUM!
ZERO!
start_w_zero
.TRANSITIONS
start i int_if
start e e
start d d
start N N
start a-c f-h j-m o q s-v x-z A-M O-Z ID
start w wain_while
start p p
start r r
start n n
start ( LPAREN
start ) RPAREN
start { LBRACE
start } RBRACE
start [ LBRACK
start ] RBRACK
start = BECOMES
start + PLUS
start - MINUS
start * STAR
start / SLASH
start % PCT
start & AMP
start , COMMA
start ; SEMI
start > GT
start < LT
start ! excla
start \s \t \r \n ?WHITESPACE
start 1-9 NUM
start 0 ZERO
NUM 0-9 NUM
ZERO 0-9 start_w_zero
start_w_zero 0-9 start_w_zero
SLASH / ?COMMENT
GT = GE
LT = LE
BECOMES = EQ
excla = NE
?WHITESPACE \s \t \r \n ?WHITESPACE
?COMMENT \x00-\x09 \x0B \x0C \x0E-\x7F ?COMMENT
int_if f IF
IF a-z A-Z 0-9 ID
int_if n in
int_if a-e g-m o-z A-Z 0-9 ID
in t INT
in a-s u-z A-Z 0-9 ID
INT a-z A-Z 0-9 ID
e l el
e a-k m-z A-Z 0-9 ID
el s els
el a-r t-z A-Z 0-9 ID
els e ELSE
els a-d f-z A-Z 0-9 ID
ELSE a-z A-Z 0-9 ID
d e de
d a-d f-z A-Z 0-9 ID
de l del
de a-k m-z A-Z 0-9 del
del e dele
del a-d f-z A-Z 0-9 ID
dele t delet
dele a-s u-z A-Z 0-9 ID
delet e DELETE
delet a-d f-z A-Z 0-9 ID
DELETE a-z A-Z 0-9 ID
N U NU
N a-z A-T V-Z 0-9 ID
NU L NUL
NU a-z A-K M-Z 0-9 ID
NUL L NULL
NUL a-z A-K M-Z 0-9 ID
NULL a-z A-Z 0-9 ID
n e ne
n a-d f-z A-Z 0-9 ID
ne w NEW
ne a-v x-z A-Z 0-9 ID
NEW a-z A-Z 0-9 ID
r e re
r a-d f-z A-Z 0-9 ID
re t ret
re a-s u-z A-Z 0-9 ID
ret u retu
ret a-t v-z A-Z 0-9 ID
retu r retur
retu a-q s-z A-Z 0-9 ID
retur n RETURN
retur a-m o-z A-Z 0-9 ID
RETURN a-z A-Z 0-9 ID
p r pr
p a-q s-z A-Z 0-9 ID
pr i pri
pr a-h j-z A-Z 0-9 ID
pri n prin
pri a-m o-z A-Z 0-9 ID
prin t print
prin a-s u-z A-Z 0-9 ID
print l printl
print a-k m-z A-Z 0-9 ID
printl n PRINTLN
printl a-m o-z A-Z 0-9 ID
PRINTLN a-z A-Z 0-9
wain_while a wa
wain_while h wh
wain_while b-g i-z A-Z 0-9 ID
wa i wai
wa a-h j-z A-Z 0-9 ID
wai n WAIN
wai a-m o-z A-Z 0-9 ID
WAIN a-z A-Z 0-9 ID
wh i whi
wh a-h j-z A-Z 0-9 ID
whi l whil
whi a-k m-z A-Z 0-9 ID
whil e WHILE
whil a-d f-z A-Z 0-9 ID
WHILE a-z A-Z 0-9 ID
ID a-z A-Z 0-9 ID
)";