CXX      = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -pthread
LDFLAGS  = -pthread

TARGET   = raytracer
OBJS     = raytracer.o intersect.o scene_io.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

raytracer.o: raytracer.cpp raytracer.hpp scene_io.h
	$(CXX) $(CXXFLAGS) -c raytracer.cpp -o raytracer.o

intersect.o: intersect.cpp raytracer.hpp scene_io.h
	$(CXX) $(CXXFLAGS) -c intersect.cpp -o intersect.o

scene_io.o: scene_io.cpp scene_io.h json.hpp
	$(CXX) $(CXXFLAGS) -c scene_io.cpp -o scene_io.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean