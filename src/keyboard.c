/* J. Kent Wirant
 * osmium
 * keyboard.c
 * Description: PS/2 keyboard driver plus enabling A20.
 */
 
//referenced https://wiki.osdev.org/PS/2_Keyboard
//referenced https://wiki.osdev.org/%228042%22_PS/2_Controller

#include <stdint.h>
#include "x86_util.h"
#include "text_util.h"
#include "string_util.h"
#include "keyboard.h"

#define KEYBOARD_CMD_QUEUE_SIZE 16

typedef uint8_t char_t;

static const uint16_t DATA_PORT = 0x60;
static const uint16_t CMD_STAT_PORT = 0x64;

static const uint8_t RESPONSE_ERR0              = 0x00;
static const uint8_t RESPONSE_SELF_TEST_PASSED  = 0xAA;
static const uint8_t RESPONSE_ECHO              = 0xEE;
static const uint8_t RESPONSE_ACK               = 0xF0;
static const uint8_t RESPONSE_SELF_TEST_FAILED0 = 0xFC;
static const uint8_t RESPONSE_SELF_TEST_FAILED1 = 0xFD;
static const uint8_t RESPONSE_RESEND            = 0xFE;
static const uint8_t RESPONSE_ERR1              = 0xFF;

static char_t state_start(uint8_t scancode);
static char_t state_extendedCode(uint8_t scancode);
static char_t state_pause(uint8_t scancode);
static char_t state_pausePr(uint8_t scancode);
static char_t state_pauseRl(uint8_t scancode);
static char_t state_prtscPr1(uint8_t scancode);
static char_t state_prtscPr2(uint8_t scancode);
static char_t state_prtscRl1(uint8_t scancode);
static char_t state_prtscRl2(uint8_t scancode);
static char_t state_error(uint8_t scancode);

typedef char_t (*KeyboardState)(uint8_t);
KeyboardState keyboardState = state_start;

static struct Command {
	enum CommandID cmdId;
	uint8_t data;
};

//uses scan code set 1
static char_t scanCodeTable[] = {
	0x00, 0x1B, '1',  '2',
	'3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',
	'-',  '=',  '\b', '\t',
	'q',  'w',  'e',  'r',
	't',  'y',  'u',  'i',
	'o',  'p',  '[',  ']',
	'\n', 0x02, 'a',  's',
	'd',  'f',  'g',  'h',
	'j',  'k',  'l',  ';',
	'\'', '`',  0x03, '\\',
	'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',
	'.',  '/',  0x04, '*',
	0x05, ' ',  0x06, 0x07,
	0x0E, 0x0F, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x18, '7', //keypad entries
	'8',  '9',  '-',  '4',
	'5',  '6',  '+',  '1',
	'2',  '3',  '0',  '.',
	0x00, 0x00, 0x00, 0x19,
	0x1A, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x1C, 0x00, 0x00, 0x00, //extended key codes; add 0x50 to access
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x1D, 0x00, 0x00,
	'\n', 0x1E, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0x00, //0xFF -> key exists but is unimplemented
	0xFF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0xFF, 0x00,
	0x00, '/',  0x00, 0x00,
	0x1F, 0x00, 0x00, 0x00, //0x38
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x80,
	0x81, 0x82, 0x00, 0x83,
	0x00, 0x84, 0x00, 0x85,
	0x86, 0x87, 0x88, 0x7F,
	0x00, 0x00, 0x00, 0x00, //0x54
	0x00, 0x00, 0x00, 0x89,
	0x8A, 0x8B, 0x8C, 0x8D,
	0x00, 0x00, 0x00, 0x8E,
	0x00, 0x8F, 0x90, 0x91, //0x64
	0x92, 0x93, 0x94, 0x95,
	0x96, 0x97, 0x00, 0x00  //0x6C
};

static char_t shiftTable[] = {
	0x00, 0x1B, '!',  '@',
	'#',  '$',  '%',  '^',
	'&',  '*',  '(',  ')',
	'_',  '+',  '\b', '\t',
	'q',  'w',  'e',  'r',
	't',  'y',  'u',  'i',
	'o',  'p',  '{',  '}',
	'\n', 0x02, 'a',  's',
	'd',  'f',  'g',  'h',
	'j',  'k',  'l',  ':',
	'"',  '~',  0x03, '|',
	'z',  'x',  'c',  'v',
	'b',  'n',  'm',  '<',
	'>',  '?'
};

/* keyFlags:
 *   bit 0 - set if key pressed, clear if released
 *   bit 1 - set if CAPS is on
 *   bit 2 - set if NumLk is on
 *   bit 3 - set if ScrLk is on
 *   bit 4 - set if Left Shift is down
 *   bit 5 - set if Right Shift is down
 *   bit 6 - set if Left Ctrl is down
 *   bit 7 - set if Right Ctrl is down
 *   bit 8 - set if Left Alt is down
 *   bit 9 - set if Right Alt is down
 *   bits 10 to 15 - reserved; should be clear
 */
static uint16_t keyFlags = 0;

//callback function
void (*keyEventHandler)(char_t c, uint8_t keyCode, uint16_t flags) = 0;

struct Command cmdQueue[KEYBOARD_CMD_QUEUE_SIZE];
uint32_t queueStart = 0;
uint32_t queueLength = 0;

//function prototypes
uint8_t keyboard_queueCommand(enum CommandID id, uint8_t data);
void processScanCode(uint8_t scancode);
void tryCommand(void);
uint8_t keyboard_checkInput(void);
void keyboard_init(void (*handler)(char_t, uint8_t, uint16_t));

//returns true if sucessfully added to queue. Does not validate command.
uint8_t keyboard_queueCommand(enum CommandID id, uint8_t data) {
	//if queue is not full, assign new element
	uint8_t hasRoom = (queueLength < KEYBOARD_CMD_QUEUE_SIZE);
	
	if(hasRoom) {
		int idx = (queueStart + queueLength) % KEYBOARD_CMD_QUEUE_SIZE;
		struct Command *cmd = &cmdQueue[idx];
		cmd->cmdId = id;
		cmd->data = data;
		queueLength++;
	}
	
	return hasRoom;
}

static char_t state_start(uint8_t scancode) {
	switch(scancode) {
		case 0xE0:
			keyboardState = state_extendedCode;
			break;
		case 0xE1:
			keyboardState = state_pause;
			break;
		default:
			return scanCodeTable[scancode & 0x7F];
	}
	return 0;
}

//TODO: implement await state
static char_t state_awaitingResponse(uint8_t scancode) {
	keyboardState = state_start;
}

static char_t state_extendedCode(uint8_t scancode) {
	switch(scancode) {
		case 0x2A:
			keyboardState = state_prtscPr1;
			break;
		case 0xB7:
			keyboardState = state_prtscRl1;
			break;
		default:
			keyboardState = state_start;
			return scanCodeTable[(scancode & 0x7F) + 0x50];
	}
	return 0;
}

static char_t state_pause(uint8_t scancode) {
	switch(scancode) {
		case 0x1D:
			keyboardState = state_pausePr;
			break;
		case 0x9D:
			keyboardState = state_pauseRl;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_pausePr(uint8_t scancode) {
	switch(scancode) {
		case 0x45:
			//pause press event
			keyboardState = state_start;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_pauseRl(uint8_t scancode) {
	switch(scancode) {
		case 0xC5:
			//pause release event
			keyboardState = state_start;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_prtscPr1(uint8_t scancode) {
	switch(scancode) {
		case 0xE0:
			keyboardState = state_prtscPr2;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_prtscRl1(uint8_t scancode) {
	switch(scancode) {
		case 0xE0:
			keyboardState = state_prtscRl2;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_prtscPr2(uint8_t scancode) {
	switch(scancode) {
		case 0x37:
			//print screen pressed event
			keyboardState = state_start;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_prtscRl2(uint8_t scancode) {
	switch(scancode) {
		case 0xAA:
			//print screen released event
			keyboardState = state_start;
			break;
		default:
			keyboardState = state_error;
	}
	return 0;
}

static char_t state_error(uint8_t scancode) {
	keyboardState = state_start;
	return 0;
}

static int isCapsLockActive() {
	return (keyFlags & 2) != 0;
}

static int isShiftActive() {
	return (keyFlags & 48) != 0; //R or L shift
}

static int isReleased() {
	return (keyFlags & 1) == 0;
}

static int isPressed() {
	return (keyFlags & 1) == 1;
}

static void updateFlags(char_t c, uint8_t scancode) {
	int bit = 0;

	if(c == 0x06) bit = 1; //CAPS lock
	else if(c == 0x03) bit = 4; //left shift
	else if(c == 0x04) bit = 5; //right shift
	else if(c == 0x02) bit = 6; //left control
	else if(c == 0x1E) bit = 7; //right control
	else if(c == 0x05) bit = 8; //left alt
	else if(c == 0x1F) bit = 9; //right alt
	
	//pressed/released flag
	keyFlags &= ~1;
	keyFlags |= ~(scancode >> 7) & 1;
	
	//set or clear appropriate bit in keyFlags
	if(bit >= 4) {
		if(scancode & 0x80) keyFlags &= ~(1 << bit); //released
		else keyFlags |= (1 << bit); //pressed
	}
	else if(bit >= 1 && (scancode & 0x80) == 0) { //lock-type keys (toggle-based)
		keyFlags ^= (1 << bit);
	}
}

static char_t applyKeyModifiers(char_t c, uint8_t scancode) {
	//switch letter to uppercase if (CAPS LOCK) XOR (R OR L shift)
	if(c >= 'a' && c <= 'z' && (isCapsLockActive() ^ isShiftActive())) {
		c &= ~0x20;
	}
	else if(scancode < 0x36 && isShiftActive()) { //if R OR L shift is pressed
		c = shiftTable[scancode & 0x7F];
	}
	return c;
}

void processScanCode(uint8_t scancode) {
	char_t c = keyboardState(scancode);
	
	//process key only if the state has a valid output
	if(c != 0) {
		updateFlags(c, scancode);
		
		if(keyEventHandler != 0 && isPressed()) {
			c = applyKeyModifiers(c, scancode);
			keyEventHandler(c, scancode, keyFlags);
		}
	}
}

void tryCommand(void) {
	if(keyboardState != state_start) //only process command when ready
		return;
	
	if(queueLength > 0) {
		int idx = (queueStart + queueLength) % KEYBOARD_CMD_QUEUE_SIZE;
		struct Command *cmd = &cmdQueue[idx];
				
		switch(cmd->cmdId) {
			//does not work as intended yet
			case CMD_SCAN_CODE_SET:
				//set scan code set to 1
				keyboardState = state_awaitingResponse;	
				x86_outb(DATA_PORT, 0xF0);
				x86_outb(DATA_PORT, 0x01);	
				break;
		}
	}
}

//returns true if input was found and processed
uint8_t keyboard_checkInput(void) {
	//read status register for output buffer status
	uint8_t isFull = x86_inb(CMD_STAT_PORT) & 1;
	
	if(isFull) {
		uint8_t data = x86_inb(DATA_PORT);
		
		//NOTE: command response is not fully tested
		if(keyboardState == state_awaitingResponse) {
			keyboardState = state_start; //no longer awaiting response
			
			if(data == RESPONSE_ACK) { //if acknowledged, remove cmd from queue		
				queueLength--;
				queueStart = (queueStart + 1) % KEYBOARD_CMD_QUEUE_SIZE;
			}
			else if(data == RESPONSE_RESEND) { //resend last command
				tryCommand();
			} 
			else {
				//return to start state for robustness	
				keyboardState = state_start;
			}
				
			/* no action needed yet for:
				RESPONSE_ERR0
				RESPONSE_ERR1
				RESPONSE_SELF_TEST_FAILED0
				RESPONSE_SELF_TEST_FAILED1
				RESPONSE_SELF_TEST_PASSED
				RESPONSE_ECHO 
			*/	
		}
		else { //received key input
			processScanCode(data);
		}
	}
	
	return isFull;
}

void keyboard_init(void (*handler)(char_t, uint8_t, uint16_t)) {
	//TODO: reset keyboard & check status
	
	keyEventHandler = handler;
	keyboardState = state_start;
	
	//TODO: enable A20
}


