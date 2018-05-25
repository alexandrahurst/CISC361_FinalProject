#ifndef _SYSTEM_STATE_H_
#define _SYSTEM_STATE_H_

#include <deque>
#include <vector>
#include <unordered_map>
#include <string>

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
    
    void cpu_request_devices(int devices);
    void cpu_release_devices(int devices);
    
    void allocate_memory(int memory);
    void release_memory(int memory);
    
    int get_quantum_length() const;
    int get_time() const;
    void set_time(int time);
    void end_quantum();
    
    void schedule_event(Event* e);
    bool has_next_event() const;
    Event* get_next_event() const;
    Event* pop_next_event();
    
    void add_job(const Job& job);
    
    void schedule_job(JobQueue queue, int job_id);
    bool has_next_job(JobQueue queue);
    int get_next_job(JobQueue queue);
    int pop_next_job(JobQueue queue);
    
    void cpu_set_job(int job_id);
    int cpu_get_job() const;
    
    void update_queues();
    bool bankers_valid(int requester_id) const;
    
    std::string to_text() const;
    std::string to_json() const;
    
    void print_event_queue() const;
private:
    int m_max_memory;
    int m_max_devices;
    int m_quantum_length;
    
    int m_allocated_memory;
    int m_allocated_devices;
    int m_time;

    std::unordered_map<int, Job> m_jobs;
    std::deque<Event*> m_event_queue;
    std::deque<int> m_hold_queue_1;
    std::deque<int> m_hold_queue_2;
    std::deque<int> m_ready_queue;
    std::deque<int> m_wait_queue;
    int m_cpu;
    int m_cpu_quantum_remaining;
    std::deque<int> m_complete_queue;
    
    std::deque<int>& get_queue(JobQueue queue);
    void cpu_allocate_requested_devices();
    std::string get_job_state(int job_id) const;
};

#endif // _SYSTEM_STATE_H_
