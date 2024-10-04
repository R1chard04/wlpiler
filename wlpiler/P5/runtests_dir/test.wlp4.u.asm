.import print
.import init
.import new
.import delete
lis $4
.word 4
lis $11
.word 1
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sub $29, $30, $4
add $2, $0, $0
sw $31, -4($30)
sub $30, $30, $4
lis $3
.word init
jalr $3
add $30, $30, $4
lw $31, -4($30)
label0:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label2
sub $3, $3, $11
label2:
add $3, $3, $11
beq $3, $0, endlabel0
label3:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label5
sub $3, $3, $11
label5:
add $3, $3, $11
beq $3, $0, endlabel3
label6:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label8
sub $3, $3, $11
label8:
add $3, $3, $11
beq $3, $0, endlabel6
lis $3
.word 1
sw $3, 8($29)
beq $0, $0, label7
endlabel6:
lis $3
.word 2
sw $3, 8($29)
label7:
beq $0, $0, label4
endlabel3:
label9:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label11
sub $3, $3, $11
label11:
add $3, $3, $11
beq $3, $0, endlabel9
lis $3
.word 3
sw $3, 8($29)
beq $0, $0, label10
endlabel9:
lis $3
.word 4
sw $3, 8($29)
label10:
label4:
beq $0, $0, label1
endlabel0:
label12:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label14
sub $3, $3, $11
label14:
add $3, $3, $11
beq $3, $0, endlabel12
label15:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label17
sub $3, $3, $11
label17:
add $3, $3, $11
beq $3, $0, endlabel15
lis $3
.word 5
sw $3, 8($29)
beq $0, $0, label16
endlabel15:
lis $3
.word 6
sw $3, 8($29)
label16:
beq $0, $0, label13
endlabel12:
label18:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
sub $5, $3, $5
add $3, $0, $0
beq $5, $0, label20
sub $3, $3, $11
label20:
add $3, $3, $11
beq $3, $0, endlabel18
lis $3
.word 7
sw $3, 8($29)
beq $0, $0, label19
endlabel18:
lis $3
.word 8
sw $3, 8($29)
label19:
label13:
label1:
lw $3, 8($29)
add $30, $30, $4
add $30, $30, $4
jr $31
