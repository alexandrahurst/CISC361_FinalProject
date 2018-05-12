#include <iostream>

#include "JobArrivalEvent.h"

using namespace std;

JobArrivalEvent::JobArrivalEvent(int time, Job job)
: Event(time), m_job(job) {
}
    
void JobArrivalEvent::process(SystemState& state) {
    if (m_job.get_max_memory() > state.get_max_memory() 
        || m_job.get_max_devices() > state.get_max_devices()) {
        cerr << "Job " << m_job.get_number() 
             << " rejected due to insufficient total system resources." 
             << endl;
        return;
    }
    if (m_job.get_max_memory() > state.get_available_memory() 
        || false /* TODO run banker's algorithm */) {
        // TODO schedule job
    }
}

Event::Type JobArrivalEvent::get_type() const {
    return Event::Type::External;
}
