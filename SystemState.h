#ifndef _SYSTEM_STATE_H_
#define _SYSTEM_STATE_H_

#include <deque>

#include "Event.h"

class Event;

struct SystemState {
    const int max_memory;
    const int max_devices;
    const int quantum_length;
    
    int allocated_memory;
    int allocated_devices;
    int time;

    std::deque<Event*> event_queue;
    /*std::deque<Job> hold_queue_1;
    std::deque<Job> hold_queue;
    std::deque<Process> ready_queue;
    std::deque<Process> wait_queue;
    Process cpu;
    std::deque<Job> complete_queue;*/
};

#endif // _SYSTEM_STATE_H_
