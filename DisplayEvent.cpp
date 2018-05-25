#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "DisplayEvent.h"
#include "SystemState.h"

using namespace std;

DisplayEvent::DisplayEvent(int time, string filename) 
: Event(time), m_filename(filename) {
}
    
void DisplayEvent::process(SystemState& state) {
    cout << get_time() << ": Display system status" << endl;
    bool include_system_turnaround = get_time() == END_TIME;
    // Print text output to console
    cout << state.to_text(include_system_turnaround) << endl;
    // Write json output to file
    string out_filename = m_filename + "_D" + to_string(get_time()) + ".json";
    ofstream out_file;
    out_file.open(out_filename);
    out_file << state.to_json(include_system_turnaround);
    out_file.close();
}

Event::Type DisplayEvent::get_type() const {
    return Event::Type::External;
}
