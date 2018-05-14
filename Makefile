CC = g++
CFLAGS = -g -Wall
TARGET = SchedulingSimulator

all: $(TARGET)

$(TARGET): main.o SystemState.o Event.o JobArrivalEvent.o Job.o QuantumEndEvent.o
	$(CC) $(CFLAFS) -o $(TARGET) main.o SystemState.o Event.o JobArrivalEvent.o QuantumEndEvent.o Job.o

main.o: main.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c main.cpp
	
SystemState.o: SystemState.cpp SystemState.h Event.h Job.h
	$(CC) $(CFLAGS) -c SystemState.cpp
	
Event.o: Event.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c Event.cpp
	
JobArrivalEvent.o: JobArrivalEvent.cpp JobArrivalEvent.h Event.h SystemState.h Job.h
	$(CC) $(CFLAGS) -c JobArrivalEvent.cpp
	
QuantumEndEvent.o: QuantumEndEvent.cpp QuantumEndEvent.h Event.h SystemState.h Job.h
	$(CC) $(CFLAGS) -c QuantumEndEvent.cpp
	
Job.o: Job.cpp Job.h
	$(CC) $(CFLAGS) -c Job.cpp

clean:
	$(RM) $(TARGET)
