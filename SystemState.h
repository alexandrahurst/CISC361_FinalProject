#ifndef _SYSTEM_STATE_H_
#define _SYSTEM_STATE_H_

#include <deque>
#include <array>

#include "Job.h"
#include "Event.h"
#include "QuantumEndEvent.h"

class SystemState {
public:
    enum class JobQueue {
        Hold1,
        Hold2,
        Ready,
        Wait,
        Complete,
    };
    
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
    
    void schedule_job(JobQueue queue, Job job);
    bool has_next_job(JobQueue queue);
    Job get_next_job(JobQueue queue);
    Job pop_next_job(JobQueue queue);
    
    void cpu_set_job(Job job);
    Job cpu_get_job() const;
    
    /*bool bankers_devices_sufficient(const Job& new_job) const;*/
private:
    int m_max_memory;
    int m_max_devices;
    int m_quantum_length;
    
    int m_allocated_memory;
    int m_allocated_devices;
    int m_time;

    std::deque<Event*> m_event_queue;
    std::deque<Job> m_hold_queue_1;
    std::deque<Job> m_hold_queue_2;
    std::deque<Job> m_ready_queue;
    std::deque<Job> m_wait_queue;
    Job m_cpu;
    QuantumEndEvent* m_cpu_quantum_end_event;
    std::deque<Job> m_complete_queue;
    
    std::deque<Job>& get_queue(JobQueue queue);
};

#endif // _SYSTEM_STATE_H_
