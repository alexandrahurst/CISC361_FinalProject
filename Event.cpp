#include "Event.h"

Event::Event(int time) 
: m_time(time) { 
}

Event::~Event() {
}

int Event::get_time() const {
    return m_time;
}

bool Event::operator< (const Event& other) const {
    if (this->get_time() != other.get_time()) {
        return this->get_time() < other.get_time();
    } else {
        return this->get_type() == Type::Internal && other.get_type() == Type::External;
    }
}
