#include "memory.h"

#include "defs.h"
#include "debug.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// #define DEBUG_MEMORY 0

static const size_t malloc_align_size = 8;

extern char __start;
extern char __end;

/**
 * Initial Setup:
 *   Get regions from boot data
 *   Sort regions based on start address
 *   Create memory nodes for each region
*/

typedef struct MemoryNode {
    unsigned int size;
    struct MemoryNode* next;
} MemoryNode;

typedef struct {
    size_t total_size;
    size_t requested_size;
    char offset;
} AllocatedRegionHeader;

enum MemoryRegionType {
    AVAILIABLE = 1,
    RESERVED = 2,
    ACPI_RECLAIM = 3,
    ACPI_NVS = 4
};

static MemoryNode* start;

/**
 * Free [length] bytes of memory starting from [base]
 * 
 * Parameters:
 *   base: The address of the first byte to free
 *   length: The number of bytes to free
 * 
 * Preconditions:
 *   none of the bytes to free are already free
*/
static void free_memory(pointer_t base, long long int length) 
{
    pointer_t end = base + length;
#ifdef DEBUG_MEMORY
    log_info("Memory", "Freeing %llx bytes from %lx to %lx", length, base, end);
#endif

    MemoryNode* previous = NULL;
    MemoryNode* next = start;

    // Find the node before the memory to free
    while (next != NULL && base > (pointer_t)next) {
        previous = next;
        next = next->next;
    }

    MemoryNode* current = (MemoryNode*)base;

    // If freeing memory will form a continuous region with the previous
    // section then combine them
    if (previous != NULL && (pointer_t)previous + previous->size >= base)
        current = previous;
    else if (previous != NULL)
        previous->next = current;

    // Save information about the freed memory
    current->size = end - (pointer_t)current;
    current->next = next;

    // If the freed memory will form a continuous region with the next section
    // then combine them
    if (next != NULL && (pointer_t)next <= end) {
        pointer_t next_end = (pointer_t)next + next->size;
        current->size = next_end - (pointer_t)current;
        current->next = next->next;
    }

    // Update the start node
    if (next == start)
        start = current;
}

/**
 * Print out the memory regions in a nice table
 * 
 * Parameters:
 *   memory_regions: The array of memory regions to print
 *   region_count: The number of regions in the array
*/
static void print_memory_regions(MemoryRegion* memory_regions, int region_count) 
{

    printf("Memory Regions: \n");
    printf("|             BASE |              END |           LENGTH |     TYPE |\n");
    for (int i=0; i<region_count; i++) {
        printf(
            "| %16llx | %16llx | %16llx | %8lx |\n", 
            memory_regions[i].BaseAddress, 
            memory_regions[i].BaseAddress + memory_regions[i].Length, 
            memory_regions[i].Length, 
            memory_regions[i].Type
        );
    }
}

/**
 * Sort memory regions in [boot_data] by their base address
 * 
 * Parameters:
 *   memory_regions: The array of memory regions to sort
 *   region_count: The number of regions in the array
*/
static void sort_regions(MemoryRegion* memory_regions, int region_count) 
{
    MemoryRegion temp;

    // Sort using insertion sort
    for (int i = 1; i < region_count; i++) {
        // Move region i backwards until it is in the correct place
        // Assumes regions 0..i-1 already sorted
        for (int j = i; j > 0; j--) {
            // If larger than previous region then sorted
            if (
                memory_regions[j - 1].BaseAddress 
                <= memory_regions[j].BaseAddress
            ) break;

            // Swap the regions
            temp = memory_regions[j-1];
            memory_regions[j-1] = memory_regions[j];
            memory_regions[j] = memory_regions[j-1];
        }
    }
}

/**
 * Free memory that is not restricted by the memory regions passed by the 
 * bootloader. That is any memory that is not marked as not avaible and that
 * later regions do not also mark as availiable.
 * 
 * Parameters:
 *   memory_regions: The array of memory regions
 *   region_count: The number of regions in the array
 *   base: The first byte of memory to free
 *   end: The last byte + 1 of the memory to free
 *   ignore_index: The index of the memory regions that should not restrict
 *     the memory. (Probably where this memory is from).
 * 
 * Returns:
 *   The number of bytes freed
 * 
 * Precondition:
 *   The memory regions are sorted by base address
 * 
*/
static long long int free_nonrestricted_mem(
    MemoryRegion* memory_regions, int region_count, 
    pointer_t base, pointer_t end, int ignore_index
) {
    long long int bytes_added = 0;

    // Add the parts of the region that do not overlap with other regions
    for (int i = 0; i < region_count; i++) {
        if (ignore_index == i) continue;

        pointer_t region_base = (pointer_t)(memory_regions[i].BaseAddress);
        pointer_t region_end = region_base + memory_regions[i].Length;

        // Skip regions that do not overlap with the memory
        if (base >= region_end) continue;
        if (region_base >= end) break;

        // Ignore regions marked availiable that start before the base
        if (base > region_base && memory_regions[i].Type == AVAILIABLE)
            continue;

        // If there is avaliable space before the overlap then add it
        if (base <= region_base) {
            free_memory(base, region_base-base);
            bytes_added += region_base - base;
        }

        // The region will continue after the overlap
        base = region_base;
        if (base >= end) break;
    }

    // Add any remaining part of the region
    if (base < end) {
        free_memory(base, end-base);
        bytes_added += end - base;
    }

    return bytes_added;
}

/**
 * Free memory regions that are marked as aviliable. Memory the overlaps with
 * the kernel or is less than [first_availiable_memory] is not freed. Memory 
 * that overlaps with regions that are not availiable is not freed.
 * 
 * Parameters:
 *   memory_regions: The array of memory regions
 *   memory_region_count: The number of memory regions in the array
 *   first_availiable_memory: The address of the first byte of memory that
 *     could be allocated
 * 
 * Returns:
 *   The number of bytes freed
 * 
 * Precondition:
 *   Memory regions are sorted by their base address
*/
static long long int free_availiable_regions(
    MemoryRegion* memory_regions, int memory_region_count, 
    pointer_t first_aviliable_memory
) {
    pointer_t kernel_start = (pointer_t)&__start;
    pointer_t kernel_end = (pointer_t)&__end;

    //! Should maybe be changed to a for loop to search for first availiable 
    //! region
    // Ensure first region does not start before first availiable memory
    if (memory_regions[0].BaseAddress < first_aviliable_memory) {
        memory_regions[0].Length = memory_regions[0].BaseAddress + 
                                   memory_regions[0].Length - 
                                   first_aviliable_memory;
        memory_regions[0].BaseAddress = first_aviliable_memory;
    }

    long long int availiable_bytes = 0;
    for (int i = 0; i < memory_region_count; i++) {
        // Skip reserved regions
        if (memory_regions[i].Type != AVAILIABLE) continue;

        // Determine start and end of region
        pointer_t base = (pointer_t)(memory_regions[i].BaseAddress);
        pointer_t end = base + memory_regions[i].Length;

        // Check for overlap with the kernel
        if (base < kernel_end && kernel_start < end) {
            long long int start_bytes = kernel_start - base;
            long long int end_bytes = end - kernel_end;

            if (start_bytes > 0) {
                availiable_bytes += free_nonrestricted_mem(
                    memory_regions, memory_region_count, base, kernel_start, i
                );
            }

            if (end_bytes > 0) {
                availiable_bytes += free_nonrestricted_mem(
                    memory_regions, memory_region_count, kernel_end, end, i
                );
            }
        } else {
            availiable_bytes += free_nonrestricted_mem(
                memory_regions, memory_region_count, base, end, i
            );
        }
    }

    return availiable_bytes;
}

long long int memory_initialize(BootData* boot_data) 
{
    log_info("Memory", "Initializing Memory");

    MemoryRegion* memory_regions = (MemoryRegion*)boot_data->MemoryMapAddr;
    sort_regions(memory_regions, boot_data->MemRegionCount);
    print_memory_regions(memory_regions, boot_data->MemRegionCount);

    long long int availiable_bytes = free_availiable_regions(
        memory_regions,
        boot_data->MemRegionCount,
        boot_data->FirstAvailiableMemory
    );

    log_info("Memory", "Availiable Memory: %#llx\n", availiable_bytes);
    printf("Availiable Memory: %#llx\n", availiable_bytes);
    return availiable_bytes;
}

void* aligned_alloc(size_t alignment, size_t size) 
{
    if (size == 0) return NULL;

    MemoryNode* best_prev = NULL;
    MemoryNode* best_fit = NULL;
    MemoryNode* previous = NULL;
    MemoryNode* current = start;
    char alignment_offset = 0;

    while (current != NULL) {
        // If the best_fit is clearly better than the current node then skip
        // this one
        if (best_fit != NULL && best_fit->size <= current->size) {
            previous = current;
            current = current->next;
            continue;
        }
        
        // Determine size required for header and alignment
        int offset = sizeof(AllocatedRegionHeader);
        int align_offset = 
            alignment - ((pointer_t)current + offset) % alignment;
        offset += (alignment - align_offset) % alignment;

        // If the size is large enough then it becomes the new best fit
        if (size + offset <= current->size) {
            alignment_offset = offset;
            best_fit = current;
            best_prev = previous;
        }
        
        previous = current;
        current = current->next;
    }

    if (best_fit == NULL) return NULL;

    // Update the free memory list
    MemoryNode* next = best_fit->next;
    size_t size_with_header = size + alignment_offset;
    if (size_with_header + sizeof(MemoryNode) < best_fit->size) {
        next = (MemoryNode*)((pointer_t)(best_fit) + size_with_header);
        next->next = best_fit->next;
        next->size = best_fit->size - size_with_header;
    } else {
        size_with_header = best_fit->size;
    }

    if (best_prev == NULL) {
        start = next;
    } else {
        best_prev->next = next;
    }

#ifdef DEBUG_MEMORY
    log_info("Memory", "Allocating %lx bytes from %p to %x", 
        size_with_header, best_fit, (pointer_t)best_fit + size_with_header);
#endif

    AllocatedRegionHeader* header = (AllocatedRegionHeader*)best_fit;
    header->total_size = size_with_header;
    header->requested_size = size;
    header->offset = alignment_offset;

    pointer_t allocated_start = (pointer_t)header + alignment_offset;
    *((char*)(allocated_start-1)) = alignment_offset;

    return (void*)(allocated_start);
}

void* calloc(size_t n_memb, size_t size) {
    return aligned_alloc(malloc_align_size, size * n_memb);
}

void* malloc(size_t size) {
    return aligned_alloc(malloc_align_size, size);
}

void free(void* ptr) {
    pointer_t start = (pointer_t)ptr;
    char align_offset = *(char*)(start-1);

    AllocatedRegionHeader* header = 
        (AllocatedRegionHeader*)(start-align_offset);

    free_memory((pointer_t)header, header->total_size);
}

void* realloc(void* ptr, size_t size) {
    pointer_t start = (pointer_t)ptr;
    char align_offset = *(char*)(start-1);

    AllocatedRegionHeader* header = 
        (AllocatedRegionHeader*)(start-align_offset);

    void* new_object = malloc(size);

    size_t to_copy = header->requested_size < size 
                        ? header->requested_size 
                        : size;
    
    char* old_bytes = (char*)ptr;
    char* new_bytes = (char*)new_object;
    for (int i=0; i<to_copy; i++)
        old_bytes[i] = new_bytes[i];

    free(ptr);

    return new_object;
}