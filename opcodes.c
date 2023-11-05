#include "headers.h"

#define OPCODE_ARRAY_SIZE 50

bool isFormat4Instruction(char* opcode);
int searchOpcodes(char* opcode);

// A format of 3 indicates a 3- or 4-byte instruction
opcode opcodes[OPCODE_ARRAY_SIZE] = { 
	{"ADD",3,0x18}, {"ADDR",2,0x90},  {"AND",3,0x40},   {"CLEAR",2,0xB4},
	{"COMP",3,0x28},{"COMPR",2,0xA0}, {"DIV",3,0x24},   {"DIVR",2,0x9C},
	{"FIX",1,0xC4}, {"HIO",1,0xF4},   {"J",3,0x3C},     {"JEQ",3,0x30},
	{"JGT",3,0x34}, {"JLT",3,0x38},   {"JSUB",3,0x48},  {"LDA",3,0x00},
	{"LDB",3,0x68}, {"LDCH",3,0x50},  {"LDL",3,0x08},   {"LDS",3,0x6C},
	{"LDT",3,0x74}, {"LDX",3,0x04},   {"LPS",3,0xD0},   {"MUL",3,0x20},
	{"MULR",2,0x98},{"OR",3,0x44},    {"RD",3,0xD8},    {"RMO",2,0xAC},
	{"RSUB",3,0x4C},{"SHIFTL",2,0xA4},{"SHIFTR",2,0xA8},{"SIO",1,0xF0},
	{"SSK",3,0xEC}, {"STA",3,0x0C},   {"STB",3,0x78},   {"STCH",3,0x54},
	{"STI",3,0xD4}, {"STL",3,0x14},   {"STS",3,0x7C},   {"STSW",3,0xE8},
	{"STT",3,0x84}, {"STX",3,0x10},   {"SUB",3,0x1C},   {"SUBR",2,0x94},
	{"SVC",2,0xB0}, {"TD",3,0xE0},    {"TIO",1,0xF8},   {"TIX",3,0x2C},
	{"TIXR",2,0xB8},{"WD",3,0xDC}
};

// Returns the format of the provided opcode
int getOpcodeFormat(char* opcode)
{
	int format;
	int isFormat4;
	
	if((isFormat4 = isFormat4Instruction(opcode)))
	{
		opcode = &opcode[1];
	}
	format = opcodes[searchOpcodes(opcode)].format;
	if(isFormat4 && format < 3)
	{
		return -1;
	}
	else if(isFormat4)
	{
		return 4;
	}
	else 
	{
		return format;
	}
}

// Returns the value of the provided opcode; otherwise; -1
int getOpcodeValue(char* opcode)
{
	int x;
	
	if(isFormat4Instruction(opcode))
	{
		opcode = &opcode[1];
	}
	if((x = searchOpcodes(opcode)) >= 0)
	{
		return opcodes[x].value;
	}
	return -1; // Should not happen
}

// Tests whether the provided opcode is extended (contains a '+' sign)
// Returns true if format 4; otherwise, false
bool isFormat4Instruction(char* opcode)
{
	return opcode[0] == '+';
}

// Tests whether the provided string is a valid opcode
// Returns true if string is valid opcode; otherwise, false
bool isOpcode(char* string)
{
	return getOpcodeValue(string) == -1 ? false : true;
}

// Performs a binary search of the opcodes array.
// Returns index of the opcode; otherwise, -1 (opcode not found)
int searchOpcodes(char* opcode)
{
	int low = 0, high = OPCODE_ARRAY_SIZE - 1;
	int mid;
	
	while(low <= high)
	{
		mid = (high + low) / 2;
		if (strcmp(opcodes[mid].name, opcode) == 0)
		{
			return mid;
		}
		else if (strcmp(opcodes[mid].name, opcode) < 0)
		{
			low = mid + 1;
		}
		else if (strcmp(opcodes[mid].name, opcode) > 0)
		{
			high = mid - 1;
		}
	}
	return -1;
}