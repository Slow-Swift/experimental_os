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
 *   object pointed bo by s1 is greater than equal to or less than the 
 *   object pointed to by s2
*/
int memcmp(const void *s1, const void *s2, size_t n);