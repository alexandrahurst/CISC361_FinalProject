#ifndef _DEVICE_RELEASE_EVENT_H_
#define _DEVICE_RELEASE_EVENT_H_

#include "Event.h"
#include "Job.h"

class SystemState;

class DeviceReleaseEvent : public Event {
public: 
    DeviceReleaseEvent(int time, int job_number, int released_devices);
    
    void process(SystemState& state);
    Type get_type() const;
    
private:
    int m_job_number;
    int m_released_devices;
};

#endif // _DEVICE_RELEASE_EVENT_H_
