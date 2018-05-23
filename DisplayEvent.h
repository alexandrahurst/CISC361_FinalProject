#ifndef _DISPLAY_EVENT_H_
#define _DISPLAY_EVENT_H_

#include "Event.h"
#include "Job.h"

class SystemState;

class DisplayEvent : public Event {
public: 
    DisplayEvent(int time);
    
    void process(SystemState& state);
    Type get_type() const;
};

#endif // _DISPLAY_EVENT_H_
