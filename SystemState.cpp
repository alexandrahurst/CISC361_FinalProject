#include "SystemState.h"

using namespace std;

SystemState::SystemState(int max_memory, int max_devices, int quantum_length, 
                         int time) 
: m_max_memory(max_memory), m_max_devices(max_devices), 
  m_quantum_length(quantum_length), m_time(time) {
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
    while (it != m_event_queue.end() && *e < **it) {
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
