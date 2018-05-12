#ifndef _JOB_H_
#define _JOB_H_

class Job {
public:
    Job(int number, int max_memory, int max_devices, int runtime, int priority);
    
    int get_number() const;
    int get_max_memory() const;
    int get_max_devices() const;
    int get_runtime() const;
    int get_priority() const;
    
    int get_allocated_devices() const;
    void set_allocated_devices(int allocated_devices);
    int get_time_remaining() const;
    void set_time_remaining(int time_remaining);
    
private:
    const int m_number;
    const int m_max_memory;
    const int m_max_devices;
    const int m_runtime;
    const int m_priority;
    
    int m_allocated_devices;
    int m_time_remaining;
};

#endif // _JOB_H_
