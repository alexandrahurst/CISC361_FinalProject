#include "Job.h"

Job::Job(int arrival_time, int number, int max_memory, int max_devices, 
         int runtime, int priority)
: m_arrival_time(arrival_time), m_number(number), m_max_memory(max_memory), 
  m_max_devices(max_devices), m_runtime(runtime), m_priority(priority), 
  m_time_remaining(runtime) {
}
    
int Job::get_arrival_time() const {
    return m_arrival_time;
}
    
int Job::get_number() const {
    return m_number;
}

int Job::get_max_memory() const {
    return m_max_memory;
}

int Job::get_max_devices() const {
    return m_max_devices;
}

int Job::get_runtime() const {
    return m_runtime;
}

int Job::get_priority() const {
    return m_priority;
}

int Job::get_allocated_devices() const {
    return m_allocated_devices;
}

void Job::set_allocated_devices(int allocated_devices) {
    m_allocated_devices = allocated_devices;
}

int Job::get_requested_devices() const {
    return m_requested_devices;
}

void Job::set_requested_devices(int requested_devices) {
    m_requested_devices = requested_devices;
}

void Job::allocate_requested_devices() {
    set_allocated_devices(get_allocated_devices() + get_requested_devices());
    set_requested_devices(0);
}

int Job::get_time_remaining() const {
    return m_time_remaining;
}

void Job::set_time_remaining(int time_remaining) {
    m_time_remaining = time_remaining;
}

bool Job::operator==(const Job& other) const { 
    return this->get_arrival_time() == other.get_arrival_time()
            && this->get_number() == other.get_number()
            && this->get_max_memory() == other.get_max_memory()
            && this->get_max_devices() == other.get_max_devices()
            && this->get_runtime() == other.get_runtime()
            && this->get_priority() == other.get_priority()
            && this->get_allocated_devices() == other.get_allocated_devices()
            && this->get_requested_devices() == other.get_requested_devices()
            && this->get_time_remaining() == other.get_time_remaining();
}

bool Job::operator!=(const Job& other) const { 
    return !(*this == other); 
}
