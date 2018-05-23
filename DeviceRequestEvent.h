#ifndef _DEVICE_REQUEST_EVENT_H_
#define _DEVICE_REQUEST_EVENT_H_

#include "Event.h"
#include "Job.h"

class SystemState;

class DeviceRequestEvent : public Event {
public: 
    DeviceRequestEvent(int time, int job_number, int requested_devices);
    
    void process(SystemState& state);
    Type get_type() const;
    
private:
    int m_job_number;
    int m_requested_devices;
};

#endif // _DEVICE_REQUEST_EVENT_H_
