#include <iostream>
#include <stdexcept>
#include <vector>

#include "DeviceRequestEvent.h"
#include "SystemState.h"

using namespace std;

DeviceRequestEvent::DeviceRequestEvent(int time, int job_number, 
                                       int requested_devices)
: Event(time), m_job_number(job_number), 
  m_requested_devices(requested_devices) {
}
    
void DeviceRequestEvent::process(SystemState& state) {
    cout << get_time() << ": Request for devices" << endl;
    if (state.cpu_get_job() != m_job_number) {
        cerr << " Error: Job requested devices while not on the CPU" << endl;
        return;
    }
    
    state.cpu_request_devices(m_requested_devices);
    state.end_quantum();
}

Event::Type DeviceRequestEvent::get_type() const {
    return Event::Type::External;
}
