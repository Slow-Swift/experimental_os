#include "bootdata.h"

/**
 * Initialize the memory manager.
 * 
 * Parameters:
 *   boot_data: The information passed by the bootloader
 * 
 * Returns:
 *   The number of free bytes of memory
*/
long long int memory_initialize(BootData* boot_data);