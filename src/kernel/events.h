#pragma once

typedef void (*Event_Handler)(void *);

typedef struct Event{
    Event_Handler handler;
    void *args;
    struct Event *next;
} Event;

void add_event(Event_Handler handler, void *args);
void call_next_event();
int get_event_count();