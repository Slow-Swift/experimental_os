#pragma once

typedef struct ListNode {
    void *value;
    struct ListNode *next;
} ListNode;

/**
 * Add a new node to the tail of the linked list
 * 
 * Parameters:
 *   head: Pointer to the list to add to
 *   value: The value of the node to add
 * 
 * Returns:
 *   0 on success
 *   -1 on malloc fail
*/
int list_add_tail(ListNode **head, void *value);

/**
 * Add a new node to the head of a linked list
 * 
 * Parameters:
 *   head: Pointer to the list to add to
 *   value: The value of the node to add
 * 
 * Returns:
 *   0 on success
 *   -1 on malloc fail
*/
int list_add_head(ListNode **head, void *value);


/**
 * Remove an item from linked list. Frees the list node.
 * 
 * Parameters:
 *   head: Pointer to the list to add to
 *   value: The value of the node to remove
 * 
 * Returns:
 *   0 on success
 *   -1 if the item was not in the list
*/
int list_remove(ListNode **head, void *value);