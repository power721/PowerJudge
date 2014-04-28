LD=g++
CXX=g++
INCLUDE=include/
CXXFLAGS=-I $(INCLUDE) -g -Wall -O2
LDFLAGS=
TARGET=bin/powerjudge
OBJECTS=bin/judge.o

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)
#	sudo cp $(TARGET) /usr/local/bin/
#	sudo chmod 4755 /usr/local/bin/powerjudge

bin/%.o: src/%.c src/*.h
	$(CXX) $< -c -o $@ $(CXXFLAGS)
