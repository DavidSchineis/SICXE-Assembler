/*********************************************
*        DO NOT REMOVE THIS MESSAGE
*
* This file is provided by Professor Littleton
* to assist students with completing Project 3.
*
*  DO NOT MODIFY THIS FILE WITHOUT PERMISSION
*
*        DO NOT REMOVE THIS MESSAGE
**********************************************/
#pragma once

typedef struct opcode
{
	char name[NAME_SIZE];
	int format; // Instruction format: 1, 2 or 3/4 bytes
	int value;
} opcode;

int getOpcodeFormat(char* opcode);
int getOpcodeValue(char* opcode);
bool isOpcode(char* string);