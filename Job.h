#ifndef _JOB_H_
#define _JOB_H_

class Job {
public:
    Job(int arrtival_time, int number, int max_memory, int max_devices, int runtime, int priority);
    
    int get_arrival_time() const;
    int get_number() const;
    int get_max_memory() const;
    int get_max_devices() const;
    int get_runtime() const;
    int get_priority() const;
    
    int get_allocated_devices() const;
    void set_allocated_devices(int allocated_devices);
    int get_requested_devices() const;
    void set_requested_devices(int requested_devices);
    void allocate_requested_devices();
    int get_time_remaining() const;
    void set_time_remaining(int time_remaining);
    
    bool operator==(const Job& other) const;
    bool operator!=(const Job& other) const;
    
private:
    int m_arrival_time;
    int m_number;
    int m_max_memory;
    int m_max_devices;
    int m_runtime;
    int m_priority;
    
    int m_allocated_devices;
    int m_time_remaining;
    int m_requested_devices;
};

static const Job NoJob{-1, -1, -1, -1, -1, -1};

#endif // _JOB_H_
