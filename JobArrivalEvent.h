#ifndef _JOB_ARRIVAL_EVENT_H_
#define _JOB_ARRIVAL_EVENT_H_

#include "Event.h"
#include "Job.h"

class SystemState;

class JobArrivalEvent : public Event {
public: 
    JobArrivalEvent(int time, Job job);
    
    void process(SystemState& state);
    Type get_type() const;

private:
    Job m_job;
};

#endif // _JOB_ARRIVAL_EVENT_H_
