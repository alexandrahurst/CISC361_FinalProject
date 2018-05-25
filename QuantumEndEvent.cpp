#include <iostream>
#include <stdexcept>
#include <vector>

#include "QuantumEndEvent.h"
#include "SystemState.h"

using namespace std;

QuantumEndEvent::QuantumEndEvent(int time) : Event(time) {
}
    
void QuantumEndEvent::process(SystemState& state) {
    cout << get_time() << ": Quantum ended" << endl;
    // QuantumEndEvent itself doesn't do anything except trigger the event 
    // processing mechanism in main() to step the CPU and swap jobs
}

Event::Type QuantumEndEvent::get_type() const {
    return Event::Type::Internal;
}
