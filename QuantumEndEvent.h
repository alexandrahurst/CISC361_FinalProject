#ifndef _QUANTUM_END_EVENT_H_
#define _QUANTUM_END_EVENT_H_

#include "Event.h"
#include "Job.h"

class SystemState;

class QuantumEndEvent : public Event {
public: 
    QuantumEndEvent(int time, Job job);
    
    void process(SystemState& state);
    Type get_type() const;
    
    void invalidate();

private:
    Job m_job;
    bool m_valid;
};

#endif // _QUANTUM_END_EVENT_H_
