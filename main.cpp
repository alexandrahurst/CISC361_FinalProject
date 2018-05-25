// Alexandra Hurst & Kalyani Khutale
// CISC-361
// 2018-05-21
// Final Project

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <stdexcept>
#include <unordered_map>

#include "SystemState.h"
#include "Event.h"
#include "JobArrivalEvent.h"
#include "DeviceRequestEvent.h"
#include "DeviceReleaseEvent.h"
#include "DisplayEvent.h"
#include "Job.h"

#define CONFIGURATION "C"
#define MAX_MEMORY "M"
#define MAX_DEVICES "S"
#define QUANTUM_LENGTH "Q"
#define JOB_ARRIVAL "A"
#define JOB_NUMBER "J"
#define RUNTIME "R"
#define PRIORITY "P"
#define DEVICE_REQUEST "Q"
#define NUM_DEVICES "D"
#define DEVICE_RELEASE "L"
#define DISPLAY "D"

using namespace std;

unordered_map<string, int> parse_command_tokens(const vector<string>& tokens) {
    unordered_map<string, int> pairs;
    for (uint i = 2; i < tokens.size(); i++) {
        if (tokens[i].size() < 3) {
            throw runtime_error("Error: Malformed input line");
        }
        string parameter = tokens[i].substr(0, 1);
        string value = tokens[i].substr(2);
        pairs.insert({{parameter, atoi(value.c_str())}});
    }
    return pairs;
}

vector<string> split(const string& str, char delimiter) {
   vector<string> tokens;
   string token;
   istringstream tokenStream(str);
   while (getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}

void process_events_through_time(int time, SystemState& state) {
    while (state.has_next_event() && state.get_next_event()->get_time() <= time) {
        // Step cpu to current time (which will reduce remaining time for 
        // current process if necessary)
        int event_time = state.get_next_event()->get_time();
        state.set_time(event_time);
        
        // Process event
        Event* e = state.pop_next_event();
        e->process(state);
        delete e;
        
        // Update queues and move jobs on/off CPU if necessary
        state.update_queues();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        throw runtime_error("Error: Please specify an input file.");
    }
    ifstream in_file(argv[1]);
    if (in_file.fail()) {
        throw runtime_error("Error: Could not find specified input file.");
    }
    
    SystemState* state;
    
    bool explicit_final_print = false;
    
    for (string line; getline(in_file, line);) {
        vector<string> tokens = split(line, ' ');
        if (tokens.size() < 2) {
            throw runtime_error("Error: Malformed input line");
        }
        int command_time = atoi(tokens[1].c_str());
        
        if (tokens[0] == CONFIGURATION) {
            cout << command_time << ": System configuration" << endl;
            unordered_map<string, int> pairs = parse_command_tokens(tokens);
            try {
                state = new SystemState(
                    pairs.at(MAX_MEMORY), 
                    pairs.at(MAX_DEVICES), 
                    pairs.at(QUANTUM_LENGTH),
                    command_time);
            } catch (const out_of_range& e) {
                throw runtime_error("Error: Malformed input line");
            }
        } else if (tokens[0] == JOB_ARRIVAL) {
            unordered_map<string, int> pairs = parse_command_tokens(tokens);
            try {
                Job job(
                    command_time,
                    pairs.at(JOB_NUMBER),
                    pairs.at(MAX_MEMORY),
                    pairs.at(MAX_DEVICES),
                    pairs.at(RUNTIME),
                    pairs.at(PRIORITY));
                Event* e = new JobArrivalEvent(command_time, job);
                state->schedule_event(e);
            } catch (const out_of_range& e) {
                throw runtime_error("Error: Malformed input line");
            }
        } else if (tokens[0] == "Q") {
            unordered_map<string, int> pairs = parse_command_tokens(tokens);
            try {
                Event* e = new DeviceRequestEvent(command_time, 
                                                  pairs.at(JOB_NUMBER),
                                                  pairs.at(NUM_DEVICES));
                state->schedule_event(e);
            } catch (const out_of_range& e) {
                throw runtime_error("Error: Malformed input line");
            }
        } else if (tokens[0] == "L") {
            unordered_map<string, int> pairs = parse_command_tokens(tokens);
            try {
                Event* e = new DeviceReleaseEvent(command_time, 
                                                  pairs.at(JOB_NUMBER),
                                                  pairs.at(NUM_DEVICES));
                state->schedule_event(e);
            } catch (const out_of_range& e) {
                throw runtime_error("Error: Malformed input line");
            }
        } else if (tokens[0] == "D") {
            if (command_time == END_TIME) {
                explicit_final_print = true;
            }
            Event* e = new DisplayEvent(command_time);
            state->schedule_event(e);
        } else {
            cerr << command_time << ": Unknown input command" << endl;
            return 1;
        }
        
        process_events_through_time(command_time, *state);
    }
    
    if (!explicit_final_print) {
        Event* e = new DisplayEvent(END_TIME);
        state->schedule_event(e);
        
        process_events_through_time(END_TIME, *state);
    }
    
    delete state;
    
    return 0;
}
