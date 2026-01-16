# Osmium - Operating System from Scratch

Osmium is my personal x86-based OS project. Currently, it features a hexadecimal editor view of system memory contents and supports some basic commands. By reading from and writing to arbitrary memory locations, one can write and execute machine code programs.

This repository features implementations for boot code, a keyboard driver, programmable interrupt controller interface, text display, and the top level memory editor application. There is active progress in improving existing code and in developing more features, such as a basic PCI driver. 

<img width="717" height="399" alt="osmium-preview" src="https://github.com/user-attachments/assets/fc9269bd-5498-4572-9a99-95471825b922" />

You can run this program yourself, too. Note that since this project is in development, there is risk of bugs that in theory can cause damage to a real hardware system. I recommend running an emulator like qemu. However, this program has run on real hardware, and loading the kernel binary to a bootable flash drive would work for this purpose. Note that this OS does not support pure UEFI boot; i.e. only BIOS-based boot is supported.

To compile from source, you should use a cross compiler and assembler (e.g. nasm) in a Linux environment (WSL is sufficient for Windows users). Simply change some values in the Makefile for your specific build environment. 
