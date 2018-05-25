#ifndef _DISPLAY_EVENT_H_
#define _DISPLAY_EVENT_H_

#include <string>

#include "Event.h"
#include "Job.h"

class SystemState;

class DisplayEvent : public Event {
public: 
    DisplayEvent(int time, std::string filename);
    
    void process(SystemState& state);
    Type get_type() const;
    
private:
    std::string m_filename;
};

#endif // _DISPLAY_EVENT_H_
