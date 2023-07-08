/* J. Kent Wirant
 * 20 Dec. 2022
 * osmium
 * string_util.c
 * Description: Custom string functions somewhat resembling those in
 *   ANSI C string library.
 */

#include "string_util.h"

/* Just like strncat, but last character in dest string is always 
 * null-terimated, and no null-padding is performed. The length of the
 * destination string (including the null character) is at most bufsize
 * characters.
 */
//char *strncat_safe(char *dest, const char *src, size_t n, size_t bufsize);

/* Just like strncpy, but last character in dest string is always 
 * null-terimated, and no null-padding is performed.
 */
char *strncpy_safe(char *dest, const char *src, size_t n) {
	for(int i = 0; i < n; i++) {
		dest[i] = src[i];
		if(src[i] == 0) break; //stop at null character
	}
	
	return dest;
}

//same as ANSI C definition
int strcmp(const char *s1, const char *s2) {
	int i;
	for(i = 0; (s1[i] == s2[i]) && (s1[i] != 0); i++);
	return s1[i] - s2[i];
}

//same as ANSI C definition
int strncmp(const char *s1, const char *s2, size_t n) {
	int i;
	for(i = 0; (i < n) && (s1[i] == s2[i]) && (s1[i] != 0); i++);
	return (i < n) ? (s1[i] - s2[i]) : 0;
}

size_t strlen(const char *s) {
	size_t len = 0;
	while(s[len]) len++;
	return len;
}

void intToHexStr(char *dest, int val, int n) {
	int rem;
	dest[n] = 0;
	
	while(n > 0) {
		n--;
		rem = val & 0xF;
		val >>= 4;
		dest[n] = ((rem < 10) ? ('0' + rem) : ('A' + rem - 10)); 
	}
}

int hexStrToInt(const char *src, int n) {
	int val = 0;
	
	for(int i = 0; i < n; i++) {
		val <<= 4;
		val |= (src[i] <= '9') ? (src[i] - '0') : ((src[i] | 0x20) - 'a' + 10);
	}
	
	return val;
}

void intToDecStr(char *dest, int val, int n) {
	int rem;
	dest[n] = 0;
	
	while(n > 0) {
		n--;
		rem = val % 10;
		val /= 10;
		dest[n] = rem + '0'; 
	}
}

int decStrToInt(const char *src, int n) {
	int val = 0;
	
	for(int i = 0; i < n; i++) {
		val *= 10;
		val += src[i] - '0';
	}
	
	return val;
}
