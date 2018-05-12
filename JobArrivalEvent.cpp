#include "JobArrivalEvent.h"

#include <iostream>

JobArrivalEvent::JobArrivalEvent(int time, int job_number, int job_max_memory, 
                                 int job_max_devices, int job_runtime, 
                                 int job_priority)
: Event(time), m_job_number(job_number), m_job_max_memory(job_max_memory),
  m_job_max_devices(job_max_devices), m_job_runtime(job_runtime), 
  m_job_priority(job_priority) {
}
    
void JobArrivalEvent::process(SystemState& state) {
    std::cout << "Job " << m_job_number << " processed." << std::endl;
}

Event::Type JobArrivalEvent::get_type() const {
    return Event::Type::External;
}
