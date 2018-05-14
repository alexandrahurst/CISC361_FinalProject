#include <stdexcept>
#include <algorithm>

#include "SystemState.h"

using namespace std;

SystemState::SystemState(int max_memory, int max_devices, int quantum_length, 
                         int time) 
: m_max_memory(max_memory), m_max_devices(max_devices), 
  m_quantum_length(quantum_length), m_time(time), m_cpu(NoJob),
  m_cpu_quantum_end_event(NULL) {
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
    if (m_cpu != NoJob) {
        /* This looks dangerous, but will never dereference a null or freed 
         * pointer because when m_cpu == NoJob, m_cpu_quantum_end_event == 
         * NULL, and the only time a QuantumEndEvent is freed is after it
         * is processed, at which point the prcoessing inside QuantumEndEvent 
         * will call cpu_set_job() with either NoJob or another job, thus 
         * updating m_cpu_quantum_end_event to a new pointer that will remain 
         * valid after the calling QuantumEndEvent is freed. */
        m_cpu_quantum_end_event->invalidate();
    }
    m_cpu = job;
    if (job == NoJob){
        m_cpu_quantum_end_event = NULL;
    } else {
        QuantumEndEvent* e = new QuantumEndEvent(
            get_time() + max(job.get_time_remaining(), get_quantum_length()), 
            job);
        schedule_event(e);
        m_cpu_quantum_end_event = e;
    }
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
