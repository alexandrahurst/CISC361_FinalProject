#include "Job.h"

Job::Job(int number, int max_memory, int max_devices, int runtime, 
         int priority)
: m_number(number), m_max_memory(max_memory), m_max_devices(max_devices),
  m_runtime(runtime), m_priority(priority), m_time_remaining(runtime) {
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

int Job::get_time_remaining() const {
    return m_time_remaining;
}

void Job::set_time_remaining(int time_remaining) {
    m_time_remaining = time_remaining;
}
