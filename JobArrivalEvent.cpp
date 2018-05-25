#include <iostream>
#include <stdexcept>
#include <vector>

#include "JobArrivalEvent.h"
#include "SystemState.h"

using namespace std;

JobArrivalEvent::JobArrivalEvent(int time, Job job)
: Event(time), m_job(job) {
}
    
void JobArrivalEvent::process(SystemState& state) {
    cout << get_time() << ": Job arrival" << endl;
    if (m_job.get_max_memory() > state.get_max_memory() 
        || m_job.get_max_devices() > state.get_max_devices()) {
        cerr << "Job " << m_job.get_number() 
             << " rejected due to insufficient total system resources." 
             << endl;
        return;
    } else if (m_job.get_max_memory() > state.get_available_memory()) {
        if (m_job.get_priority() == 1) {
            state.add_job(m_job);
            state.schedule_job(SystemState::JobQueue::Hold1, m_job.get_number());
        } else if (m_job.get_priority() == 2) {
            state.add_job(m_job);
            state.schedule_job(SystemState::JobQueue::Hold2, m_job.get_number());
        } else {
            throw runtime_error("Error: Invalid job priority.");
        }
    } else {
        state.allocate_memory(m_job.get_max_memory());
        state.add_job(m_job);
        state.schedule_job(SystemState::JobQueue::Ready, m_job.get_number());
    }
}

Event::Type JobArrivalEvent::get_type() const {
    return Event::Type::External;
}

