/* J. Kent Wirant
 * 19 Dec. 2022
 * ECE 1895 - Project 3
 * string_util.h
 * Description: Custom string functions somewhat resembling those in
 *   ANSI C string library.
 */

#ifndef STRING_H
#define STRING_H

typedef unsigned long int size_t;
//strcat and strcpy have been excluded to avoid buffer overflows.

/* Just like strncat, but last character in dest string is always 
 * null-terimated, and no null-padding is performed. The length of the
 * destination string (including the null character) is at most bufsize
 * characters.
 */
//char *strncat_safe(char *dest, const char *src, size_t n, size_t bufsize);

/* Just like strncpy, but last character in dest string is always 
 * null-terimated, and no null-padding is performed.
 */
char *strncpy_safe(char *dest, const char *src, size_t n);

//same as ANSI C definition
int strcmp(const char *s1, const char *s2);

//same as ANSI C definition
int strncmp(const char *s1, const char *s2, size_t n);

//same as ANSI C definition
size_t strlen(const char *s);

//converts integer to hex string with n digits and null at the end 
void intToHexStr(char *dest, int val, int n);

//parses integer from hex string with n digits. 
//input is assumed to be valid.
int hexStrToInt(const char *src, int n);

//converts integer to decimal string with n digits and null at the end 
void intToDecStr(char *dest, int val, int n);

//parses integer from decimal string with n digits. 
//input is assumed to be valid.
int decStrToInt(const char *src, int n);

#endif
