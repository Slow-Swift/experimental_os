#include "events.h"
#include <stdlib.h>
#include <stddef.h>

static Event *head;
static Event *tail;
static int event_count = 0;

void add_event(Event_Handler handler, void* args) {
    Event* new_event = malloc(sizeof(Event));
    new_event->handler = handler;
    new_event->next = NULL;
    new_event->args = args;

    // Add to head if there is no element
    if (head == NULL)
        head = new_event;

    // At to tail
    if (tail != NULL)
        tail->next = new_event;

    tail = new_event;
    event_count++;
}

void call_next_event() {
    if (head == NULL) return;

    Event_Handler handler = head->handler;
    void *args = head->args;
    Event *next = head;
    head = head->next;
    event_count--;
    free(next);
    
    handler(args);
}

int get_event_count() {
    return event_count;
}