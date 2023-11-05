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

## Passes
Pass 1:
* Processes SIC/XE source code file
* Computes and aligns addresses
* Creates and fills symbolTable

Pass 2:
* Processes directives/opcodes
* Translates instructions
* Writes to object and listing files
