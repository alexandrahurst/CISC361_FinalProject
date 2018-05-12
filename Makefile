CC = g++
CFLAGS = -g -Wall
TARGET = SchedulingSimulator

all: $(TARGET)

$(TARGET): main.o Event.o
	$(CC) $(CFLAFS) -o $(TARGET) main.o Event.o

main.o: main.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c main.cpp
	
Event.o: Event.cpp Event.h SystemState.h
	$(CC) $(CFLAGS) -c Event.cpp

clean:
	$(RM) $(TARGET)
