#include <stdexcept>
#include <algorithm>
#include <vector>

#include "SystemState.h"

using namespace std;

SystemState::SystemState(int max_memory, int max_devices, int quantum_length, 
                         int time) 
: m_max_memory(max_memory), m_max_devices(max_devices), 
  m_quantum_length(quantum_length), m_time(time), m_cpu(NoJob),
  m_cpu_quantum_remaining(0) {
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

void SystemState::set_time(int time) {
    int delta = time - m_time;
    m_time = time;
    if (m_cpu != NoJob) {
        m_cpu.set_time_remaining(m_cpu.get_time_remaining() - delta);
        m_cpu_quantum_remaining -= delta;
    }
}

// Can be used to trigger premature swapping off CPU from within events
void SystemState::end_quantum() {
    m_cpu_quantum_remaining = 0;
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
    if (job == NoJob){
        m_cpu_quantum_remaining = 0;
    } else {
        m_cpu_quantum_remaining = min(job.get_time_remaining(), 
                                      get_quantum_length());
        QuantumEndEvent* e = new QuantumEndEvent(get_time() 
                                                 + m_cpu_quantum_remaining);
        schedule_event(e);
    }
}

Job SystemState::cpu_get_job() const {
    return m_cpu;
}

bool SystemState::bankers_valid(const Job& requester) const {
    // Collect jobs
    vector<Job> active_jobs;
    if (m_cpu != NoJob) {
        active_jobs.push_back(m_cpu);
    }
    for (Job j : m_ready_queue) {
        active_jobs.push_back(j);
    }
    for (Job j : m_wait_queue) {
        active_jobs.push_back(j);
    }
    
    // Setup
    int Available = m_max_devices;
    vector<int> Max;
    for (const Job& j : active_jobs) {
        Max.push_back(j.get_max_devices());
    }
    vector<int> Allocation;
    for (const Job& j : active_jobs) {
        Allocation.push_back(j.get_allocated_devices());
    }
    vector<int> Need;
    for (uint i = 0; i < active_jobs.size(); i++) {
        Need.push_back(Max[i] - Allocation[i]);
    }
    int requester_i;
    for (uint i = 0; i < active_jobs.size(); i++) {
        if (active_jobs[i] == requester) {
            requester_i = i;
            break;
        }
    }
    int Request = active_jobs[requester_i].get_requested_devices();
    
    // Resource request algorithm
    // Step 1
    if (Request > Need[requester_i]) {
        throw runtime_error("Error: process has exceeded its maximum claim.");
    }
    // Step 2
    if (Request > Available) {
        return false;
    }
    // Step 3: pretend to allocate resources
    Available -= Request;
    Allocation[requester_i] += Request;
    Need[requester_i] -= Request;
    
    // Safety algorithm
    // Step 1
    int Work = Available;
    vector<bool> Finish;
    for (uint i = 0; i < active_jobs.size(); i++) {
        Finish.push_back(false);
    }
    // Step 2
    search:
    for (uint i = 0; i < active_jobs.size(); i++) {
        if (Finish[i] == false && Need[i] < Work) {
            // Step 3
            Work += Allocation[i];
            Finish[i] = true;
            goto search;
        }
    }
    // Step 4
    for (bool f : Finish) {
        if (f == false) {
            return false;
        }
    }
    return true;
}

void SystemState::update_queues() {
    // Move all jobs in wait queue that now pass banker's check to ready queue
    for (deque<Job>::iterator it = m_wait_queue.begin();
         it != m_wait_queue.end();
         it++) {
        Job job = *it;
        if (bankers_valid(job)) {
            it = m_wait_queue.erase(it);
            job.allocate_requested_devices();
            schedule_job(JobQueue::Ready, job);
        }
    }
    
    // TODO Move all jobs in hold queue 1 that now fit into memory into ready queue
    
    // TODO Move all jobs in hold queue 2 that now fit into memory into ready queue
    
    // Push job off cpu into ready queue (or wait queue if there is an active 
    // request for devices that cannot be fulfilled)
    if (m_cpu != NoJob && m_cpu_quantum_remaining == 0) {
        // The job on the CPU is done (either because quantum ended or device 
        // request/release)
        if (m_cpu.get_requested_devices() > 0) { 
            // A device request was made
            if (bankers_valid(m_cpu)) { 
                // The request can be granted immediately
                m_cpu.allocate_requested_devices();
                schedule_job(JobQueue::Ready, m_cpu);
            } else {
                // The request must wait
                schedule_job(JobQueue::Wait, m_cpu);
            }
        } else {
            // No device request was made
            schedule_job(JobQueue::Ready, m_cpu);
        }
    }
    
    // Pull next job from ready queue into cpu. If there are no jobs in 
    // ready queue when pulling into cpu, set cpu job to NoJob.
    if (has_next_job(JobQueue::Ready)) {
        cpu_set_job(pop_next_job(JobQueue::Ready));
    } else {
        cpu_set_job(NoJob);
    }
}
