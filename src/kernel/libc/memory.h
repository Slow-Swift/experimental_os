#pragma once

#include "bootdata.h"
#include <stddef.h>

/**
 * initialize the memory manager.
 * 
 * Parameters:
 *   boot_data: The information passed by the bootloader
 * 
 * Returns:
 *   The number of free bytes of memory
*/
long long int memory_initialize(BootData* boot_data);

/**
 * Allocates space for an object whose alignment is specified by [alignment],
 * and whose size is specified by [size].
 * 
 * Parameters:
 *   alignment: The alignment of the object
 *   size: The size of the object
 * 
 * Returns:
 *   The address of the allocated memory
*/
void* aligned_alloc(size_t alignment, size_t size);

/**
 * Allocates space for an array of [nmemb] objects, each of whose size is 
 * [size]. All bits are initialized to zero.
 * 
 * Parameters:
 *   alignment: The number of objects in the array
 *   size: The size of an object in the array
 * 
 * Returns:
 *   The address of the allocated memory
*/
void* calloc(size_t n_memb, size_t size);

/**
 * Allocates space for an object whose size is specified by [size].
 * 
 * Parameters:
 *   size: The size of the space to allocate
 * 
 * Returns:
 *   The address of the allocated memory
*/
void* malloc(size_t size);

/**
 * Deallocate the object pointed to by [ptr] and return a pointer to a new
 * object that has the size specified by [size] and which has the same
 * contents as the old object prior to deallocation. Any bytes in the new 
 * object beyond the size of the old object have indeterminate values.
 * 
 * Parameters:
 *   ptr: The pointer to the old object
 *   size: The size of the new object to allocate
 * 
 * Returns:
 *   The address of the new object
*/
void* realloc(void* ptr, size_t size);

/**
 * Free memory that was previously allocated
 * 
 * Parameters:
 *   ptr: The address of the memory to free
*/
void free(void* ptr);