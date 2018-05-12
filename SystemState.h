#ifndef _SYSTEM_STATE_H_
#define _SYSTEM_STATE_H_

#include <deque>

#include "Event.h"
#include "Job.h"

class Event;

class SystemState {
public:
    SystemState(int max_memory, int max_devices, int quantum_length, int time);
    
    int get_max_memory() const;
    int get_max_devices() const;
    int get_allocated_memory() const;
    int get_allocated_devices() const;
    int get_available_memory() const;
    int get_available_devices() const;
    
    int get_quantum_length() const;
    int get_time() const;
    
    void schedule_event(Event* e);
    bool has_next_event() const;
    Event* get_next_event() const;
    Event* pop_next_event();
private:
    const int m_max_memory;
    const int m_max_devices;
    const int m_quantum_length;
    
    int m_allocated_memory;
    int m_allocated_devices;
    int m_time;

    std::deque<Event*> m_event_queue;
    std::deque<Job> m_hold_queue_1;
    std::deque<Job> m_hold_queue_2;
    /*std::deque<Process> m_ready_queue;
    std::deque<Process> m_wait_queue;
    Process m_cpu;
    std::deque<Job> m_complete_queue;*/
};

#endif // _SYSTEM_STATE_H_
