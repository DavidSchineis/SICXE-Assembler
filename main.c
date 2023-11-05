#include "headers.h"

// Pass 1 constants
#define COMMENT 35
#define INPUT_BUF_SIZE 60
#define NEW_LINE 10
#define SPACE 32
#define SYMBOL_TABLE_SIZE 100

// Pass 2 constants
#define BLANK_INSTRUCTION 0x000000
#define FLAG_B 0x04
#define FLAG_E 0x01
#define FLAG_I 0x10
#define FLAG_N 0x20
#define FLAG_P 0x02
#define FLAG_X 0x08
#define FORMAT_1 1
#define FORMAT_2 2
#define FORMAT_3 3
#define FORMAT_3_MULTIPLIER 0x1000
#define FORMAT_4 4
#define FORMAT_4_MULTIPLIER 0x100000
#define IMMEDIATE_CHARACTER '#'
#define INDEX_STRING ",X"
#define INDIRECT_CHARACTER '@'
#define MAX_RECORD_BYTE_COUNT 30
#define OPCODE_MULTIPLIER 0x100
#define OUTPUT_BUF_SIZE 70
#define REGISTER_A 0X0
#define REGISTER_B 0X3
#define REGISTER_L 0X2
#define REGISTER_MULTIPLIER 0x10
#define REGISTER_S 0X4
#define REGISTER_T 0X5
#define REGISTER_X 0X1
#define RSUB_INSTRUCTION 0x4C0000
#define BASE_MAX_RANGE 4096
#define PC_MAX_RANGE 2048
#define PC_MIN_RANGE -2048

// Pass 1 functions
void performPass1(symbol* symbolTable[], char* filename, address* addresses);
segment* prepareSegments(char* line);
void trim(char string[]);

// Pass 2 functions
int computeFlagsAndAddress(struct symbol* symbolArray[], address* addresses, segment* segments, int format);
char* createFilename(char* filename, const char* extension);
void flushTextRecord(FILE* file, objectFileData* data, address* addresses);
int getRegisters(char* operand);
int getRegisterValue(char registerName);
bool isNumeric(char* string);
void performPass2(struct symbol* symbolTable[], char* filename, address* addresses);
void writeToLstFile(FILE* file, int address, segment* segments, int opcode);
void writeToObjFile(FILE* file, objectFileData data);

int main(int argc, char* argv[])
{
	address addresses = { 0x00, 0x00, 0x00 };

	// Check whether at least one (1) input file was provided
	if(argc < 2)
	{
		displayError(MISSING_COMMAND_LINE_ARGUMENTS, argv[0]);
		exit(-1);
	}
	symbol* symbols[SYMBOL_TABLE_SIZE] = { NULL };
	
	// Pass 1 - processes SIC/XE code, loads symbols into symbol table, and computes addressing
	performPass1(symbols, argv[1], &addresses);

	// Display symbol table data
	displaySymbolTable(symbols);

	// Display the assembly summary data
	printf("\nStarting Address: 0x%X\nEnding Address: 0x%X\nProgram Size (bytes): %d\n", addresses.start, addresses.current, addresses.current - addresses.start);
	
	// Pass 2 - creates object code file and listing file
	performPass2(symbols, argv[1], &addresses);

}

// Determines the Format 3/4 flags and computes address displacement for Format 3 instruction
int computeFlagsAndAddress(symbol* symbolArray[], address* addresses, segment* segments, int format)
{
	char buffer[OUTPUT_BUF_SIZE];
    strcpy(buffer, segments->operand);

    int bitFlags = 0;

    if (strchr(buffer, IMMEDIATE_CHARACTER) != NULL) {	// Flag I Check
        bitFlags += FLAG_I;
        memmove(buffer, buffer + 1, strlen(buffer) + 1);
    }
	else if	(strchr(buffer, INDIRECT_CHARACTER) != NULL) {	// Flag N Check
        bitFlags += FLAG_N;
        memmove(buffer, buffer + 1, strlen(buffer) + 1);
    } else {
        bitFlags += FLAG_I + FLAG_N;
    }

    if (strstr(buffer, INDEX_STRING) != NULL) { // Flag X Check
        bitFlags += FLAG_X;
        char* comma = strstr(buffer, INDEX_STRING);
		memmove(comma, comma + 2, strlen(comma + 2) + 1);
    }

    if (format == FORMAT_4) { // Flag E Check
        bitFlags += FLAG_E;
	}

    if (strcmp(segments->operation, "RSUB") == 0){ //RSUB Check
        bitFlags *= FORMAT_3_MULTIPLIER;
		return bitFlags;
	}

    

    if (isNumeric(buffer)) { //Numeric
		long int displacement = strtol(buffer, NULL, 10);
        if (format == FORMAT_3)
            bitFlags *= FORMAT_3_MULTIPLIER;
        else if (format == FORMAT_4)
            bitFlags *= FORMAT_4_MULTIPLIER;

        bitFlags += displacement;
        return bitFlags;
    }
	else if (format == FORMAT_4){ // Non numeric format 4
		int symbolAddress = getSymbolAddress(symbolArray, buffer);
		bitFlags *= FORMAT_4_MULTIPLIER;
		bitFlags += symbolAddress;
		return bitFlags;

	}
	else{ // Non numeric format 3 
		int symbolAddress = getSymbolAddress(symbolArray, buffer);
		int pcRelative = (symbolAddress - (addresses->current + addresses->increment));
		int baseRelative = symbolAddress - addresses->base;
		if (pcRelative >= PC_MIN_RANGE && pcRelative <= PC_MAX_RANGE){
			bitFlags |= FLAG_P;

		}
		else {
			if (baseRelative >= 0 && baseRelative <= BASE_MAX_RANGE){
				bitFlags |= FLAG_B;
			}
			else {
				displayError(ADDRESS_OUT_OF_RANGE, segments->operation);
				exit(1);
			}
			pcRelative = baseRelative;
		}
		
		if (pcRelative < 0){
			pcRelative += 4096;
		}

		bitFlags *= FORMAT_3_MULTIPLIER;
		bitFlags += pcRelative;
		return bitFlags;
	}
}

// Returns a new filename using the provided filename and extension
char* createFilename(char* filename, const char* extension)
{
	char* temp = (char*)malloc(sizeof(char) * strlen(filename) + 1);
	
	int n = strchr(filename, '.') - filename;
	strncpy(temp, filename, n);
	strcat(temp, extension);
	return temp;
}

// Writes existing data to Object Data file and resets values
void flushTextRecord(FILE* file, objectFileData* data, address* addresses)
{
	writeToObjFile(file, *data);
	data->recordAddress = addresses->current;
	data->recordByteCount = 0;
	data->recordEntryCount = 0;
}

// Returns a hex byte containing the registers listed in the provided operand
int getRegisters(char* operand)
{
    register int registerVal= getRegisterValue(operand[0]);
    registerVal *= REGISTER_MULTIPLIER;

    if (strstr(operand, ",") != NULL) {
        int registerValTemp = getRegisterValue(operand[strlen(operand) - 1]);
        registerVal += registerValTemp;
    }

    return registerVal;
}

// Returns the hex value for the provided register name
int getRegisterValue(char registerName)
{
	switch(registerName)
	{
		case 'A':
			return REGISTER_A;
		case 'B':
			return REGISTER_B;
		case 'L':
			return REGISTER_L;
		case 'S':
			return REGISTER_S;
		case 'T':
			return REGISTER_T;
		case 'X':
			return REGISTER_X;
		default:
			return -1;
	}
}

// Returns true if the provided string contains a numeric value; otherwise, false
bool isNumeric(char* string)
{
	for(int x = 0; x < strlen(string); x++)
	{
		if(!isdigit(string[x]))
		{
			return false;
		}
	}
	return true;
}

// Performs Pass 1 of the SIC/XE assembler
void performPass1(symbol* symbolTable[], char* filename, address* addresses)
{
	char line[INPUT_BUF_SIZE];
	FILE* file;
	int directiveType = 0;
	
	file = fopen(filename, "r");
	if (!file)
	{
		displayError(FILE_NOT_FOUND, filename);
		exit(-1);
	}

	while (fgets(line, INPUT_BUF_SIZE, file))
	{
		// Test PC address value
		if (addresses->current >= 0x100000)
		{
			char value[10];
			sprintf(value, "0x%X", addresses->current);
			displayError(OUT_OF_MEMORY, value);
			exit(-1);
		}
		// Test first character of statement
		if (line[0] < SPACE)
		{
			displayError(BLANK_RECORD, NULL);
			exit(-1);
		}
		else if (line[0] == COMMENT)
		{
			continue;
		}
		else
		{
			// Parse statement
			segment* segments = prepareSegments(line);

			// Test label segment for directive/opcode		
			if (isDirective(segments->label) || isOpcode(segments->label))
			{
				displayError(ILLEGAL_SYMBOL, segments->label);
				exit(-1);
			}
			// Test operation segment for directive/opcode
			if ((directiveType = (isDirective(segments->operation))))
			{
				if (isStartDirective(directiveType))
				{
					addresses->start = addresses->current = strtol(segments->operand, NULL, 16);
					continue;
				}
				else
				{
					addresses->increment = getMemoryAmount(directiveType, segments->operand);
				}
			}
			else if (isOpcode(segments->operation))
			{
				addresses->increment = getOpcodeFormat(segments->operation);
			}
			else
			{
				displayError(ILLEGAL_OPCODE_DIRECTIVE, segments->operation);
				exit(-1);
			}
			// Add label to symbolTable
			if (strlen(segments->label) > 0)
			{
				insertSymbol(symbolTable, segments->label, addresses->current);
			}
			
			// Adjust address
			addresses->current += addresses->increment;
		}
		memset(line, '\0', INPUT_BUF_SIZE);
	}
	fclose(file);
}

// Performs Pass 2 of the SIC/XE assembler
void performPass2(struct symbol* symbolTable[], char* filename, address* addresses)
{
	objectFileData objectData = { 0, { 0x0 }, { "\0" }, 0, 0x0, 0, { 0 }, 0, '\0', 0x0 };
	
	char inData[INPUT_BUF_SIZE];
	char objData[OUTPUT_BUF_SIZE];

	FILE *fileIn, *fileLst, *fileObj;
	int directiveType = 0;
		
	char* lstFilename = createFilename(filename, ".lst");
	char* objFilename = createFilename(filename, ".obj");

	fileIn = fopen(filename, "r");
	if (!fileIn)
	{
		displayError(FILE_NOT_FOUND, filename);
		exit(-1);
	}
	fileLst = fopen(lstFilename, "w");
	fileObj = fopen(objFilename, "w");
	
	while (fgets(inData, INPUT_BUF_SIZE, fileIn))
	{ 
        objectData.recordType = 'T';

        // Call the prepareSegments() function to convert the statement into three segments
        segment* segments = prepareSegments(inData);

		// Test if the operation is a directive
		directiveType = isDirective(segments->operation);

        if (isDirective(segments->operation)) {

            // Check if it's the START directive
            if (isStartDirective(directiveType)) {
                objectData.recordType = 'H';

                // Set programName, startAddress, recordAddress, programSize
                strcpy(objectData.programName, segments->label);
                objectData.startAddress = addresses->start;
                objectData.recordAddress = addresses->start;
                objectData.programSize = addresses->current - addresses->start;
				addresses->current = addresses->start;

                // Write to object and listing files
                writeToObjFile(fileObj, objectData);
                writeToLstFile(fileLst, addresses->current, segments, BLANK_INSTRUCTION);
                continue;
            }
            
            // Check if it's the BASE directive
            if (isBaseDirective(directiveType)) {

                // Get the BASE address and set it in addresses->base
                addresses->base = getSymbolAddress(symbolTable, segments->operand);

                // Write to listing file
                writeToLstFile(fileLst, addresses->current, segments, BLANK_INSTRUCTION);
                continue;
            }
            
            // Check if it's the END directive
            if (isEndDirective(isDirective(segments->operation))) {

                // Check if there is an open text record and flush it
                if (objectData.recordByteCount > 0){
                    flushTextRecord(fileObj, &objectData, addresses);
				}
				objectData.recordType = 'E';
				
                // Write to object and listing files
                writeToObjFile(fileObj, objectData);
                writeToLstFile(fileLst, addresses->current, segments, BLANK_INSTRUCTION);
                continue;
            }
            
            // Check if it's a RESB or RESW directive
            if (isReserveDirective(directiveType)) {

                // Check if there is an open text record and flush it
                if (objectData.recordByteCount > 0){
                    flushTextRecord(fileObj, &objectData, addresses); 
				}

                // Write to listing file
                writeToLstFile(fileLst, addresses->current, segments, BLANK_INSTRUCTION);

                // Update memory
                addresses->increment = getMemoryAmount(directiveType, segments->operand);
                objectData.recordAddress += addresses->increment;
				addresses->current += addresses->increment;
                continue;
            }
            
            // Check if it's a BYTE directive
            if (isDataDirective(directiveType)) {

                // Check if there is an open text record and flush it if necessary
				addresses->increment = getMemoryAmount(directiveType, segments->operand);
                if (objectData.recordByteCount > (MAX_RECORD_BYTE_COUNT - addresses->increment)) {
                    flushTextRecord(fileObj, &objectData, addresses);
				}

                // Get the byte value and add new record in the object data
                int byteValue = getByteValue(directiveType, segments->operand);
                objectData.recordEntries[objectData.recordEntryCount].numBytes = addresses->increment;
                objectData.recordEntries[objectData.recordEntryCount].value = byteValue;
                objectData.recordEntryCount++;
                objectData.recordByteCount += addresses->increment;

				// Write to listing file
                writeToLstFile(fileLst, addresses->current, segments, byteValue);

				// Update memory
				addresses->current += addresses->increment;
                continue;
            }
        }
        
        // Check if the operation is an opcode
        if (isOpcode(segments->operation)) {

            // Get the opcode value and format
            opcode op;
			op.value = getOpcodeValue(segments->operation);
            addresses->increment = getOpcodeFormat(segments->operation);
           
            // Align the opcode value with the instruction format
            switch (addresses->increment) {
				case -1:
					displayError(ILLEGAL_OPCODE_FORMAT, segments->operation);
                	exit(1);
					break;
				case 2:
					op.value *= OPCODE_MULTIPLIER;
					op.value += getRegisters(segments->operand);
					break;
				case 3:
					for(int i = 0; i < 2; i++){
						op.value *= OPCODE_MULTIPLIER;
					}
					op.value += computeFlagsAndAddress(symbolTable, addresses, segments, addresses->increment);
					break;
				case 4:
					for(int i = 0; i < 3; i++){
						op.value *= OPCODE_MULTIPLIER;
					}
					op.value += computeFlagsAndAddress(symbolTable, addresses, segments, addresses->increment);
					break;
				default:
					break;
			}

            // Check if there is an open text record and flush it if necessary
            if (objectData.recordByteCount > (MAX_RECORD_BYTE_COUNT - addresses->increment)){
                flushTextRecord(fileObj, &objectData, addresses);
			}

			// Add a record entry to the object data
			objectData.recordEntries[objectData.recordEntryCount].numBytes = addresses->increment;
			objectData.recordEntries[objectData.recordEntryCount].value = op.value;
			objectData.recordEntryCount++;
			objectData.recordByteCount += addresses->increment;
			writeToLstFile(fileLst, addresses->current, segments, op.value);

			// Update memory
			addresses->current += addresses->increment;

			// Clear the statement buffer
			memset(inData, '\0', INPUT_BUF_SIZE);
		}

		if (objectData.recordByteCount > (MAX_RECORD_BYTE_COUNT - addresses->increment)){
			flushTextRecord(fileObj, &objectData, addresses);
		}
	}
	fclose(fileIn);
	fclose(fileLst);
	fclose(fileObj);
}

// Separates a SIC/XE instruction into individual sections
segment* prepareSegments(char* statement)
{
	segment* temp = malloc(sizeof(segment));
	strncpy(temp->label, statement, SEGMENT_SIZE - 1);
	strncpy(temp->operation, statement + SEGMENT_SIZE - 1, SEGMENT_SIZE - 1);
	strncpy(temp->operand, statement + (SEGMENT_SIZE - 1) * 2, SEGMENT_SIZE - 1);

	trim(temp->label); // Label
	trim(temp->operation); // Operation
	trim(temp->operand); // Operand
	return temp;
}

// Removes spaces from the end of a segment value
void trim(char value[])
{
	for (int x = 0; x < SEGMENT_SIZE; x++)
	{
		if (value[x] == SPACE)
		{
			value[x] = '\0';
		}
	}
}

// Write SIC/XE instructions along with address and object code information of source code listing file
void writeToLstFile(FILE* file, int address, segment* segments, int opcode)
{
	char ctrlString[27];
	int length;
	
	int directiveType = isDirective(segments->operation);
	if (isStartDirective(directiveType) || 
		isBaseDirective(directiveType) || 
		isReserveDirective(directiveType))
	{
		fprintf(file, "%-8X%-8s%-8s%-8s\n", address, segments->label, segments->operation, segments->operand);
	}
	else if (isEndDirective(directiveType))
	{
		fprintf(file, "%-8X%-8s%-8s%-8s", address, segments->label, segments->operation, segments->operand);
	}
	else
	{
		if (isDataDirective(directiveType))
		{
			length = getMemoryAmount(directiveType, segments->operand) * 2;
		}
		else
		{
			length = getOpcodeFormat(segments->operation) * 2;
		}
		sprintf(ctrlString, "%%-8X%%-8s%%-8s%%-8s    %%0%dX\n", length);

		fprintf(file, ctrlString, address, segments->label, segments->operation, segments->operand, opcode);
	}
}

// Write object code data to object code file
void writeToObjFile(FILE* file, objectFileData data)
{
	char ctrlString[27];
	
	if (data.recordType == 'H')
	{
		fprintf(file, "H%-6s%06X%06X\n", data.programName, data.startAddress, data.programSize);
	}
	else if (data.recordType == 'T')
	{
		fprintf(file, "T%06X%02X", data.recordAddress, data.recordByteCount);
		for (int x = 0; x < data.recordEntryCount; x++)
		{
			sprintf(ctrlString, "%%0%dX", data.recordEntries[x].numBytes * 2);
			fprintf(file, ctrlString, data.recordEntries[x].value);
		}
		fprintf(file, "\n");
	}
	else if (data.recordType == 'E')
	{
		fprintf(file, "E%06X", data.startAddress);
	}
	else if (data.recordType == 'M')
	{
		for (int x = 0; x < data.modificationCount; x++)
		{
			fprintf(file, "M%06X05+%s\n", data.modificationEntries[x], data.programName);
		}
	}
}