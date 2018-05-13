#include <iostream>
#include <stdexcept>
#include <vector>

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
    } else if (m_job.get_max_memory() > state.get_available_memory()) {
        if (m_job.get_priority() == 1) {
            state.schedule_job(SystemState::JobQueue::Hold1, m_job);
        } else if (m_job.get_priority() == 2) {
            state.schedule_job(SystemState::JobQueue::Hold2, m_job);
        } else {
            throw runtime_error("Error: Invalid job priority.");
        }
    } else {
        if (state.has_next_job(SystemState::JobQueue::Ready) || state.cpu_get_job() != NoJob) {
            state.schedule_job(SystemState::JobQueue::Ready, m_job);
        } else {
            state.cpu_set_job(m_job);
        }
    }
}

Event::Type JobArrivalEvent::get_type() const {
    return Event::Type::External;
}

