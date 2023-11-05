#include "headers.h"

#define SINGLE_QUOTE 39

// List of valid directives, ERROR helps isDirective()
enum directives {
	ERROR, BASE, BYTE, END, RESB, RESW, START
};

// Converts a character to its hexadecimal value
int charToHex(char c) {
    return (int)c;
}

// Returns the value associated with a BYTE directive
int getByteValue(int directiveType, char* string)
{
	char charTemp[SINGLE_QUOTE];
	int bytesRequired = -1;

	if (strstr(string, "X'") != NULL) {	// X Value
		if (strlen(string) > 6){
			displayError(OUT_OF_RANGE_BYTE, string);
			exit(1);
		}
		else{
			strncpy(charTemp, &string[2], 2);
            charTemp[2] = '\0';  
			bytesRequired = strtol(charTemp, NULL, 16);
		}
	} else if (strstr(string, "C'") != NULL) {	// C String
        int len = strlen(string) - 3;

        // Each character will be represented by 2 hexadecimal digits
        char hexString[2 * len + 1]; 

        // Convert each character to its equivalent hexadecimal value
        for (int i = 2, j = 0; i < len + 2; i++, j += 2) {
            int hexVal = charToHex(string[i]);
            sprintf(hexString + j, "%02X", hexVal);
        }
        bytesRequired = (int)strtol(hexString, NULL, 16);
    }

	return bytesRequired;
}

// Returns the number of bytes required to store the BYTE directive value in memory
int getMemoryAmount(int directiveType, char* string)
{
	char hex[9] = { '\0' };
	int temp = 0;

	switch (directiveType)
	{
		case BASE:
		case END:
		case START:
			return 0;
			break;
		case BYTE:
			if (string[0] == 'X')
			{
				if (strlen(string) != 5)
				{
					displayError(OUT_OF_RANGE_BYTE, string);
					exit(-1);
				}
				else
					return 1;
			}
			else if (string[0] == 'C')
				return strlen(string) - 3;
			break;
		case RESB:
			return strtol(string, NULL, 10);
			break;
		case RESW:
			return strtol(string, NULL, 10) * 3;
			break;
	}
	return -1;
}

// Returns true if the provided directive type is the BASE directive; otherwise, false
bool isBaseDirective(int directiveType)
{
	return directiveType == BASE;
}

// Returns true if the provided directive type is the BYTE directive; otherwise, false
bool isDataDirective(int directiveType)
{
	return directiveType == BYTE;
}

// Tests whether the provided string is a valid directive
// Returns true if string is valid directive; otherwise, false
int isDirective(char* string) 
{
	if (strcmp(string, "BASE") == 0) { return BASE; }
	else if (strcmp(string, "BYTE") == 0) { return BYTE; }
	else if(strcmp(string, "END") == 0) { return END; }
	else if (strcmp(string, "RESB") == 0) { return RESB; }
	else if (strcmp(string, "RESW") == 0) { return RESW; }
	else if (strcmp(string, "START") == 0) { return START; }
	else { return ERROR; }
}

// Returns true if the provided directive type is the END directive; otherwise, false
bool isEndDirective(int directiveType)
{
	return directiveType == END;
}

// Returns true if the provided directive type is the RESB or RESW directive; otherwise, false
bool isReserveDirective(int directiveType)
{
	if (directiveType == RESB || directiveType == RESW){
		return true;
	}

	return false;
}

// Returns true if the provided directive type is the START directive; otherwise, false
bool isStartDirective(int directiveType)
{
	return directiveType == START;
}