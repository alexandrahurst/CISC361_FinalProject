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

#include "Event.h"
#include "SystemState.h"

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

template<typename T>
void insert_pointer_sorted(const T& item, deque<T>& queue) {
    typename deque<T>::iterator it = queue.begin();
    while (it != queue.end() && *item < **it) {
        it++;
    }
    it.insert(it, item);
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

int main(int argc, char** argv) {
    if (argc < 2) {
        throw runtime_error("Error: Please specify an input file.");
    }
    ifstream in_file(argv[1]);
    if (in_file.fail()) {
        throw runtime_error("Error: Could not find specified input file.");
    }
    
    SystemState* state;
    
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
                state = new SystemState { 
                    pairs.at(MAX_MEMORY), 
                    pairs.at(MAX_DEVICES), 
                    pairs.at(QUANTUM_LENGTH),
                    command_time
                };
            } catch (const out_of_range& e) {
                throw runtime_error("Error: Malformed input line");
            }
        } else if (tokens[0] == JOB_ARRIVAL) {
            cout << command_time << ": Job arrival" << endl;
            // TODO create and schedule event
        } else if (tokens[0] == "Q") {
            cout << command_time << ": Request for devices" << endl;
            // TODO create and schedule event
        } else if (tokens[0] == "L") {
            cout << command_time << ": Release for devices" << endl;
            // TODO create and schedule event
        } else if (tokens[0] == "D") {
            cout << command_time << ": Display system status" << endl;
            // TODO create and schedule event
        } else {
            cerr << command_time << ": Unknown input command" << endl;
            return 1;
        }
        
        
        
        while (!state->event_queue.empty() && state->event_queue.front()->get_time() <= command_time) {
            Event* e = state->event_queue.front();
            state->event_queue.pop_front();
            e->process(*state);
            delete e;
        }
    }
    
    delete state;
    
    return 0;
}
