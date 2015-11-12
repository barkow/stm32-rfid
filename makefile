CXX=g++

INCLUDES=-I/usr/arm-linux-gnueabi/include/c++/4.6.3 -I. -I./rfid 

#CPPFLAGS=-D__GXX_EXPERIMENTAL_CXX0X__ -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8

#CXXFLAGS=-std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -pthread $(CPPFLAGS) $(INCLUDES)
CXXFLAGS=-O0 -g3 -Wall -c -fmessage-length=0 $(CPPFLAGS) $(INCLUDES)

#LDFLAGS=-lpthread

#LDLIBS=-L/usr/arm-linux-gnueabi/lib

RM=rm -f

SOURCES=./main.cpp ./rfid/MFRC522.cpp ./spiclass.cpp ./MFRC522Desfire.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=test


all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDLIBS) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS)

