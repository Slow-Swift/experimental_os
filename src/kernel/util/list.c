#include "list.h"

#include <stdlib.h>

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
int list_add_tail(ListNode **head, void *value) {
    ListNode *new_node = malloc(sizeof(ListNode));
    if (new_node == NULL) return -1;

    new_node->value = value;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return 0;
    }

    ListNode *current = *head;
    while(current->next != NULL) current = current->next;
    current->next = new_node;
    return 0;
}

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
int list_add_head(ListNode **head, void *value) {
    ListNode *new_node = malloc(sizeof(ListNode));
    if (new_node == NULL) return -1;

    new_node->value = value;
    new_node->next = *head;
    *head = new_node;
    return 0;
}


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
int list_remove(ListNode **head, void *value) {
    ListNode *prev = NULL;
    ListNode *curr = *head;

    if (*head != NULL && (*head)->value == value) {
        *head = (*head)->next;
        free(curr);
        return 0;
    }

    while (curr != NULL && curr->value != value) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return -1;
    prev->next = curr->next;
    free(curr);
    return 0;
}