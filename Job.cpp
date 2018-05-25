#include "Job.h"

Job::Job(int arrival_time, int number, int max_memory, int max_devices, 
         int runtime, int priority)
: m_arrival_time(arrival_time), m_number(number), m_max_memory(max_memory), 
  m_max_devices(max_devices), m_runtime(runtime), m_priority(priority), 
  m_allocated_devices(0), m_time_remaining(runtime), m_requested_devices(0),
  m_completion_time(0) {
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

void Job::release_devices(int devices) {
    m_allocated_devices -= devices;
}

int Job::get_time_remaining() const {
    return m_time_remaining;
}

void Job::set_time_remaining(int time_remaining) {
    m_time_remaining = time_remaining;
}

void Job::step_time(int time) {
    m_time_remaining -= time;
}


int Job::get_completion_time() const {
    return m_completion_time;
}

void Job::set_completion_time(int time) {
    m_completion_time = time;
}
