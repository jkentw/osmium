/* J. Kent Wirant
 * 20 Dec. 2022
 * ECE 1895 - Project 3
 * init.c
 * Description:
 *   This code is called directly by the bootloader. It initializes the 
 *   Operating System by loading all necessary drivers and starting the
 *   Command Line.
 */
 
//normally this definition would be in <stdlib.h>, but our OS does not
//have this library.
typedef unsigned long int size_t;

#include <stdint.h>
#include "text_util.h"
#include "string_util.h"
#include "interrupts.h"
#include "isr.h"
#include "keyboard.h"
#include "driver_pci.h"

#define TERMINAL_ROWS 16 //16 rows of 16 bytes

int cursorRow = 0; //32 columns to account for 2 hex digits per byte
int cursorCol = 0;

uint8_t hexBuffer[TERMINAL_ROWS * 16 * 2 + 1]; //account for null space
uint8_t asciiBuffer[TERMINAL_ROWS * 16 + 1]; //account for null space
uint8_t commandBuffer[32 + 1]; //account for null space
uint8_t statusBuffer[32 + 1]; //account for null space
uint8_t extraBuffer[80*4 + 1]; //account for null space
uint8_t selectedBuffer = 0; //0 for hex, 1 for ascii, 2 for command

uint32_t memLocation = 0x7000;

void updateDisplay(void) {
	//write header to display
	setCursorPosition(0, 0);
	char line[81];
	line[80] = 0;
	uint32_t memData;
	uint32_t memRow = memLocation & ~0xF;
	
	//line 1
	for(int i = 0; i < 80; i++) {
		line[i] = ' ';
	}
	strncpy_safe(line, "Press ESC to enter a command.", 29);
	printRaw(line); //line 1
	
	//line 2
	strncpy_safe(line, " Address  | ", 12);
	for(int i = 0; i < 16; i++) {
		line[12 + i * 3] = '_';
		intToHexStr(&line[13 + i * 3], i, 1);
		line[14 + i * 3] = ' ';
	}
	strncpy_safe(&line[60], "| 0123456789ABCDEF  ", 20);
	printRaw(line);
	
	//line 3
	for(int i = 0; i < 80; i++) {
		line[i] = '_';
	}
	printRaw(line);
	
	//lines 4 to (3+TERMINAL_ROWS)
	for(int i = 0; i < TERMINAL_ROWS; i++) {
		line[0] = ' ';
		intToHexStr(&line[1], memRow + i * 16, 8);
		strncpy_safe(&line[8], "_ | ", 4);
		
		for(int j = 0; j < 4; j++) {
			memData = *((uint32_t *)(memRow + i * 16 + j * 4));
			
			for(int k = 0; k < 4; k++) {
				//hex portion of display
				intToHexStr(&line[12 + 3 * (j * 4 + k)], memData, 2);
				line[14 + 3 * (j * 4 + k)] = ' ';
								
				//ascii portion of display
				if((memData & 0xFF) >= 0x20 && (memData & 0xFF) < 0x7F) {
					line[62 + j * 4 + k] = memData & 0xFF;
				}
				else {
					line[62 + j * 4 + k] = '.'; //filler character
				}
				
				memData >>= 8;
			}
		}
		
		line[60] = '|';
		line[61] = ' ';
		line[78] = ' ';
		line[79] = ' ';	
		printRaw(line);
	}
	
	//lines (4+TERMINAL_ROWS) to 23
	for(int i = 0; i < 4; i++) {
		strncpy_safe(line, &extraBuffer[i*80], 80);
		printRaw(line);
	}

	//line 24
	strncpy_safe(line, " Commands: goto <addr16>; call <addr16>; pciEnum <addr16> <count10>", 67);
	for(int i = 67; i < 80; i++) {
		line[i] = ' ';
	}
	printRaw(line);

	//line 25
	line[0] = '>';
	line[1] = ' ';
	
	for(int i = 2; i < 80; i++) {
		line[i] = ' ';
	}
	
	strncpy_safe(&line[2], commandBuffer, 32);
	strncpy_safe(&line[46], statusBuffer, 32);
	printRaw(line);
	
	//convert cursor position to screen coordinates
	char row = cursorRow + 3;
	char col;
	
	if(selectedBuffer == 0) { //hex: formatted as "## ## ## ..."
		col = 12 + cursorCol / 2 * 3 + (cursorCol & 1);
	}
	else if(selectedBuffer == 1) { //ascii: formatted as "####...####"
		col = 62 + cursorCol / 2; //ascii offset
	}
	else if(selectedBuffer == 2) { //command buffer
		col = 2 + cursorCol;
		row = 24;
	}
	
	highlight(row, col);
}

void updateMemory(void) {
	//The terminal program runs in the address range of
	//0x7800 to 0xB800. It has been made read-only to avoid 
	//unintentional writes from viewing the range.
	//if(memLocation >= 0x7800 && memLocation < 0xB800)
	//	return;
	
	if(selectedBuffer == 0) { //hex buffer
		for(int i = 0; i < 16 * TERMINAL_ROWS; i++) {
			((uint8_t *)memLocation)[i] = hexStrToInt(&hexBuffer[i*2], 2);
		}
	}
	else if(selectedBuffer == 1) { //ascii buffer
		//update only one row 
		for(int i = 0; i < 16 * TERMINAL_ROWS; i++) {
			((uint8_t *)memLocation)[i] = asciiBuffer[i];
		}
	}
}

void processCommand(void) {
	int commandLength = 0;
	int cmpLength = 4;
	int address;
	int addressOffset;
	int addressLength;
	int arg;
	int argOffset;
	int argLength;
	uint8_t shouldParseAddress = 0;
	uint8_t commandId = 0;
	uint8_t isGood = 1;
	
	//clear status buffer
	for(int i = 0; i < 32; i++) {
		statusBuffer[i] = ' ';
	}
	
	//find length of command by index of first space (or invisible char)
	while(commandBuffer[commandLength] > 0x20 && commandLength < 32)
		commandLength++;
	
	if(commandLength > 4)
		cmpLength = commandLength;
	
	if(strncmp(commandBuffer, "goto", cmpLength) == 0) {
		commandId = 1;
		shouldParseAddress = 1;
	}
	else if(strncmp(commandBuffer, "call", cmpLength) == 0) {
		commandId = 2;
		shouldParseAddress = 1;
	}
	else if(strncmp(commandBuffer, "pciEnum", cmpLength) == 0) {
		commandId = 3;
		shouldParseAddress = 1;
	}
	
	if(shouldParseAddress) {
		//bypass spaces
		while(commandBuffer[commandLength] <= 0x20 && commandLength < 32) {
			commandLength++;
		}
		
		//start of 1st argument
		addressOffset = commandLength;
		
		//get number of hex digits
		while((commandBuffer[commandLength] >= '0' &&
		  commandBuffer[commandLength] <= '9' ||
		  (commandBuffer[commandLength] | 0x20) >= 'a' && 
		  (commandBuffer[commandLength] | 0x20) <= 'f') && 
		  commandLength < 32) {
			commandLength++;
		}
		
		addressLength = commandLength - addressOffset;
		
		//invalid literal
		if(addressLength == 0 || commandBuffer[commandLength] > 0x20) {
			strncpy_safe(statusBuffer, "[Invalid args; must be base 16]", 31);
			isGood = 0;
		}
		else { //parse hex
			address = hexStrToInt(&commandBuffer[addressOffset], addressLength);
		}
	}
	
	if(isGood) {
		if(commandId == 1) { //goto
			strncpy_safe(statusBuffer, "[goto successful]", 17);
			memLocation = address;
		}
		else if(commandId == 2) {
			strncpy_safe(statusBuffer, "[call successful]", 17);
			((void (*)(void))address)();
		}
		else if(commandId == 3) {
			//bypass spaces
			while(commandBuffer[commandLength] <= 0x20 && commandLength < 32) {
				commandLength++;
			}
			
			//start of 1st argument
			argOffset = commandLength;
			
			//get number of decimal digits
			while(commandBuffer[commandLength] >= '0' &&
			  commandBuffer[commandLength] <= '9' &&
			  commandLength < 32) {
				commandLength++;
			}
			
			argLength = commandLength - argOffset;
			
			//invalid literal
			if(argLength == 0 || commandBuffer[commandLength] > 0x20) {
				strncpy_safe(statusBuffer, "[Invalid args; must be base 10]", 31);
				isGood = 0;
			}
			else { //parse decimal
				arg = decStrToInt(&commandBuffer[argOffset], argLength);
			}

			if(isGood) {
				int numFn = pciEnumerate((uint16_t *) address, arg);

				char tmp[6];
				intToDecStr(tmp, numFn, 5);

				strncpy_safe(statusBuffer, "[pciEnum successful (", 21);
				strncpy_safe(statusBuffer + 21, tmp, 5);
				strncpy_safe(statusBuffer + 26, ")]", 2);

				//temporary tests
				intToHexStr(extraBuffer, pciConfigReadInt8(0, 0, 0, PCI_HDR_VENDOR_ID), 2);
				extraBuffer[2] = ' ';
				intToHexStr(&extraBuffer[3], pciConfigReadInt16(0, 0, 0, PCI_HDR_VENDOR_ID), 4);
				extraBuffer[7] = ' ';
				intToHexStr(&extraBuffer[8], pciConfigReadInt16(0, 0, 0, PCI_HDR_REVISION_ID), 8);
				extraBuffer[16] = ' ';

				//end of tests
			}
		}
		else {
			strncpy_safe(statusBuffer, "[Invalid command.]", 18);
		}
	}
	
	//clear command buffer
	for(int i = 0; i < 32; i++) {
		commandBuffer[i] = ' ';
		cursorCol = 0; //reset cursor
	}
}

void keyboardHandler(uint8_t c, uint8_t keyCode, uint16_t flags) {
	
	//ensure memory-buffer coherency
	uint8_t memData;
		
	for(int i = 0; i < 16 * TERMINAL_ROWS; i++) {
		memData = ((uint8_t *)memLocation)[i];
		intToHexStr(&hexBuffer[i*2], memData, 2);
		asciiBuffer[i] = memData;
	}
	
	//change cursor if an arrow key was pressed OR enter command
	
	if(c == 0x1B) { //select command buffer
		if(selectedBuffer == 2) selectedBuffer = 0;
		else selectedBuffer = 2;
		cursorCol = 0;
		cursorRow = 0;
	}
	else if(c == '\n' && selectedBuffer == 2) { //command entered
		processCommand();
	}
	else if(c == 0x81) { //up arrow
		cursorCol &= ~1; //first hex digit, if applicable
		cursorRow--;
	}
	else if(c == 0x83) { //left arrow
		cursorCol &= ~1; //first hex digit, if applicable
		cursorCol -= 2; //move by byte position
	}
	else if(c == 0x84) { //right arrow
		cursorCol &= ~1; //first hex digit, if applicable 
		cursorCol += 2; //move by byte position
	}
	else if(c == 0x86) { //down arrow
		cursorCol &= ~1; //first hex digit, if applicable
		cursorRow++;
	}
	else { //type a character
		if(selectedBuffer == 0) { //typed in hex buffer
			//hex chars only
			if(c >= '0' && c <= '9') {
				hexBuffer[32*cursorRow + cursorCol] = c;
				updateMemory();
				cursorCol++;
			}
			else if((c | 0x20) >= 'a' && (c | 0x20) <= 'f') {
				hexBuffer[32*cursorRow + cursorCol] = c & ~0x20;
				updateMemory();
				cursorCol++;
			}
			
			//go to next row within selected buffer
			if(cursorCol >= 32) {
				cursorCol = 0;
				cursorRow++;
			}
		}
		else if(selectedBuffer == 1) { //typed in ascii buffer
			//any displayable character
			if(c >= 0x20 && c < 0x7F) {
				asciiBuffer[16*cursorRow + cursorCol/2] = c;
				updateMemory();
				cursorCol &= ~1; //should be even index for ascii buffer
				cursorCol += 2; //move by byte position
			}
			
			//go to next row within selected buffer
			if(cursorCol >= 32) {
				cursorCol = 0;
				cursorRow++;
			}
		} else if(selectedBuffer == 2) { //typed in command buffer
			//any displayable character
			if(c >= 0x20 && c < 0x7F) {
				commandBuffer[cursorCol] = c;
				cursorCol++;
			}
		}
	}
	
	//handle cursor out of bounds
	if(cursorRow >= TERMINAL_ROWS) { //scroll down
		memLocation += 16;
		cursorRow = TERMINAL_ROWS - 1;
	}
	else if(cursorRow < 0) { //scroll up
		memLocation -= 16;
		cursorRow = 0;
	}
	else if(cursorCol < 0) { //switch to hex or don't move
		if(selectedBuffer == 1) {
			selectedBuffer = 0;
			cursorCol = 30;
		}
		else if(selectedBuffer == 0 || selectedBuffer == 2) {
			cursorCol = 0;
		}
	}
	else if(cursorCol >= 32) { //switch to ascii or don't move
		if(selectedBuffer == 0) {
			selectedBuffer = 1;
			cursorCol = 0;
		}
		else if(selectedBuffer == 1) {
			cursorCol = 30;
		}
		else if(selectedBuffer == 2) {
			cursorCol = 31;
		}
	}
	
	updateDisplay();
}

//entry point from bootloader
void _start(void) {
	clearScreen();
	setInterruptDescriptor(isr_keyboard, 0x21, 0);
	loadIdt();
	pic_init();
	keyboard_init(keyboardHandler);
	strncpy_safe(statusBuffer, "[J. Kent Wirant, 2022]", 22);
	
	for(int i = 0; i < 320; i++) {
		extraBuffer[i] = ' ';
	}

	extraBuffer[320] = 0;
	updateDisplay();
	while(1); //hang
}
