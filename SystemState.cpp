#include <stdexcept>

#include "SystemState.h"

using namespace std;

SystemState::SystemState(int max_memory, int max_devices, int quantum_length, 
                         int time) 
: m_max_memory(max_memory), m_max_devices(max_devices), 
  m_quantum_length(quantum_length), m_time(time), m_cpu(NoJob) {
}

int SystemState::get_max_memory() const {
    return m_max_memory;
}

int SystemState::get_max_devices() const {
    return m_max_devices;
}

int SystemState::get_allocated_memory() const {
    return m_allocated_memory;
}

int SystemState::get_allocated_devices() const {
    return m_allocated_devices;
}

int SystemState::get_available_memory() const {
    return get_max_memory() - get_allocated_memory();
}

int SystemState::get_available_devices() const {
    return get_max_devices() - get_allocated_devices();
}

int SystemState::get_quantum_length() const {
    return m_quantum_length;
}

int SystemState::get_time() const {
    return m_time;
}

void SystemState::schedule_event(Event* e) {
    deque<Event*>::iterator it = m_event_queue.begin();
    while (it != m_event_queue.end() && **it < *e) {
        it++;
    }
    m_event_queue.insert(it, e);
}

bool SystemState::has_next_event() const {
    return !m_event_queue.empty();
}

Event* SystemState::get_next_event() const {
    return m_event_queue.front();
}

Event* SystemState::pop_next_event() {
    Event* e = m_event_queue.front();
    m_event_queue.pop_front();
    return e;
}

void SystemState::schedule_job(JobQueue queue, Job job) {
    if (queue == JobQueue::Hold1) {
        deque<Job>::iterator it = m_hold_queue_1.begin();
        while (it != m_hold_queue_1.end() && it->get_runtime() < job.get_runtime()) {
            it++;
        }
        m_hold_queue_1.insert(it, job);
    } else if (queue == JobQueue::Hold2) {
        m_hold_queue_2.push_back(job);
    } else if (queue == JobQueue::Ready) {
        m_ready_queue.push_back(job);
    } else if (queue == JobQueue::Wait) {
        m_wait_queue.push_back(job);
    } else if (queue == JobQueue::Complete) {
        m_complete_queue.push_back(job);
    }
}
    
bool SystemState::has_next_job(JobQueue queue) {
    return !get_queue(queue).empty();
}

Job SystemState::get_next_job(JobQueue queue) {
    return get_queue(queue).front();
}

Job SystemState::pop_next_job(JobQueue queue) {
    Job job = get_queue(queue).front();
    get_queue(queue).pop_front();
    return job;
}

deque<Job>& SystemState::get_queue(JobQueue queue) {
    switch (queue) {
        case JobQueue::Hold1: return m_hold_queue_1;
        case JobQueue::Hold2: return m_hold_queue_2;
        case JobQueue::Ready: return m_ready_queue;
        case JobQueue::Wait: return m_wait_queue;
        case JobQueue::Complete: return m_complete_queue;
        default: throw runtime_error("Error: Invalid queue requested.");
    }
}

void SystemState::cpu_set_job(Job job) {
    m_cpu = job;
}

Job SystemState::cpu_get_job() const {
    return m_cpu;
}

/*bool bankers_devices_sufficient(const Job& new_job) {
    int available = m_max_devices;
    vector<int> max;
    for (const Job& job : m_ready_queue) {
        max.push_back(job.get_max_devices());
    }
    max.push_back(new_job.get_max_devices());
    
        
    // TODO implement banker's algorithm
    return false;
}*/
