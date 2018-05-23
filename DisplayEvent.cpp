#include <iostream>
#include <stdexcept>
#include <vector>

#include "DisplayEvent.h"
#include "SystemState.h"

using namespace std;

DisplayEvent::DisplayEvent(int time) : Event(time) {
}
    
void DisplayEvent::process(SystemState& state) {
    // TODO implement display logic
}

Event::Type DisplayEvent::get_type() const {
    return Event::Type::External;
}
