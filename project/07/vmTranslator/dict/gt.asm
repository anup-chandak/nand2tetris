// gt
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=M-D
D=M
@GT_TRUE.J
D;JGT
D=0
@END_GT.J
0;JMP
(GT_TRUE.J)
D=-1
(END_GT.J)
@SP
A=M
M=D
@SP
M=M+1