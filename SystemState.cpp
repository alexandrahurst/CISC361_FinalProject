#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>
#include <numeric>

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

void SystemState::allocate_requested_devices(int job_id) {
    m_allocated_devices += m_jobs.at(job_id).get_requested_devices();
    m_jobs.at(job_id).allocate_requested_devices();
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
    for (unsigned int i = 0; i < active_jobs.size(); i++) {
        Need.push_back(Max[i] - Allocation[i]);
    }
    int requester_i;
    for (unsigned int i = 0; i < active_jobs.size(); i++) {
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
    for (unsigned int i = 0; i < active_jobs.size(); i++) {
        Finish.push_back(false);
    }
    // Step 2
    search:
    for (unsigned int i = 0; i < active_jobs.size(); i++) {
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
                    allocate_requested_devices(m_cpu);
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
            allocate_requested_devices(job_id);
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

string pad_center(const string& contents, char pad_char, unsigned int pad_width) {
    if (contents.size() >= pad_width) {
        // Don't need to pad
        return contents;
    }
    int left_pad_width = (pad_width - contents.size()) / 2;
    int right_pad_width = (pad_width - contents.size()) - left_pad_width;
    return string(left_pad_width, pad_char) + contents 
           + string(right_pad_width, pad_char);
}

string pad_left(const string& contents, char pad_char, unsigned int pad_width) {
    if (contents.size() >= pad_width) {
        // Don't need to pad
        return contents;
    }
    return contents + string(pad_width - contents.size(), pad_char);
}

string print_table(const vector<vector<string>>& columns,
                   const vector<string>& headers = vector<string>(),
                   const string& title = "") {
    // Table components
    const string LEFT_COLUMN_BORDER = "| ";
    const int LEFT_COLUMN_BORDER_WIDTH = LEFT_COLUMN_BORDER.size();
    const string CENTER_COLUMN_BORDER = " | ";
    const int CENTER_COLUMN_BORDER_WIDTH = CENTER_COLUMN_BORDER.size();
    const string RIGHT_COLUMN_BORDER = " |";
    const int RIGHT_COLUMN_BORDER_WIDTH = RIGHT_COLUMN_BORDER.size();
    const char TITLE_PADDING = '=';
    const string TITLE_BORDER = "===";
    const char HORIZONTAL_BORDER = '-';
    const char PADDING = ' ';
    const string EMPTY = "";
    
    // Calculate widths
    vector<int> column_widths;
    for (unsigned int i = 0; i < columns.size(); i++) {
        string::size_type width = max_length(columns[i]);
        if (!headers.empty()) {
            width = max(width, headers[i].size());
        }
        column_widths.push_back(width);
    }
    
    int total_width = accumulate(column_widths.begin(), column_widths.end(), 0)
                      + LEFT_COLUMN_BORDER_WIDTH 
                      + RIGHT_COLUMN_BORDER_WIDTH
                      + CENTER_COLUMN_BORDER_WIDTH * (columns.size() - 1);
    
    stringstream ss;
    // Title
    if (title.size() > 0) {
        string title_string = TITLE_BORDER + PADDING + title + PADDING + TITLE_BORDER;
        ss << pad_center(title_string, TITLE_PADDING, total_width) << endl;
    }
    // Headers
    if (!headers.empty()) {
        ss << pad_left(EMPTY, HORIZONTAL_BORDER, total_width) << endl;
        for (unsigned int i = 0; i < headers.size(); i++) {
            if (i == 0) {
                ss << LEFT_COLUMN_BORDER;
            } else {
                ss << CENTER_COLUMN_BORDER;
            }
            ss << pad_left(headers[i], PADDING, column_widths[i]);
        } 
        ss << RIGHT_COLUMN_BORDER << endl;
    }
    // Columns
    ss << pad_left(EMPTY, HORIZONTAL_BORDER, total_width) << endl;
    for (unsigned int j = 0; j < columns[0].size(); j++) {
        for (unsigned int i = 0; i < columns.size(); i++) {
            if (i == 0) {
                ss << LEFT_COLUMN_BORDER;
            } else {
                ss << CENTER_COLUMN_BORDER;
            }
            ss << pad_left(columns[i][j], PADDING, column_widths[i]);
        } 
        ss << RIGHT_COLUMN_BORDER << endl;
    }
    ss << pad_left(EMPTY, HORIZONTAL_BORDER, total_width) << endl;
    
    return ss.str();
}

string SystemState::print_queue_table(const string& queue_name, JobQueue queue) {
    vector<string> queue_vector;
    for (int job_id : get_queue(queue)) {
        queue_vector.push_back(to_string(job_id));
    }
    string queue_table = print_table(
        { 
            queue_vector 
        },
        {
            "Jobs"
        }, 
        queue_name);
    return queue_table;
}

int unweighted_turnaround(const Job& job) {
    return job.get_completion_time() - job.get_arrival_time();
}

double weighted_turnaround(const Job& job) {
    return (job.get_completion_time() - job.get_arrival_time()) 
           / (double) job.get_runtime();
}

string format_unweighted_turnaround(int turnaround) {
    if (turnaround < 0) {
        return "";
    } else {
        return to_string(turnaround);
    }
}

string format_weighted_turnaround(double turnaround) {
    if (turnaround < 0) {
        return "";
    } else {
        return to_string(turnaround);
    }
}

string SystemState::to_text(bool include_system_turnaround) {
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
        job_unweighted_turnaround_times.push_back(
            format_unweighted_turnaround(unweighted_turnaround(j.second)));
        job_weighted_turnaround_times.push_back(
            format_weighted_turnaround(weighted_turnaround(j.second)));
    }
    string jobs_table = print_table(
        {
            job_numbers, 
            job_states, 
            job_remaining_times, 
            job_unweighted_turnaround_times, 
            job_weighted_turnaround_times
        },
        {
            "#",
            "State",
            "Time Remaining",
            "Turnaround Time (Unweighted)",
            "Turnaround Time (Weighted)"
        },
        "Jobs");
    
    // Print queues
    string hold_queue_1_table = print_queue_table("Hold Queue 1", JobQueue::Hold1);
    string hold_queue_2_table = print_queue_table("Hold Queue 2", JobQueue::Hold2);
    string ready_queue_table = print_queue_table("Ready Queue", JobQueue::Ready);
    string wait_queue_table = print_queue_table("Device Wait Queue", JobQueue::Wait);
    string complete_queue_table = print_queue_table("Complete Queue", JobQueue::Complete);
    
    stringstream ss;
    ss << jobs_table
       << hold_queue_1_table
       << hold_queue_2_table
       << ready_queue_table
       << wait_queue_table
       << complete_queue_table;
    
    if (include_system_turnaround) {
        int sum_unweighted_turnarounds = 0;
        double sum_weighted_turnarounds = 0;
        int num_complete_jobs = 0;
        for (const pair<int, Job>& j : m_jobs) {
            if (queue_contains(m_complete_queue, j.first)) {
                sum_unweighted_turnarounds += unweighted_turnaround(j.second);
                sum_weighted_turnarounds += weighted_turnaround(j.second);
                num_complete_jobs++;
            }
        }
        double average_unweighted_turnaround = sum_unweighted_turnarounds 
                                               / (double) num_complete_jobs;
        ss << "System average unweighted turnaround: " << average_unweighted_turnaround << endl;
        double average_weighted_turnaround = sum_weighted_turnarounds 
                                             / (double) num_complete_jobs;
        ss << "System average weighted turnaround: " << average_weighted_turnaround << endl;
    }
    
    return ss.str();
}

string join_ints(const deque<int>& ints, const string& delimiter) {
    stringstream ss;
    for (unsigned int i = 0; i < ints.size(); i++) {
        ss << to_string(ints[i]);
        if (i != ints.size() - 1) {
            ss << delimiter;
        }
    }
    return ss.str();
}

string join_strings(const vector<string>& strings, const string& delimiter) {
    stringstream ss;
    for (unsigned int i = 0; i < strings.size(); i++) {
        ss << strings[i];
        if (i != strings.size() - 1) {
            ss << delimiter;
        }
    }
    return ss.str();
}

string SystemState::print_job(const Job& job) {
    stringstream ss;
    ss << "{"
       << "\"arrival_time\": " << job.get_arrival_time() << ", ";
    if (queue_contains(m_ready_queue, job.get_number()) 
        || queue_contains(m_wait_queue, job.get_number())
        || m_cpu == job.get_number()) {
        ss << "\"devices_allocated\": " << job.get_allocated_devices() << ", ";
    }
    ss << "\"id\": " << job.get_number() << ", "
       << "\"remaining_time\": " << job.get_time_remaining();
    if (queue_contains(m_complete_queue, job.get_number())) {
        ss << ", "
           << "\"completion_time\": " << job.get_completion_time();
    }
    ss << "}";
    
    return ss.str();
}

string SystemState::to_json(bool include_system_turnaround) {
    stringstream ss;
    
    vector<string> job_strings;
    for (const pair<int, Job>& j : m_jobs) {
        job_strings.push_back(print_job(j.second));
    }
    
    const string DELIMITER = ", ";
    
    ss << "{"
       << "\"readyq\": [" << join_ints(m_ready_queue, DELIMITER) << "]" << DELIMITER
       << "\"current_time\": " << m_time << DELIMITER
       << "\"total_memory\": " << m_max_memory << DELIMITER
       << "\"available_memory\": " << get_available_memory() << DELIMITER
       << "\"total_devices\": " << m_max_devices << DELIMITER
       << "\"running\" :" << m_cpu << DELIMITER
       << "\"submitq\": []" << DELIMITER
       << "\"holdq2\": [" << join_ints(m_hold_queue_2, DELIMITER) << "]" << DELIMITER
       << "\"job\": [" << join_strings(job_strings, DELIMITER) << "]" << DELIMITER
       << "\"holdq1\": [" << join_ints(m_hold_queue_1, DELIMITER) << "]" << DELIMITER
       << "\"available_devices\": " << get_available_devices() << DELIMITER
       << "\"quantum\": " << m_quantum_length << DELIMITER
       << "\"completeq\": [" << join_ints(m_complete_queue, DELIMITER) << "]" << DELIMITER
       << "\"waitq\": [" << join_ints(m_wait_queue, DELIMITER) << "]";
       
    if (include_system_turnaround) {
        ss << DELIMITER;
        int sum_unweighted_turnarounds = 0;
        double sum_weighted_turnarounds = 0;
        int num_complete_jobs = 0;
        for (const pair<int, Job>& j : m_jobs) {
            if (queue_contains(m_complete_queue, j.first)) {
                sum_unweighted_turnarounds += unweighted_turnaround(j.second);
                sum_weighted_turnarounds += weighted_turnaround(j.second);
                num_complete_jobs++;
            }
        }
        double average_unweighted_turnaround = sum_unweighted_turnarounds 
                                               / (double) num_complete_jobs;
        ss << "\"turnaround\": " << average_unweighted_turnaround << DELIMITER;
        double average_weighted_turnaround = sum_weighted_turnarounds 
                                             / (double) num_complete_jobs;
        ss << "\"weighted_turnaround\": " << average_weighted_turnaround;
    }
    ss << "}";
    
    return ss.str();
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
