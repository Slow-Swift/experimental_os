#pragma once

#include <stddef.h>

/**
 * Copy n bytes from the opject pointed to by s2 into the opbject pointed to by
 * s1. 
 * Parameters:
 *   s1: The address to copy to
 *   s2: The address to copy from
 *   n: The number of bytes to copy
 * 
 * Returns: 
 *   A pointer to the first byte copied
*/
void *memcpy(void * restrict s1, const void * restrict s2, size_t n);

/**
 * Copy n bytes from the opject pointed to by s2 into the opbject pointed to by
 * s1 as though s2 is first copied to a temporary buffer which is then copied
 * to s1.
 * Parameters:
 *   s1: The address to copy to
 *   s2: The address to copy from
 *   n: The number of bytes to copy
 * 
 * Returns: 
 *   A pointer to the first byte copied
*/
void *memmove(void *s1, const void *s2, size_t n);

/**
 * Copy the string pointed to by s2 into the string pointed to by s1.
 * Parameters:
 *   s1: The string to copy to
 *   s2: The string to copy from
 * 
 * Returns: 
 *   A pointer to the first character copied
*/
char *strcpy(char * restrict s1, const char * restrict s2);

/**
 * Copy a maximum of n characters from s2 to s1. If there are less than n
 * characters in s2, then s1 is padded with null characters until there
 * are n characters total
 * Parameters:
 *   s1: The string to copy to
 *   s2: The string to copy from
 *   n: The maximum number of characters to copy
 * 
 * Returns: 
 *   A pointer to the first character copied
*/
char *strncpy(char * restrict s1, const char * restrict s2, size_t n);

/**
 * Append a copy of the string pointed to by s2 to the end of the string pointed
 * to by s1, replacing the null character of s1 by the first character of s2.
 * Parameters:
 *   s1: The string to append to
 *   s2: The string to append
 * 
 * Returns:
 *   A pointer to s1
*/
char *strcat(char * restrict s1, const char * restrict s2);

/**
 * Append not more than n characters from the string pointed to by s2 to the 
 * end of the string pointed to by s1, replacing the null character of s1 by 
 * the first character of s2.
 * Parameters:
 *   s1: The string to append to
 *   s2: The string to append
 *   n: The maximum number of characters to append to s1
 * 
 * Returns:
 *   A pointer to s1
*/
char *strncat(char * restrict s1, const char * restrict s2, size_t n);

/**
 * Compare the first n characters of the object pointed to by s1 to the first n
 * characters of the object pointed to by s2
 * 
 * Parameters:
 *   s1: One of the objects to compare
 *   s2: The second object to compare
 *   n: The number of bytes to compare
 * 
 * Returns: 
 *   an integer greater then, equal to, or less than zero, accordingly as the
 *   object pointed to by s1 is greater than equal to or less than the 
 *   object pointed to by s2
*/
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * Compare the string pointed to by s1 to the string pointed to by s2
 * 
 * Parameters:
 *   s1: The first string to compare
 *   s2: The second string to compare
 * 
 * Returns: 
 *   an integer greater then, equal to, or less than zero, accordingly as the
 *   string pointed to by s1 is greater than equal to or less than the 
 *   string pointed to by s2
*/
int strcmp(const char *s1, const char *s2);

/**
 * Compare up to n characters of the string pointed to by s1 to the string 
 * pointed to by s2
 * 
 * Parameters:
 *   s1: The first string to compare
 *   s2: The second string to compare
 *   m: The maximum number of characters to compare
 * 
 * Returns: 
 *   an integer greater then, equal to, or less than zero, accordingly as the
 *   string pointed to by s1 is greater than equal to or less than the 
 *   string pointed to by s2
*/
int strncmp(const char *s1, const char *s2, size_t n);

/**
 * Find the first occurrence of c (casted to an unsigned char) in the object
 * pointed to by s1.
 * 
 * Parameters:
 *   s1: The object to search through
 *   c: The byte to find
 *   n: The number of bytes in s1
 * 
 * Returns:
 *   A pointer to the first occurrence of c in s1
*/
void *memchr(const void *s1, int c, size_t n);

/**
 * Find the first occurrence of c (casted to a char) in the string pointed to 
 * by s1.
 * 
 * Parameters:
 *   s1: The string to search through
 *   c: The character to find
 * 
 * Returns:
 *   A pointer to the first occurrence of c in s1
*/
char *strchr(const char *s, int c);

/**
 * Determine the size of the largest prefix of s1 that consists only of 
 * characters that are not in s2.
 * 
 * Parameters:
 *   s1: The string to find the prefix of
 *   s2: The string of disallowed characters
 * 
 * Returns:
 *   The length of the largest prefix
*/
size_t strcspn(const char * s1, const char * s2);

/**
 * Find the first character in s1 that is also in s2
 * 
 * Parameters:
 *   s1: The string to search throught
 *   s2: The string of characters to search for
 * 
 * Returns:
 *   A pointer to the first character in s1 that is also in s2
*/
void *strpbrk(const char * s1, const char * s2);

/**
 * Find the last occurrence of c (casted to a char) in the string pointed to 
 * by s1.
 * 
 * Parameters:
 *   s1: The string to search through
 *   c: The character to find
 * 
 * Returns:
 *   A pointer to the last occurrence of c in s1
*/
char *strrchr(const char *s, int c);

/**
 * Determine the size of the largest prefix of s1 that consists only of 
 * characters that are in s2.
 * 
 * Parameters:
 *   s1: The string to find the prefix of
 *   s2: The string of allowed characters
 * 
 * Returns:
 *   The length of the largest prefix
*/
size_t strspn(const char *s1, const char *s2);

/**
 * Find the first occurrence of the string pointed to by s2 in the string 
 * pointed to by s1
 * 
 * Parameters:
 *   s1: The string to search through
 *   s2: The string to search for
 * 
 * Returns:
 *   A pointer to the first occurrence of s2 in s1
*/
char *strstr(const char *s1, const char *s2);

/**
 * Separate a string into tokens separated by characters in s2. If called with
 * a non-NULL value for s1, s1 becomes the string that will be tokenized. If
 * called with a NULL value for s1, then tokenization continues on the previous
 * string.
 * 
 * Parameters:
 *   s1: The string to tokenize or NULL to continue tokenizing the previous
 *       string
 *   s2: The string of separator characters
 * 
 * Returns:
 *   A pointer to the next token or NULL if there is no more tokens
*/
char *strtok(char * restrict s1, const char * restrict s2);

/**
 * Used by library functions so that strtok can be used without detroying
 * the string that is being tokenized. Call this then use strtok, and then call
 * set_s_strtok.
 * 
 * Returns:
 *   A pointer to the current string being tokenized
*/
char *get_s_strtok();

/**
 * Used by library functions to restore the string being tokenized
 * 
 * Parameters:
 *   s: The string to tokenize
*/
void set_s_strtok(char *s);

/**
 * Fill n bytes of the object pointed to by s with the c (casted to an unsigned
 * char).
 * 
 * Parameters:
 *   s: The object to set the bytes of
 *   c: The value to set the bytes to
 *   n: The number of bytes to set
 * 
 * Returns:
 *   A pointer to s
*/
void *memset(void *s, int c, size_t n);

/**
 * Convert an error code to an error message. The error message should not
 * be modified.
 * 
 * Parameters:
 *   errnum: The error code
 * 
 * Returns:
 *   A pointer to the error message.
*/
char *strerror(int errnum);

/**
 * Determine the number of characters in the string pointed to by s.
 * 
 * Parameters:
 *   s: The string the find the length of
 * 
 * Returns:
 *   The length of s
*/
size_t strlen(const char *s);