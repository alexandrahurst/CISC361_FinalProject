#ifndef _EVENT_H_
#define _EVENT_H_

class SystemState;

class Event {
public: 
    enum class Type {
        Internal, 
        External,
    };
    
    explicit Event(int time);
    virtual ~Event();
    
    virtual void process(SystemState& state) = 0;
    int get_time() const;
    virtual Type get_type() const = 0;
    bool operator< (const Event& other) const;

private:
    int m_time;
};

#endif // _EVENT_H_
