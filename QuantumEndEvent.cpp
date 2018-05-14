#include <iostream>
#include <stdexcept>
#include <vector>

#include "QuantumEndEvent.h"
#include "SystemState.h"

using namespace std;

QuantumEndEvent::QuantumEndEvent(int time, Job job)
: Event(time), m_job(job), m_valid(true) {
}
    
void QuantumEndEvent::process(SystemState& state) {
    if (m_valid) {
        // TODO Move current job back into ready queue or move to complete 
        // queue. If job was moved back to ready queue, pull next job from 
        // ready queue into cpu. If job was moved to complete queue, check 
        // device wait queue, then hold queue 1, then hold queue 2 for 
        // processes that can move into ready queue and move them if so. Then 
        // pull next job from ready queue into cpu. In either case, if there 
        // are no jobs in ready queue when pulling into cpu, set cpu job to 
        // NoJob.
    }
}

Event::Type QuantumEndEvent::get_type() const {
    return Event::Type::Internal;
}

void QuantumEndEvent::invalidate() {
    m_valid = false;
}
