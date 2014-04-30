LD=g++
CXX=g++
CXXFLAGS=-g -Wall -O2 -DLOG_LEVEL=LOG_DEBUG
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

.PHONY: test clean
test:
	g++ -o test/data/1405/spj test/data/1405/spj.cc
	chmod a+x test/unitTest.sh
	cd test && ./unitTest.sh

clean:
	sudo rm -f bin/judge.o bin/powerjudge
	sudo rm -rf test/temp