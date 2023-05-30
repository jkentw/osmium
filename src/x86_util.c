/* J. Kent Wirant
 * 14 Dec. 2022
 * ECE 1895 - Project 3
 * x86_util.h
 * Description: Utility functions in x86 assembly.
 */
 
#include "x86_util.h"

// I/O instructions

uint8_t x86_inb(uint16_t port) {
	uint8_t data;
	asm volatile ("in %%dx, %%al" : "=a" (data) : "d" (port) : "memory");
	return data;
}

uint16_t x86_inw(uint16_t port) {
	uint16_t data;
	asm volatile ("in %%dx, %%ax" : "=a" (data) : "d" (port) : "memory");
	return data;
}

uint32_t x86_ind(uint16_t port) {
	uint32_t data;
	asm volatile ("in %%dx, %%eax" : "=a" (data) : "d" (port) : "memory");
	return data;
}

void x86_outb(uint16_t port, uint8_t data) {
	asm volatile ("out %%al, %%dx" : : "a" (data), "d" (port) : "memory");
}

void x86_outw(uint16_t port, uint16_t data) {
	asm volatile ("out %%ax, %%dx" : : "a" (data), "d" (port) : "memory");
}

void x86_outd(uint16_t port, uint32_t data) {
	asm volatile ("out %%eax, %%dx" : : "a" (data), "d" (port) : "memory");
}

//referenced https://wiki.osdev.org/Model_Specific_Registers
//read from model specific register 
void x86_readMSR(uint32_t msr, uint32_t *lo, uint32_t *hi) {
	asm volatile ("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr));
}

//write to model specific register 
void x86_writeMSR(uint32_t msr, uint32_t lo, uint32_t hi) {
	asm volatile ("wrmsr" : : "a" (lo), "d" (hi), "c" (msr));
}
