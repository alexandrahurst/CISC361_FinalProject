#include <iostream>
#include <stdexcept>
#include <vector>

#include "DisplayEvent.h"
#include "SystemState.h"

using namespace std;

DisplayEvent::DisplayEvent(int time) : Event(time) {
}
    
void DisplayEvent::process(SystemState& state) {
    cout << get_time() << ": Display system status" << endl;
    // TODO write json to file instead of stdout
    bool include_system_turnaround = get_time() == END_TIME;
    cout << state.to_text(include_system_turnaround) << endl;
    //cout << state.to_json() << endl;
}

Event::Type DisplayEvent::get_type() const {
    return Event::Type::External;
}
