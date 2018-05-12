#ifndef _JOB_ARRIVAL_EVENT_H_
#define _JOB_ARRIVAL_EVENT_H_

#include "Event.h"
#include "SystemState.h"

class JobArrivalEvent : public Event {
public: 
    JobArrivalEvent(int time, int job_number, int job_max_memory, 
                    int job_max_devices, int job_runtime, 
                    int job_priority);
    
    void process(SystemState& state);
    Type get_type() const;

private:
    int m_job_number;
    int m_job_max_memory;
    int m_job_max_devices;
    int m_job_runtime;
    int m_job_priority;
};

#endif // _JOB_ARRIVAL_EVENT_H_
