CC = g++
CFLAGS = -g -Wall
TARGET = SchedulingSimulator

all: $(TARGET)

$(TARGET): main.o SystemState.o Event.o JobArrivalEvent.o
	$(CC) $(CFLAFS) -o $(TARGET) main.o SystemState.o Event.o JobArrivalEvent.o

main.o: main.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c main.cpp
	
SystemState.o: SystemState.cpp SystemState.h Event.h
	$(CC) $(CFLAGS) -c SystemState.cpp
	
Event.o: Event.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c Event.cpp
	
JobArrivalEvent.o: JobArrivalEvent.cpp JobArrivalEvent.h Event.h SystemState.h
	$(CC) $(CFLAGS) -c JobArrivalEvent.cpp

clean:
	$(RM) $(TARGET)
