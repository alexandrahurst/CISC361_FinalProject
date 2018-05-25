#include <iostream>
#include <stdexcept>
#include <vector>

#include "DeviceReleaseEvent.h"
#include "SystemState.h"

using namespace std;

DeviceReleaseEvent::DeviceReleaseEvent(int time, int job_number, 
                                       int released_devices)
: Event(time), m_job_number(job_number), 
  m_released_devices(released_devices) {
}
    
void DeviceReleaseEvent::process(SystemState& state) {
    cout << get_time() << ": Release for devices" << endl;
    if (state.cpu_get_job() != m_job_number) {
        cerr << " Error: Job attempted to release devices while not on the CPU"
             << endl;
        return;
    }
    
    state.cpu_release_devices(m_released_devices);
    state.end_quantum();
}

Event::Type DeviceReleaseEvent::get_type() const {
    return Event::Type::External;
}
