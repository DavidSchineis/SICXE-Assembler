# SIC/XE Assembler
>A simple SIC/XE two-pass assembler implemented in C and completed during my systems programming class.

## Features

Accepts all formats of SIC/XE language as the assembly language source code.

* Format 1: Consists of 8 bits of allocated memory to store instructions.
* Format 2: Consists of 16 bits of allocated memory to store 8 bits of instructions and two 4-bits segments to store operands.
* Format 3: Consists of 6 bits to store an instruction, 6 bits of flag values, and 12 bits of displacement.
* Format 4: Consists of the same elements as format 3, but instead of a 12-bit displacement, stores a 20-bit address.

Both format 3 and format 4 have six-bit flag values in them, consisting of the following flag bits:

* n: Indirect addressing flag
* i: Immediate addressing flag
* x: Indexed addressing flag
* b: Base address-relative flag
* p: Program counter-relative flag
* e: Format 4 instruction flag

Learn more about SIC/XE [here](https://en.wikipedia.org/wiki/Simplified_Instructional_Computer).
## Passes
Pass 1:
* Processes SIC/XE source code file
* Computes and aligns addresses
* Creates and fills symbolTable

Pass 2:
* Processes directives/opcodes
* Translates instructions
* Writes to object and listing files


## How to Compile and Run
GCC Compiler
```
gcc main.c opcodes.c symbols.c directives.c errors.c
./.a.out input.sic
```
* input.sic is the SIC/XE file the user wishes to process (try your own!)
* test0.sic is an example file with no errors
* test1.sic, test2.sic, and test3.sic contain one (1) error respectively

## Sample Input
```
COPY    START   1000 
FIRST   STL     RETADR 
        LDB     #LENGTH 
        BASE    LENGTH 
CLOOP   +JSUB   RDREC 
        LDA     LENGTH 
        COMP    #0 
        JEQ     ENDFIL 
        +JSUB   WRREC 
        J       CLOOP 
ENDFIL  LDA     EOF 
        STA     BUFFER 
        LDA     #3 
        STA     LENGTH 
        +JSUB   WRREC 
        J       @RETADR 
EOF     BYTE    C'EOF' 
RETADR  RESW    1 
LENGTH  RESW    1 
BUFFER  RESB    4096 
RDREC   CLEAR   X 
        CLEAR   A 
        CLEAR   S 
        +LDT    #4096 
RLOOP   TD      INPUT 
        JEQ     RLOOP 
        RD      INPUT 
        COMPR   A,S 
        JEQ     EXIT 
        STCH    BUFFER,X 
        TIXR    T 
        JLT     RLOOP 
EXIT    STX     LENGTH 
        RSUB 
INPUT   BYTE    X'F1' 
WRREC   CLEAR   X 
        LDT     LENGTH 
WLOOP   TD      OUTPUT 
        JEQ     WLOOP 
        LDCH    BUFFER,X 
        WD      OUTPUT 
        TIXR    T 
        JLT     WLOOP 
        RSUB 
OUTPUT  BYTE    X'05' 
        END     FIRST
```
## Sample Output
Listing file
```
1000    COPY    START   1000    
1000    FIRST   STL     RETADR      17202D
1003            LDB     #LENGTH     69202D
1006            BASE    LENGTH  
1006    CLOOP   +JSUB   RDREC       4B102036
100A            LDA     LENGTH      032026
100D            COMP    #0          290000
1010            JEQ     ENDFIL      332007
1013            +JSUB   WRREC       4B10205D
1017            J       CLOOP       3F2FEC
101A    ENDFIL  LDA     EOF         032010
101D            STA     BUFFER      0F2016
1020            LDA     #3          010003
1023            STA     LENGTH      0F200D
1026            +JSUB   WRREC       4B10205D
102A            J       @RETADR     3E2003
102D    EOF     BYTE    C'EOF'      454F46
1030    RETADR  RESW    1       
1033    LENGTH  RESW    1       
1036    BUFFER  RESB    4096    
2036    RDREC   CLEAR   X           B410
2038            CLEAR   A           B400
203A            CLEAR   S           B440
203C            +LDT    #4096       75101000
2040    RLOOP   TD      INPUT       E32019
2043            JEQ     RLOOP       332FFA
2046            RD      INPUT       DB2013
2049            COMPR   A,S         A004
204B            JEQ     EXIT        332008
204E            STCH    BUFFER,X    57C003
2051            TIXR    T           B850
2053            JLT     RLOOP       3B2FEA
2056    EXIT    STX     LENGTH      134000
2059            RSUB                4F0000
205C    INPUT   BYTE    X'F1'       F1
205D    WRREC   CLEAR   X           B410
205F            LDT     LENGTH      774000
2062    WLOOP   TD      OUTPUT      E32011
2065            JEQ     WLOOP       332FFA
2068            LDCH    BUFFER,X    53C003
206B            WD      OUTPUT      DF2008
206E            TIXR    T           B850
2070            JLT     WLOOP       3B2FEF
2073            RSUB                4F0000
2076    OUTPUT  BYTE    X'05'       05
2077            END     FIRST   
```
Object file
```
HCOPY  001000001077
T0010001D17202D69202D4B1020360320262900003320074B10205D3F2FEC032010
T00101D130F20160100030F200D4B10205D3E2003454F46
T0020361DB410B400B44075101000E32019332FFADB2013A00433200857C003B850
T0020531D3B2FEA1340004F0000F1B410774000E32011332FFA53C003DF2008B850
T002070073B2FEF4F000005
E001000
```
