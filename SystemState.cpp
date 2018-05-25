#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>

#include "SystemState.h"

using namespace std;

SystemState::SystemState(int max_memory, int max_devices, int quantum_length, 
                         int time) 
: m_max_memory(max_memory), m_max_devices(max_devices), 
  m_quantum_length(quantum_length), m_allocated_memory(0),
  m_allocated_devices(0), m_time(time), m_jobs(), m_event_queue(), 
  m_hold_queue_1(), m_hold_queue_2(), m_ready_queue(), m_wait_queue(), 
  m_cpu(NoJob), m_cpu_quantum_remaining(0), m_complete_queue() {
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

void SystemState::cpu_allocate_requested_devices() {
    cout << "Allocated " << m_jobs.at(m_cpu).get_requested_devices() 
         << " devices to job " << m_cpu << endl;
    m_allocated_devices += m_jobs.at(m_cpu).get_requested_devices();
    m_jobs.at(m_cpu).allocate_requested_devices();
}

void SystemState::cpu_request_devices(int devices) {
    m_jobs.at(m_cpu).set_requested_devices(devices);
}

void SystemState::cpu_release_devices(int devices) {
    m_jobs.at(m_cpu).release_devices(devices);
    m_allocated_devices -= devices;
}

void SystemState::allocate_memory(int memory) {
    m_allocated_memory += memory;
}

void SystemState::release_memory(int memory) {
    m_allocated_memory -= memory;
}

int SystemState::get_quantum_length() const {
    return m_quantum_length;
}

int SystemState::get_time() const {
    return m_time;
}

void SystemState::set_time(int time) {
    
    cout << "Time set to " << time << ", was " << m_time << endl;
    int delta = time - m_time;
    m_time = time;
    if (m_cpu != NoJob) {
        m_jobs.at(m_cpu).step_time(delta);
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

void SystemState::add_job(const Job& job) {
    m_jobs.insert({{ job.get_number(), job }});
}

void SystemState::schedule_job(JobQueue queue, int job_id) {
    if (queue == JobQueue::Hold1) {
        deque<int>::iterator it = m_hold_queue_1.begin();
        while (it != m_hold_queue_1.end() 
               && m_jobs.at(*it).get_runtime() < m_jobs.at(job_id).get_runtime()) {
            it++;
        }
        m_hold_queue_1.insert(it, job_id);
        cout << "Job " << job_id << " placed in hold queue 1" << endl;
    } else if (queue == JobQueue::Hold2) {
        m_hold_queue_2.push_back(job_id);
        cout << "Job " << job_id << " placed in hold queue 2" << endl;
    } else if (queue == JobQueue::Ready) {
        m_ready_queue.push_back(job_id);
        cout << "Job " << job_id << " placed in ready queue" << endl;
    } else if (queue == JobQueue::Wait) {
        cout << "Job " << job_id << " placed in wait queue" << endl;
        m_wait_queue.push_back(job_id);
    } else if (queue == JobQueue::Complete) {
        m_complete_queue.push_back(job_id);
        cout << "Job " << job_id << " placed in complete queue" << endl;
    }
}
    
bool SystemState::has_next_job(JobQueue queue) {
    return !get_queue(queue).empty();
}

int SystemState::get_next_job(JobQueue queue) {
    return get_queue(queue).front();
}

int SystemState::pop_next_job(JobQueue queue) {
    int job_id = get_queue(queue).front();
    get_queue(queue).pop_front();
    return job_id;
}

deque<int>& SystemState::get_queue(JobQueue queue) {
    switch (queue) {
        case JobQueue::Hold1: return m_hold_queue_1;
        case JobQueue::Hold2: return m_hold_queue_2;
        case JobQueue::Ready: return m_ready_queue;
        case JobQueue::Wait: return m_wait_queue;
        case JobQueue::Complete: return m_complete_queue;
        default: throw runtime_error("Error: Invalid queue requested.");
    }
}

void SystemState::cpu_set_job(int job_id) {
    m_cpu = job_id;
    if (job_id == NoJob){
        m_cpu_quantum_remaining = 0;
    } else {
        m_cpu_quantum_remaining = min(m_jobs.at(job_id).get_time_remaining(), 
                                      get_quantum_length());
        QuantumEndEvent* e = new QuantumEndEvent(get_time() 
                                                 + m_cpu_quantum_remaining);
        schedule_event(e);
    }
}

int SystemState::cpu_get_job() const {
    return m_cpu;
}

bool SystemState::bankers_valid(int requester) const {
    // Collect jobs
    vector<Job> active_jobs;
    if (m_cpu != NoJob) {
        active_jobs.push_back(m_jobs.at(m_cpu));
    }
    for (int j : m_ready_queue) {
        active_jobs.push_back(m_jobs.at(j));
    }
    for (int j : m_wait_queue) {
        active_jobs.push_back(m_jobs.at(j));
    }
    
    // Setup
    int Available = get_available_devices();
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
        if (active_jobs[i].get_number() == requester) {
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
    // Push job off cpu into ready queue (or wait queue if there is an active 
    // request for devices that cannot be fulfilled) (or complete queue if 
    // there is no time remaining)
    if (m_cpu != NoJob && m_cpu_quantum_remaining == 0) {
        // The job on the CPU is done (either because quantum ended or device 
        // request/release)
        if (m_jobs.at(m_cpu).get_time_remaining() == 0) {
            // Job is complete, so release memory and devices
            cout << "Job " << m_cpu << " is complete, so release memory and devices" << endl;
            release_memory(m_jobs.at(m_cpu).get_max_memory());
            cpu_release_devices(m_jobs.at(m_cpu).get_allocated_devices());
            m_jobs.at(m_cpu).set_completion_time(m_time);
            schedule_job(JobQueue::Complete, m_cpu);
        } else {
            // Job is not yet complete
            if (m_jobs.at(m_cpu).get_requested_devices() > 0) { 
                // A device request was made
                if (bankers_valid(m_cpu)) { 
                    // The request can be granted immediately
                    cpu_allocate_requested_devices();
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
        cpu_set_job(NoJob);
    }
    
    // Move all jobs in wait queue that now pass banker's check to ready queue
    for (deque<int>::iterator it = m_wait_queue.begin();
         it != m_wait_queue.end();) {
        int job_id = *it;
        if (bankers_valid(job_id)) {
            it = m_wait_queue.erase(it);
            m_jobs.at(job_id).allocate_requested_devices();
            schedule_job(JobQueue::Ready, job_id);
        } else {
            it++;
        }
    }
    
    // Move all jobs in hold queue 1 that now fit into memory into ready queue
    for (deque<int>::iterator it = m_hold_queue_1.begin();
         it != m_hold_queue_1.end();) {
        int job_id = *it;
        if (m_jobs.at(job_id).get_max_memory() <= get_available_memory()) {
            it = m_hold_queue_1.erase(it);
            allocate_memory(m_jobs.at(job_id).get_max_memory());
            schedule_job(JobQueue::Ready, job_id);
        } else {
            it++;
        }
    }
    
    
    // Move all jobs in hold queue 2 that now fit into memory into ready queue
    for (deque<int>::iterator it = m_hold_queue_2.begin();
         it != m_hold_queue_2.end();) {
        int job_id = *it;
        if (m_jobs.at(job_id).get_max_memory() <= get_available_memory()) {
            it = m_hold_queue_2.erase(it);
            allocate_memory(m_jobs.at(job_id).get_max_memory());
            schedule_job(JobQueue::Ready, job_id);
        } else {
            it++;
        }
    }
    
    // If no job on CPU, pull next job from ready queue into cpu (if there is 
    // one)
    if (m_cpu == NoJob && has_next_job(JobQueue::Ready)) {
        cout << "Job " << get_next_job(JobQueue::Ready) << " placed on the CPU" << endl;
        cpu_set_job(pop_next_job(JobQueue::Ready));
    }
}

// Display code

string::size_type max_length(const vector<string>& strings) {
    string::size_type max_length = 0;
    for (const string& s : strings) {
        if (s.size() > max_length) {
            max_length = s.size();
        }
    }
    return max_length;
}

bool queue_contains(const deque<int>& queue, int value) {
    for (int v : queue) {
        if (v == value) {
            return true;
        }
    }
    return false;
}

string SystemState::get_job_state(int job_id) const {
    if (m_cpu == job_id) {
        return "CPU";
    } else if (queue_contains(m_hold_queue_1, job_id)) {
        return "Hold queue 1";
    } else if (queue_contains(m_hold_queue_2, job_id)) {
        return "Hold queue 2";
    } else if (queue_contains(m_ready_queue, job_id)) {
        return "Ready queue";
    } else if (queue_contains(m_wait_queue, job_id)) {
        return "Device wait queue";
    } else if (queue_contains(m_complete_queue, job_id)) {
        return "Complete at time " + to_string(m_jobs.at(job_id).get_completion_time());
    } else {
        return "???";
    }
}

string format_time_remaining(int time_remaining) {
    if (time_remaining == 0) {
        return "";
    } else {
        return to_string(time_remaining);
    }
}

string pad_center(const string& contents, char pad_char, int pad_width) {
    int left_pad_width = (pad_width - contents.size()) / 2;
    int right_pad_width = (pad_width - contents.size()) - left_pad_width;
    return string(left_pad_width, pad_char) + contents 
           + string(right_pad_width, pad_char);
}

string pad_left(const string& contents, char pad_char, int pad_width) {
    return contents + string(pad_width - contents.size(), pad_char);
}

string SystemState::to_text() const {
    // Table components
    const string LEFT_COLUMN_BORDER = "| ";
    const int LEFT_COLUMN_BORDER_WIDTH = LEFT_COLUMN_BORDER.size();
    const string CENTER_COLUMN_BORDER = " | ";
    const int CENTER_COLUMN_BORDER_WIDTH = CENTER_COLUMN_BORDER.size();
    const string RIGHT_COLUMN_BORDER = " |";
    const int RIGHT_COLUMN_BORDER_WIDTH = RIGHT_COLUMN_BORDER.size();
    
    // Print jobs
    vector<string> job_numbers;
    vector<string> job_states;
    vector<string> job_remaining_times;
    vector<string> job_unweighted_turnaround_times;
    vector<string> job_weighted_turnaround_times;
    for (const pair<int, Job>& j : m_jobs) {
        job_numbers.push_back(to_string(j.first));
        job_states.push_back(get_job_state(j.first));
        job_remaining_times.push_back(format_time_remaining(j.second.get_time_remaining()));
        job_unweighted_turnaround_times.push_back("TODO");// TODO
        job_weighted_turnaround_times.push_back("TODO");// TODO
    }
    const string JOB_NUMBERS_HEADER = "#";
    const string JOB_STATES_HEADER = "State";
    const string JOB_REMAINING_TIMES_HEADER = "Time Remaining";
    const string JOB_UNWEIGHTED_TURNAROUND_TIMES_HEADER = "Turnaround Time (Unweighted)";
    const string JOB_WEIGHTED_TURNAROUND_TIMES_HEADER = "Turnaround Time (Weighted)";
    int job_numbers_width = max(max_length(job_numbers), JOB_NUMBERS_HEADER.size());
    int job_states_width = max(max_length(job_states), JOB_STATES_HEADER.size());
    int job_remaining_times_width = max(max_length(job_remaining_times), JOB_REMAINING_TIMES_HEADER.size());
    int job_unweighted_turnaround_times_width = max(max_length(job_unweighted_turnaround_times), JOB_UNWEIGHTED_TURNAROUND_TIMES_HEADER.size());
    int job_weighted_turnaround_times_width = max(max_length(job_weighted_turnaround_times), JOB_WEIGHTED_TURNAROUND_TIMES_HEADER.size());
    int total_width = LEFT_COLUMN_BORDER_WIDTH + job_numbers_width 
                      + CENTER_COLUMN_BORDER_WIDTH + job_states_width
                      + CENTER_COLUMN_BORDER_WIDTH + job_remaining_times_width
                      + CENTER_COLUMN_BORDER_WIDTH + job_unweighted_turnaround_times_width
                      + CENTER_COLUMN_BORDER_WIDTH + job_weighted_turnaround_times_width 
                      + RIGHT_COLUMN_BORDER_WIDTH;
    stringstream ss;
    ss << pad_center(" Jobs ", '=', total_width) << endl;
    ss << pad_left("", '-', total_width) << endl;
    ss << LEFT_COLUMN_BORDER << pad_left(JOB_NUMBERS_HEADER, ' ', job_numbers_width)
       << CENTER_COLUMN_BORDER << pad_left(JOB_STATES_HEADER, ' ', job_states_width)
       << CENTER_COLUMN_BORDER << pad_left(JOB_REMAINING_TIMES_HEADER, ' ', job_remaining_times_width)
       << CENTER_COLUMN_BORDER << pad_left(JOB_UNWEIGHTED_TURNAROUND_TIMES_HEADER, ' ', job_unweighted_turnaround_times_width)
       << CENTER_COLUMN_BORDER << pad_left(JOB_WEIGHTED_TURNAROUND_TIMES_HEADER, ' ', job_weighted_turnaround_times_width)
       << RIGHT_COLUMN_BORDER << endl;
    ss << pad_left("", '-', total_width) << endl;
    for (uint i = 0; i < m_jobs.size(); i++) {
        ss << LEFT_COLUMN_BORDER << pad_left(job_numbers[i], ' ', job_numbers_width)
           << CENTER_COLUMN_BORDER << pad_left(job_states[i], ' ', job_states_width)
           << CENTER_COLUMN_BORDER << pad_left(job_remaining_times[i], ' ', job_remaining_times_width)
           << CENTER_COLUMN_BORDER << pad_left(job_unweighted_turnaround_times[i], ' ', job_unweighted_turnaround_times_width)
           << CENTER_COLUMN_BORDER << pad_left(job_weighted_turnaround_times[i], ' ', job_weighted_turnaround_times_width)
           << RIGHT_COLUMN_BORDER << endl;
    }
    ss << pad_left("", '-', total_width) << endl;
    // TODO output the rest
    return ss.str();
}

string SystemState::to_json() const {
    // TODO
    return "";
}

void SystemState::print_event_queue() const {
    cout << "=== PRINT EVENT QUEUE ===" << endl;
    for (const Event* const e : m_event_queue) {
        cout << e->get_time() << ": " 
             << ((e->get_type() == Event::Type::Internal) ? "Internal" : "External")
             << endl;
    }
    cout << "=========================" << endl;
}
