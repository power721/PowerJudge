LD=g++
CXX=g++
CXXFLAGS=-g -Wall -O2
LDFLAGS=
TARGET=bin/powerjudge
OBJECTS=bin/judge.o

ifdef LOG_LEVEL
	CXXFLAGS+= -DLOG_LEVEL=${LOG_LEVEL}
endif

ifdef FAST_JUDGE
	CXXFLAGS+= -DFAST_JUDGE
endif

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)

bin/%.o: src/%.c src/*.h
	$(CXX) $< -c -o $@ $(CXXFLAGS)

.PHONY: test install clean
test:
	g++ -o test/data/1405/spj test/data/1405/spj.cc
	chmod a+x test/unitTest.sh
	cd test && ./unitTest.sh

install:
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod 4755 /usr/local/bin/powerjudge

clean:
	sudo rm -f bin/judge.o bin/powerjudge
	sudo rm -rf test/temp
