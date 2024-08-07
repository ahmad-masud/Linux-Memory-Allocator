TARGET = myalloc
OBJS = main.o myalloc.o

CXXFLAGS = -Wall -g -std=c++11
CXX = g++

all: clean $(TARGET)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)